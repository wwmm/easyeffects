/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "voice_suppressor.hpp"
#include <fftw3.h>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <qtypes.h>
#include <algorithm>
#include <cmath>
#include <format>
#include <mutex>
#include <numbers>
#include <span>
#include <string>
#include <vector>
#include "db_manager.hpp"
#include "easyeffects_db_voice_suppressor.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

VoiceSuppressor::VoiceSuppressor(const std::string& tag,
                                 pw::Manager* pipe_manager,
                                 PipelineType pipe_type,
                                 QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::voiceSuppressor,
                 tags::plugin_package::Package::ee,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(db::Manager::self().get_plugin_db<db::VoiceSuppressor>(
          pipe_type,
          tags::plugin_name::BaseName::voiceSuppressor + "#" + instance_id)) {
  // bypass, input and output gain controls

  init_common_controls<db::VoiceSuppressor>(settings);
}

VoiceSuppressor::~VoiceSuppressor() {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  free_fftw();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void VoiceSuppressor::reset() {
  settings->setDefaults();
}

void VoiceSuppressor::clear_data() {
  setup();
}

void VoiceSuppressor::setup() {
  if (rate == 0 || n_samples == 0) {
    // Some signals may be emitted before PipeWire calls our setup function
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  ready = false;

  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
  QMetaObject::invokeMethod(
      baseWorker,
      [this] {
        if (ready) {
          return;
        }

        std::scoped_lock<std::mutex> lock(data_mutex);

        block_time = static_cast<double>(n_samples) / static_cast<double>(rate);

        buf_in_L.clear();
        buf_in_R.clear();
        buf_out_L.clear();
        buf_out_R.clear();

        ola_L.resize(n_samples, 0.0F);
        ola_R.resize(n_samples, 0.0F);

        data_L.resize(n_samples);
        data_R.resize(n_samples);

        free_fftw();

        realL = static_cast<double*>(fftw_malloc(sizeof(double) * n_samples));
        realR = static_cast<double*>(fftw_malloc(sizeof(double) * n_samples));

        fft_size = (n_samples / 2U) + 1U;

        hop = n_samples / 2;

        complexL = fftw_alloc_complex(fft_size);
        complexR = fftw_alloc_complex(fft_size);

        planL = fftw_plan_dft_r2c_1d(static_cast<int>(n_samples), realL, complexL, FFTW_ESTIMATE);
        planR = fftw_plan_dft_r2c_1d(static_cast<int>(n_samples), realR, complexR, FFTW_ESTIMATE);

        planInvL = fftw_plan_dft_c2r_1d(static_cast<int>(n_samples), complexL, realL, FFTW_ESTIMATE);
        planInvR = fftw_plan_dft_c2r_1d(static_cast<int>(n_samples), complexR, realR, FFTW_ESTIMATE);

        freqs.resize(fft_size);

        previous_wrapped.resize(fft_size, 0.0);
        previous_unwrapped.resize(fft_size, 0.0);

        fft_mag_L.resize(fft_size, 0.0);
        fft_mag_R.resize(fft_size, 0.0);

        fft_cross_real.resize(fft_size, 0.0);
        fft_cross_img.resize(fft_size, 0.0);

        hanning_window.resize(n_samples);

        for (uint k = 0; k < fft_size; k++) {
          freqs[k] = k * static_cast<float>(rate) / n_samples;
        }

        for (uint n = 0U; n < n_samples; n++) {
          hanning_window[n] = 0.5F * (1.0F - std::cos(2.0F * std::numbers::pi_v<float> * static_cast<float>(n) /
                                                      static_cast<float>(n_samples - 1U)));
        }

        ready = true;

        notify_latency = true;
      },
      Qt::QueuedConnection);
  // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
}

void VoiceSuppressor::process(std::span<float>& left_in,
                              std::span<float>& right_in,
                              std::span<float>& left_out,
                              std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    return;
  }

  if (!ready) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    if (output_gain != 1.0F) {
      apply_gain(left_out, right_out, output_gain);
    }

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  buf_in_L.insert(buf_in_L.end(), left_in.begin(), left_in.end());
  buf_in_R.insert(buf_in_R.end(), right_in.begin(), right_in.end());

  while (buf_in_L.size() >= n_samples) {
    util::copy_bulk_remove_half(buf_in_L, data_L);
    util::copy_bulk_remove_half(buf_in_R, data_R);

    for (uint n = 0; n < n_samples; n++) {
      realL[n] = static_cast<double>(data_L[n]);
      realR[n] = static_cast<double>(data_R[n]);
    }

    fftw_execute(planL);
    fftw_execute(planR);

    for (uint k = 0; k < fft_size; k++) {
      float Lr = complexL[k][0];
      float Li = complexL[k][1];
      float Rr = complexR[k][0];
      float Ri = complexR[k][1];

      fft_mag_L[k] = std::hypot(Lr, Li);
      fft_mag_R[k] = std::hypot(Rr, Ri);

      // Inner product between the left channel and the complexity conjugated of the right channel

      fft_cross_real[k] = (Lr * Rr) + (Li * Ri);
      fft_cross_img[k] = (Li * Rr) - (Lr * Ri);
    }

    for (uint k = 0; k < fft_size; k++) {
      double phase_gain = 1.0;
      double corr_gain = 1.0;
      double inst_freq_gain = 1.0;
      double kurtosis_gain = 1.0;

      // Correlation
      {
        auto cross_mag = std::hypot(fft_cross_real[k], fft_cross_img[k]);
        auto correlation = cross_mag / ((fft_mag_L[k] * fft_mag_R[k]) + epsilon);

        if (!settings->invertedMode()) {
          corr_gain = sigmoid(correlation / (settings->correlation() * 0.01));
        } else {
          corr_gain = sigmoid((settings->correlation() * 0.01) / std::max(correlation, epsilon));
        }
      }

      // Phase difference
      {
        auto phase_diff = std::abs(std::atan2(fft_cross_img[k], fft_cross_real[k]));

        if (!settings->invertedMode()) {
          phase_gain = sigmoid(phase_diff / (settings->phaseDifference() * std::numbers::pi_v<double> / 180.0));
        } else {
          phase_gain = sigmoid((settings->phaseDifference() * std::numbers::pi_v<double> / 180.0) /
                               std::max(phase_diff, epsilon));
        }
      }

      // Local kurtosis
      {
        auto kurtosis_L = compute_local_kurtosis(k, fft_mag_L.data());
        auto kurtosis_R = compute_local_kurtosis(k, fft_mag_R.data());
        auto kurtosis = std::max(kurtosis_L, kurtosis_R);

        if (!settings->invertedMode()) {
          kurtosis_gain = sigmoid(kurtosis / settings->minKurtosis());
        } else {
          kurtosis_gain = sigmoid(settings->minKurtosis() / std::max(kurtosis, epsilon));
        }
      }

      // Instantaneous frequency
      {
        auto freq_diff = std::abs(calc_instantaneous_frequency(k));

        if (!settings->invertedMode()) {
          inst_freq_gain = sigmoid(freq_diff / settings->maxInstFreq());
        } else {
          inst_freq_gain = sigmoid(settings->maxInstFreq() / std::max(freq_diff, epsilon));
        }
      }

      // Deciding if we should attenuate the frequency

      if ((freqs[k] >= settings->freqStart()) && (freqs[k] <= settings->freqEnd())) {
        auto gain = phase_gain * corr_gain * kurtosis_gain * inst_freq_gain;

        complexL[k][0] *= gain;
        complexL[k][1] *= gain;
        complexR[k][0] *= gain;
        complexR[k][1] *= gain;
      }

      // util::warning(std::format("f = {}, coor_gain = {}, phase_gain = {}", freqs[k], corr_gain, phase_gain));
    }

    fftw_execute(planInvL);
    fftw_execute(planInvR);

    float norm = 1.0F / n_samples;

    for (uint n = 0; n < n_samples; n++) {
      data_L[n] = hanning_window[n] * realL[n] * norm;
      data_R[n] = hanning_window[n] * realR[n] * norm;
    }

    // ----- Overlap-add into OLA buffer
    for (uint n = 0; n < n_samples; n++) {
      ola_L[n] += data_L[n];
      ola_R[n] += data_R[n];
    }

    // ----- Push first hop to output FIFO
    buf_out_L.insert(buf_out_L.end(), ola_L.begin(), ola_L.end() - hop);
    buf_out_R.insert(buf_out_R.end(), ola_R.begin(), ola_R.end() - hop);

    // ----- Shift OLA buffer
    std::move(ola_L.begin() + hop, ola_L.end(), ola_L.begin());
    std::fill(ola_L.begin() + hop, ola_L.end(), 0.0F);

    std::move(ola_R.begin() + hop, ola_R.end(), ola_R.begin());
    std::fill(ola_R.begin() + hop, ola_R.end(), 0.0F);
  }

  if (buf_out_L.size() < n_samples) {
    std::ranges::fill(left_out, 0.0F);
    std::ranges::fill(right_out, 0.0F);
  } else {
    util::copy_bulk(buf_out_L, left_out);
    util::copy_bulk(buf_out_R, right_out);
  }

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (notify_latency) {
    latency_value = static_cast<float>(hop) / static_cast<float>(rate);

    util::debug(std::format("{}{} latency: {} s", log_tag, name.toStdString(), latency_value));

    update_filter_params();

    notify_latency = false;
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void VoiceSuppressor::process([[maybe_unused]] std::span<float>& left_in,
                              [[maybe_unused]] std::span<float>& right_in,
                              [[maybe_unused]] std::span<float>& left_out,
                              [[maybe_unused]] std::span<float>& right_out,
                              [[maybe_unused]] std::span<float>& probe_left,
                              [[maybe_unused]] std::span<float>& probe_right) {}

auto VoiceSuppressor::get_latency_seconds() -> float {
  return 0.0F;
}

void VoiceSuppressor::free_fftw() {
  if (realL != nullptr) {
    fftw_free(realL);
  }

  if (realR != nullptr) {
    fftw_free(realR);
  }

  if (complexL != nullptr) {
    fftw_free(complexL);
  }

  if (complexR != nullptr) {
    fftw_free(complexR);
  }

  if (planL != nullptr) {
    fftw_destroy_plan(planL);
  }

  if (planR != nullptr) {
    fftw_destroy_plan(planR);
  }

  if (planInvL != nullptr) {
    fftw_destroy_plan(planInvL);
  }

  if (planInvR != nullptr) {
    fftw_destroy_plan(planInvR);
  }
}

auto VoiceSuppressor::compute_local_kurtosis(int k, double* magnitude_spectrum) const -> double {
  int W = 1;
  double mean = 0;
  double var = 0;
  double fourth = 0;

  for (int i = k - W; i <= k + W; i++) {
    if (i >= 0 && i < static_cast<int>(fft_size)) {
      mean += magnitude_spectrum[i];
    }
  }

  mean /= ((2 * W) + 1);

  for (int i = k - W; i <= k + W; i++) {
    if (i >= 0 && i < static_cast<int>(fft_size)) {
      double d = magnitude_spectrum[i] - mean;

      var += d * d;
      fourth += d * d * d * d;
    }
  }

  var /= ((2 * W) + 1);
  fourth /= ((2 * W) + 1);

  return fourth / (var * var + epsilon);
}

auto VoiceSuppressor::calc_instantaneous_frequency(const int& k) -> double {
  double wrapped = std::atan2(fft_cross_img[k], fft_cross_real[k]);

  // --- unwrap

  double delta = wrapped - previous_wrapped[k];

  if (delta > std::numbers::pi_v<double>) {
    delta -= 2.0 * std::numbers::pi_v<double>;
  }

  if (delta < -std::numbers::pi_v<double>) {
    delta += 2.0 * std::numbers::pi_v<double>;
  }

  double unwrapped = previous_unwrapped[k] + delta;

  double dphi = unwrapped - previous_unwrapped[k];

  previous_wrapped[k] = wrapped;
  previous_unwrapped[k] = unwrapped;

  // Instantaneous frequency. This is the frequency at which the channel phase is changing

  return dphi / (2.0 * std::numbers::pi_v<double> * block_time);
}

auto VoiceSuppressor::sigmoid(const double& x) -> double {
  return x / (1.0 + std::abs(x));
}