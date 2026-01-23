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

        prev_wrapped_L.resize(fft_size, 0.0);
        prev_wrapped_R.resize(fft_size, 0.0);
        prev_unwrapped_L.resize(fft_size, 0.0);
        prev_unwrapped_R.resize(fft_size, 0.0);

        fft_mag_L.resize(fft_size, 0.0);
        fft_mag_R.resize(fft_size, 0.0);

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
    }

    // double flatness_L = compute_spectral_flatness(fft_mag_L.data());
    // double flatness_R = compute_spectral_flatness(fft_mag_R.data());

    // double flatness = std::max(flatness_L, flatness_R);

    for (uint k = 0; k < fft_size; k++) {
      float Lr = complexL[k][0];
      float Li = complexL[k][1];
      float Rr = complexR[k][0];
      float Ri = complexR[k][1];

      // Power spectra

      double P_L = (Lr * Lr) + (Li * Li);
      double P_R = (Rr * Rr) + (Ri * Ri);

      // Inner product between the left channel and the complexity conjugated of the right channel

      double cross_real = (Lr * Rr) + (Li * Ri);
      double cross_imag = (Li * Rr) - (Lr * Ri);

      // Correlation

      double cross_mag = std::sqrt((cross_real * cross_real) + (cross_imag * cross_imag));
      double correlation = cross_mag / std::sqrt((P_L * P_R) + 1e-9);

      // Phase difference

      double phase_diff = std::atan2(cross_imag, cross_real);

      double abs_phase_diff = std::abs(phase_diff);

      // Local kurtosis

      double kurtosis_L = compute_local_kurtosis(k, fft_mag_L.data());
      double kurtosis_R = compute_local_kurtosis(k, fft_mag_R.data());
      double kurtosis = std::sqrt(kurtosis_L * kurtosis_R);

      // Local crest

      // double crest_L = compute_local_crest(k, fft_mag_L.data());
      // double crest_R = compute_local_crest(k, fft_mag_R.data());
      // double crest = std::sqrt(crest_L * crest_R);

      // double wrapped_L = std::atan2(Li, Lr);
      // double wrapped_R = std::atan2(Ri, Rr);

      // --- unwrap left
      // double delta_L = wrapped_L - prev_wrapped_L[k];
      // if (delta_L > std::numbers::pi_v<double>) {
      //   delta_L -= 2.0 * std::numbers::pi_v<double>;
      // }
      // if (delta_L < -std::numbers::pi_v<double>) {
      //   delta_L += 2.0 * std::numbers::pi_v<double>;
      // }

      // double unwrapped_L = prev_unwrapped_L[k] + delta_L;

      // --- unwrap right
      // double delta_R = wrapped_R - prev_wrapped_R[k];
      // if (delta_R > std::numbers::pi_v<double>) {
      //   delta_R -= 2.0 * std::numbers::pi_v<double>;
      // }
      // if (delta_R < -std::numbers::pi_v<double>) {
      //   delta_R += 2.0 * std::numbers::pi_v<double>;
      // }

      // double unwrapped_R = prev_unwrapped_R[k] + delta_R;

      // --- instantaneous frequency
      // double inst_freq_L = delta_L / (2.0 * std::numbers::pi_v<double> * block_time);
      // double inst_freq_R = delta_R / (2.0 * std::numbers::pi_v<double> * block_time);
      // double freq_diff = std::abs(inst_freq_L - inst_freq_R);

      // --- store state
      // prev_wrapped_L[k] = wrapped_L;
      // prev_unwrapped_L[k] = unwrapped_L;
      // prev_wrapped_R[k] = wrapped_R;
      // prev_unwrapped_R[k] = unwrapped_R;

      // util::warning(std::format("f = {}, coor = {}, kurtosis = {}, crest = {}, freq_diff = {}", freqs[k],
      // correlation,
      //                           kurtosis, crest, freq_diff));

      // Deciding if we should attenuate the frequency

      bool freq_cond = (freqs[k] >= settings->freqStart()) && (freqs[k] <= settings->freqEnd());
      bool corr_cond = correlation >= settings->correlation() * 0.01;
      bool phase_cond = abs_phase_diff <= settings->phaseDifference() * std::numbers::pi_v<double> / 180.0;
      // bool flatness_cond = flatness <= settings->maxFlatness();
      bool kurtosis_cond = kurtosis >= settings->minKurtosis();
      // bool crest_cond = crest >= settings->minKurtosis();

      double mask = 0;

      if (freq_cond && corr_cond && phase_cond && kurtosis_cond) {
        complexL[k][0] *= mask;
        complexL[k][1] *= mask;
        complexR[k][0] *= mask;
        complexR[k][1] *= mask;

        // util::warning(std::format("f = {}, coor = {}, phase = {}", freqs[k], correlation, phase_diff));

        // util::warning(std::format("f = {}, flat_L = {}, flat_R = {}, env_flatness = {}", freqs[k], flatness_L,
        //                           flatness_R, env_flatness));
      }
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

auto VoiceSuppressor::compute_spectral_flatness(double* magnitude_spectrum) const -> double {
  uint count = 0;
  double log_sum = 0.0;
  double linear_sum = 0.0;

  constexpr double epsilon = 1e-10;  // Small value to avoid log(0)

  for (uint i = 0; i < fft_size; i++) {
    bool freq_cond = (freqs[i] >= settings->freqStart()) && (freqs[i] <= settings->freqEnd());

    if (!freq_cond) {
      continue;
    }

    double mag = magnitude_spectrum[i] + epsilon;

    log_sum += std::log(mag);

    linear_sum += mag;

    count++;
  }

  if (linear_sum <= epsilon) {
    return 0.0;
  }

  count = count == 0 ? 1 : count;

  // Geometric mean: exp( (1/N) * Σ log(magnitude) )
  double geometric_mean = std::exp(log_sum / count);

  // Arithmetic mean: (1/N) * Σ magnitude
  double arithmetic_mean = linear_sum / count;

  // Spectral flatness: geometric mean / arithmetic mean
  return geometric_mean / arithmetic_mean;
}

auto VoiceSuppressor::compute_local_kurtosis(int k, double* magnitude_spectrum) const -> double {
  uint W = 6;
  double mean = 0;
  double var = 0;
  double fourth = 0;

  for (uint i = k - W; i <= k + W; ++i) {
    if (i > 0 && i < fft_size) {
      mean += magnitude_spectrum[i];
    }
  }

  mean /= ((2 * W) + 1);

  for (uint i = k - W; i <= k + W; ++i) {
    if (i > 0 && i < fft_size) {
      double d = magnitude_spectrum[i] - mean;
      var += d * d;
      fourth += d * d * d * d;
    }
  }

  var /= ((2 * W) + 1);
  fourth /= ((2 * W) + 1);

  return fourth / (var * var + 1e-9);
}

auto VoiceSuppressor::compute_local_crest(int k, double* magnitude_spectrum) const -> double {
  uint W = 6;
  double rms = 0;
  double peak = 0;

  for (uint i = k - W; i <= k + W; ++i) {
    if (i > 0 && i < fft_size) {
      auto v = std::fabs(magnitude_spectrum[i]);

      rms += v * v;

      peak = std::max(v, peak);
    }
  }

  rms = sqrt(rms / ((2 * W) + 1));

  return (rms > 1e-6) ? (peak / rms) : 1.0;
}