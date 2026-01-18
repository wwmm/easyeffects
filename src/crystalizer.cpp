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

#include "crystalizer.hpp"
#include <qlist.h>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <qtypes.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <format>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <vector>
#include "db_manager.hpp"
#include "easyeffects_db_crystalizer.h"
#include "fir_filter_bandpass.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "resampler.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

// NOLINTNEXTLINE
#define BIND_BAND(index)                                                                                    \
  {                                                                                                         \
    band_intensity.at(index) = util::db_to_linear(settings->intensityBand##index());                        \
    band_mute.at(index) = settings->muteBand##index();                                                      \
    band_bypass.at(index) = settings->bypassBand##index();                                                  \
    connect(settings, &db::Crystalizer::intensityBand##index##Changed,                                      \
            [this]() { band_intensity.at(index) = util::db_to_linear(settings->intensityBand##index()); }); \
    connect(settings, &db::Crystalizer::muteBand##index##Changed,                                           \
            [this]() { band_mute.at(index) = settings->muteBand##index(); });                               \
    connect(settings, &db::Crystalizer::bypassBand##index##Changed,                                         \
            [this]() { band_bypass.at(index) = settings->bypassBand##index(); });                           \
  }

Crystalizer::Crystalizer(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::crystalizer,
                 tags::plugin_package::Package::ee,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(db::Manager::self().get_plugin_db<db::Crystalizer>(
          pipe_type,
          tags::plugin_name::BaseName::crystalizer + "#" + instance_id)),
      adaptive_intensities(nbands, 1.0F) {
  for (uint n = 0U; n < nbands; n++) {
    filters.at(n) = std::make_unique<FirFilterBandpass>(log_tag + name.toStdString() + " band" + util::to_string(n));
  }

  std::ranges::fill(band_mute, false);
  std::ranges::fill(band_bypass, false);
  std::ranges::fill(band_intensity, 1.0F);
  std::ranges::fill(band_previous_L, 0.0F);
  std::ranges::fill(band_previous_R, 0.0F);
  std::ranges::fill(env_kurtosis_L, 3.0F);
  std::ranges::fill(env_kurtosis_R, 3.0F);
  std::ranges::fill(env_crest_L, 1.0F);
  std::ranges::fill(env_crest_R, 1.0F);
  std::ranges::fill(env_flux_L, 1.0F);
  std::ranges::fill(env_flux_R, 1.0F);

  auto f_edges = make_geometric_edges(20, 20000);

  freq_centers = compute_band_centers(f_edges);

  for (uint n = 0; n <= nbands; n++) {
    frequencies[n] = f_edges[n];
  }

  init_common_controls<db::Crystalizer>(settings);

  BIND_BAND(0);
  BIND_BAND(1);
  BIND_BAND(2);
  BIND_BAND(3);
  BIND_BAND(4);
  BIND_BAND(5);
  BIND_BAND(6);
  BIND_BAND(7);
  BIND_BAND(8);
  BIND_BAND(9);
  BIND_BAND(10);
  BIND_BAND(11);
  BIND_BAND(12);

  connect(settings, &db::Crystalizer::useFixedQuantumChanged, [&]() { setup(); });

  connect(settings, &db::Crystalizer::oversamplingChanged, [&]() { setup(); });

  connect(settings, &db::Crystalizer::transitionBandChanged, [&]() { setup(); });

  connect(settings, &db::Crystalizer::oversamplingQualityChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    if (resampler_inL) {
      resampler_inL->set_quality(settings->oversamplingQuality());
    }

    if (resampler_inR) {
      resampler_inR->set_quality(settings->oversamplingQuality());
    }

    if (resampler_outL) {
      resampler_outL->set_quality(settings->oversamplingQuality());
    }

    if (resampler_outR) {
      resampler_outR->set_quality(settings->oversamplingQuality());
    }
  });
}

Crystalizer::~Crystalizer() {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  filters_are_ready = false;

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void Crystalizer::reset() {
  settings->setDefaults();
}

void Crystalizer::setup() {
  if (rate == 0 || n_samples == 0) {  // oversamplingChanged may be emitted before pipewire calls our setup function
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  auto same_blocksize = settings->useFixedQuantum() ? n_samples == default_quantum : n_samples == blocksize;

  if (filters_are_ready && rate == current_rate && same_blocksize) {
    return;
  }

  filters_are_ready = false;

  /**
   * As zita uses fftw we have to be careful when reinitializing it.
   * The thread that creates the fftw plan has to be the same that destroys it.
   * Otherwise segmentation faults can happen. As we do not want to do this
   * initializing in the plugin realtime thread we send it to the main thread.
   */

  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)

  QMetaObject::invokeMethod(
      baseWorker,
      [this] {
        if (filters_are_ready) {
          return;
        }

        current_rate = rate;

        do_oversampling = settings->oversampling();

        block_time = static_cast<float>(n_samples) / static_cast<float>(rate);

        attack_coeff = std::exp(-block_time / attack_time);
        release_coeff = std::exp(-block_time / release_time);

        auto blockrate = do_oversampling ? 2 * rate : rate;

        blocksize = do_oversampling ? 2 * n_samples : n_samples;

        blocksize = settings->useFixedQuantum() ? default_quantum : blocksize;

        n_samples_is_power_of_2 = (blocksize & (blocksize - 1U)) == 0 && blocksize != 0U;

        if (!n_samples_is_power_of_2) {
          while ((blocksize & (blocksize - 1U)) != 0 && blocksize > 2U) {
            blocksize--;
          }
        }

        blocksize = std::max<uint>(blocksize, 64);    // zita does not work with less than 64
        blocksize = std::min<uint>(blocksize, 8192);  // zita does not work with more that 8192

        util::debug(std::format("{}{} blocksize: {}", log_tag, name.toStdString(), blocksize));

        notify_latency = true;
        is_first_buffer = true;

        latency_n_frames = 0U;

        buf_in_L.clear();
        buf_in_R.clear();
        buf_out_L.clear();
        buf_out_R.clear();

        data_L.resize(blocksize);
        data_R.resize(blocksize);

        previous_data_L.resize(blocksize);
        previous_data_R.resize(blocksize);

        std::ranges::fill(previous_data_L, 0.0F);
        std::ranges::fill(previous_data_R, 0.0F);

        global_previous_L = 0.0F;
        global_previous_R = 0.0F;

        for (uint n = 0U; n < nbands; n++) {
          band_data_L.at(n).resize(blocksize);
          band_data_R.at(n).resize(blocksize);

          band_second_derivative_L.at(n).resize(blocksize);
          band_second_derivative_R.at(n).resize(blocksize);

          band_previous_data_L.at(n).resize(blocksize);
          band_previous_data_R.at(n).resize(blocksize);

          std::ranges::fill(band_previous_data_L.at(n), 0.0F);
          std::ranges::fill(band_previous_data_R.at(n), 0.0F);

          global_second_derivative_L.resize(blocksize);
          global_second_derivative_R.resize(blocksize);
        }

        for (uint n = 0U; n < nbands; n++) {
          filters.at(n)->set_n_samples(blocksize);
          filters.at(n)->set_rate(blockrate);
          filters.at(n)->set_min_frequency(frequencies.at(n));
          filters.at(n)->set_max_frequency(frequencies.at(n + 1U));
          filters.at(n)->set_transition_band(settings->transitionBand());
          filters.at(n)->setup();
        }

        resampler_inL = std::make_unique<Resampler>(rate, 2 * rate);
        resampler_inR = std::make_unique<Resampler>(rate, 2 * rate);

        resampler_outL = std::make_unique<Resampler>(2 * rate, rate);
        resampler_outR = std::make_unique<Resampler>(2 * rate, rate);

        resampler_inL->set_quality(settings->oversamplingQuality());

        resampler_inR->set_quality(settings->oversamplingQuality());

        resampler_outL->set_quality(settings->oversamplingQuality());

        resampler_outR->set_quality(settings->oversamplingQuality());

        std::scoped_lock<std::mutex> lock(data_mutex);

        filters_are_ready = true;
      },
      Qt::QueuedConnection);

  // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
}

void Crystalizer::process(std::span<float>& left_in,
                          std::span<float>& right_in,
                          std::span<float>& left_out,
                          std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  if (!filters_are_ready) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    if (output_gain != 1.0F) {
      apply_gain(left_out, right_out, output_gain);
    }

    return;
  }

  if (n_samples_is_power_of_2 && blocksize == n_samples && !do_oversampling) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    enhance_peaks(left_out, right_out);
  } else {
    if (!do_oversampling) {
      buf_in_L.insert(buf_in_L.end(), left_in.begin(), left_in.end());
      buf_in_R.insert(buf_in_R.end(), right_in.begin(), right_in.end());
    } else {
      const auto resampled_inL = resampler_inL->process(left_in);
      const auto resampled_inR = resampler_inR->process(right_in);

      buf_in_L.insert(buf_in_L.end(), resampled_inL.begin(), resampled_inL.end());
      buf_in_R.insert(buf_in_R.end(), resampled_inR.begin(), resampled_inR.end());
    }

    // util::warning(std::format("size 1: {}, size 2: {}, size 3: {}", buf_in_L.size(), left_in.size(), data_L.size()));

    while (buf_in_L.size() >= blocksize) {
      util::copy_bulk(buf_in_L, data_L);
      util::copy_bulk(buf_in_R, data_R);

      enhance_peaks(data_L, data_R);

      if (!do_oversampling) {
        buf_out_L.insert(buf_out_L.end(), data_L.begin(), data_L.end());
        buf_out_R.insert(buf_out_R.end(), data_R.begin(), data_R.end());
      } else {
        auto resampled_outL = resampler_outL->process(data_L);
        auto resampled_outR = resampler_outR->process(data_R);

        buf_out_L.insert(buf_out_L.end(), resampled_outL.begin(), resampled_outL.end());
        buf_out_R.insert(buf_out_R.end(), resampled_outR.begin(), resampled_outR.end());
      }
    }

    // copying the processed samples to the output buffers

    if (buf_out_L.size() >= n_samples) {
      util::copy_bulk(buf_out_L, left_out);
      util::copy_bulk(buf_out_R, right_out);
    } else {
      const uint offset = n_samples - buf_out_L.size();

      if (offset != latency_n_frames) {
        latency_n_frames = offset;

        notify_latency = true;
      }

      // Fill beginning with zeros
      std::fill_n(left_out.begin(), offset, 0.0F);
      std::fill_n(right_out.begin(), offset, 0.0F);

      std::ranges::copy(buf_out_L, left_out.begin() + offset);
      std::ranges::copy(buf_out_R, right_out.begin() + offset);

      buf_out_L.clear();
      buf_out_R.clear();
    }
  }

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (notify_latency) {
    latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(std::format("{}{} latency: {} s", log_tag, name.toStdString(), latency_value));

    update_filter_params();

    notify_latency = false;
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void Crystalizer::process([[maybe_unused]] std::span<float>& left_in,
                          [[maybe_unused]] std::span<float>& right_in,
                          [[maybe_unused]] std::span<float>& left_out,
                          [[maybe_unused]] std::span<float>& right_out,
                          [[maybe_unused]] std::span<float>& probe_left,
                          [[maybe_unused]] std::span<float>& probe_right) {}

auto Crystalizer::get_latency_seconds() -> float {
  return this->latency_value;
}

auto Crystalizer::compute_kurtosis(float* data) const -> float {
  float mean = 0.0F;

  for (uint i = 0; i < blocksize; i++) {
    mean += data[i];
  }

  mean /= blocksize;

  // Calculate second and fourth moments

  float m2 = 0.0F;
  float m4 = 0.0F;

  for (uint i = 0; i < blocksize; i++) {
    float deviation = data[i] - mean;
    float deviation2 = deviation * deviation;

    m2 += deviation2;
    m4 += deviation2 * deviation2;
  }

  m2 /= blocksize;
  m4 /= blocksize;

  return (m2 > 1e-6F) ? m4 / (m2 * m2) : 3.0F;
}

auto Crystalizer::compute_crest(float* data) const -> float {
  float rms = 0.0;

  float peak = 0.0F;

  for (uint i = 0; i < blocksize; i++) {
    float v = std::fabs(data[i]);

    rms += v * v;

    peak = std::max(v, peak);
  }

  rms = std::sqrt(rms / blocksize);

  return (rms > 1e-6F) ? (peak / rms) : 1.0F;
}

auto Crystalizer::compute_spectral_flux(float* data, float* previous_data) const -> float {
  float flux = 0.0F;

  for (uint i = 0; i < blocksize; i++) {
    flux += std::abs(data[i] - previous_data[i]);
  }

  for (uint i = 0; i < blocksize; i++) {
    previous_data[i] = data[i];
  }

  flux /= blocksize;

  return flux > 1e-6 ? flux : 1.0F;
}

void Crystalizer::compute_global_crest(float* data, const bool& isLeft) {
  float crest = compute_crest(data);

  auto& env_crest = isLeft ? global_crest_L : global_crest_R;

  float alpha = (crest > env_crest) ? attack_coeff : release_coeff;

  env_crest = (alpha * env_crest) + ((1.0F - alpha) * crest);
}

void Crystalizer::compute_global_kurtosis(float* data, const bool& isLeft) {
  float kurtosis = compute_kurtosis(data);

  auto& env_kurtosis = isLeft ? global_kurtosis_L : global_kurtosis_R;

  float alpha = (kurtosis > env_kurtosis) ? attack_coeff : release_coeff;

  env_kurtosis = (alpha * env_kurtosis) + ((1.0F - alpha) * kurtosis);
}

void Crystalizer::compute_global_flux(float* data, const bool& isLeft) {
  auto& previous_data = isLeft ? previous_data_L : previous_data_R;

  auto flux = compute_spectral_flux(data, previous_data.data());

  float& env_flux = isLeft ? global_flux_L : global_flux_R;

  float alpha = (flux > env_flux) ? attack_coeff : release_coeff;

  env_flux = (alpha * env_flux) + ((1.0F - alpha) * flux);
}

auto Crystalizer::compute_adaptive_intensity(const uint& band_index,
                                             float base_intensity,
                                             float* band_data,
                                             const bool& isLeft) -> float {
  // kurtosis calculation

  float kurtosis = compute_kurtosis(band_data);

  auto& env_kurtosis = isLeft ? env_kurtosis_L[band_index] : env_kurtosis_R[band_index];

  float alpha = (kurtosis > env_kurtosis) ? attack_coeff : release_coeff;

  env_kurtosis = (alpha * env_kurtosis) + ((1.0F - alpha) * kurtosis);

  float kurtosis_ratio = isLeft ? global_kurtosis_L / env_kurtosis : global_kurtosis_R / env_kurtosis;

  // crest calculation

  float crest = compute_crest(band_data);

  auto& env_crest = isLeft ? env_crest_L[band_index] : env_crest_R[band_index];

  float alpha_crest = (crest > env_crest) ? attack_coeff : release_coeff;

  env_crest = (alpha_crest * env_crest) + ((1.0F - alpha_crest) * crest);

  float crest_ratio = isLeft ? global_crest_L / env_crest : global_crest_R / env_crest;

  // spectral flux calculation

  float flux = compute_spectral_flux(
      band_data, isLeft ? band_previous_data_L[band_index].data() : band_previous_data_R[band_index].data());

  auto& env_flux = isLeft ? env_flux_L[band_index] : env_flux_R[band_index];

  float alpha_flux = (flux > env_flux) ? attack_coeff : release_coeff;

  env_flux = (alpha_flux * env_flux) + ((1.0F - alpha_flux) * flux);

  float flux_ratio = isLeft ? global_flux_L / env_flux : global_flux_R / env_flux;

  // intensity calculation

  auto intensity = base_intensity * std::cbrtf(crest_ratio * kurtosis_ratio * flux_ratio);

  // util::warning(std::format("n = {}, intensity = {}, kurtosis_r = {}, crest_r = {}, flux_r = {}", band_index,
  // intensity,
  //                           kurtosis_ratio, crest_ratio, flux_ratio));

  return intensity;
}

auto Crystalizer::extrapolate_next(const std::vector<float>& x) -> float {
  size_t n = x.size();

  if (x.size() < 2) {
    return x.back();
  }

  float xm1 = x[n - 2];
  float xm = x[n - 1];

  return (2.0F * xm) - xm1;
}

auto Crystalizer::extrapolate_next(const std::span<float>& x) -> float {
  size_t n = x.size();

  if (x.size() < 2) {
    return x.back();
  }

  float xm1 = x[n - 2];
  float xm = x[n - 1];

  return (2.0F * xm) - xm1;
}

auto Crystalizer::make_geometric_edges(float fmin, float fmax) -> std::array<float, nbands + 1U> {
  std::array<float, nbands + 1U> edges;

  float gamma = 0.413F;

  float r = fmax / fmin;

  for (unsigned i = 0; i <= nbands; i++) {
    float t = static_cast<float>(i) / nbands;

    edges[i] = fmin * std::pow(r, std::pow(t, gamma));
  }

  return edges;
}

auto Crystalizer::compute_band_centers(const std::array<float, nbands + 1U>& edges) -> std::array<float, nbands> {
  std::array<float, nbands> centers;

  for (size_t i = 0; i < nbands; i++) {
    float low = edges[i];
    float high = edges[i + 1];

    centers[i] = 0.5F * (high + low);
  }

  return centers;
}

float Crystalizer::getBandFrequency(const int& index) {
  return freq_centers[index];
}

QList<float> Crystalizer::getAdaptiveIntensities() {
  return adaptive_intensities;
}
