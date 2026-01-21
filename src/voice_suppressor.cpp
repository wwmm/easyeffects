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

  connect(settings, &db::VoiceSuppressor::attackTimeChanged,
          [&]() { attack_coeff = std::exp(-block_time / settings->attackTime()); });

  connect(settings, &db::VoiceSuppressor::releaseTimeChanged,
          [&]() { release_coeff = std::exp(-block_time / settings->releaseTime()); });
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

void VoiceSuppressor::setup() {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (rate == 0 || n_samples == 0) {  // some database signals may be emitted before pipewire calls our setup function
    return;
  }

  ready = false;

  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
  QMetaObject::invokeMethod(
      baseWorker,
      [this] {
        if (ready) {
          return;
        }

        std::scoped_lock<std::mutex> lock(data_mutex);

        block_time = static_cast<float>(n_samples) / static_cast<float>(rate);

        attack_coeff = std::exp(-block_time / settings->attackTime());
        release_coeff = std::exp(-block_time / settings->releaseTime());

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
        env_mask.resize(fft_size);
        hanning_window.resize(n_samples);

        std::ranges::fill(env_mask, 1.0F);

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
      realL[n] = hanning_window[n] * static_cast<double>(data_L[n]);
      realR[n] = hanning_window[n] * static_cast<double>(data_R[n]);
    }

    fftw_execute(planL);
    fftw_execute(planR);

    for (uint k = 0; k < fft_size; k++) {
      float Lr = complexL[k][0];
      float Li = complexL[k][1];
      float Rr = complexR[k][0];
      float Ri = complexR[k][1];

      // Power spectra

      float P_L = (Lr * Lr) + (Li * Li);
      float P_R = (Rr * Rr) + (Ri * Ri);

      // Inner product between the left channel and the complexity conjugated of the right channel

      float cross_real = (Lr * Rr) + (Li * Ri);
      float cross_imag = (Li * Rr) - (Lr * Ri);

      // Correlation

      float cross_mag = std::sqrt((cross_real * cross_real) + (cross_imag * cross_imag));
      float correlation = cross_mag / std::sqrt((P_L * P_R) + 1e-9);

      // Phase difference

      float phase_diff = std::atan2(cross_imag, cross_real);
      float abs_phase_diff = std::abs(phase_diff);

      // Deciding if we should attenuate the frequency

      bool freq_cond = (freqs[k] >= settings->freqStart()) && (freqs[k] <= settings->freqEnd());
      bool corr_cond = correlation >= settings->correlation() * 0.01F;
      bool phase_cond = abs_phase_diff <= settings->phaseDifference() * std::numbers::pi_v<float> / 180.0F;

      if (freq_cond && corr_cond && phase_cond) {
        float phase_norm = 1.0F - (abs_phase_diff / std::numbers::pi_v<float>);

        float mask = 1.0F - std::sqrt(correlation * phase_norm);

        // smoothing the correlation

        auto& e_mask = env_mask[k];

        float alpha_mask = (mask < e_mask) ? attack_coeff : release_coeff;

        e_mask = (alpha_mask * e_mask) + ((1.0F - alpha_mask) * mask);

        float Mr = 0.5F * (Lr + Rr);
        float Mi = 0.5F * (Li + Ri);
        float Sr = 0.5F * (Lr - Rr);
        float Si = 0.5F * (Li - Ri);

        Mr *= e_mask;
        Mi *= e_mask;

        complexL[k][0] = Mr + Sr;
        complexL[k][1] = Mi + Si;
        complexR[k][0] = Mr - Sr;
        complexR[k][1] = Mi - Si;

        // util::warning(
        //     std::format("f = {}, coor = {}, phase = {}, mask = {}", freqs[k], correlation, phase_diff, e_mask));
      }
    }

    fftw_execute(planInvL);
    fftw_execute(planInvR);

    float norm = 1.0F / n_samples;

    for (uint n = 0; n < n_samples; n++) {
      data_L[n] = realL[n] * norm;
      data_R[n] = realR[n] * norm;
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
