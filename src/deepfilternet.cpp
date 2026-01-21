/**
 * Copyright © 2023-2026 Torge Matthies
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

#include "deepfilternet.hpp"
#include <qnamespace.h>
#include <qobject.h>
#include <algorithm>
#include <format>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <vector>
#include "db_manager.hpp"
#include "easyeffects_db_deepfilternet.h"
#include "ladspa_macros.hpp"
#include "ladspa_wrapper.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "resampler.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

DeepFilterNet::DeepFilterNet(const std::string& tag,
                             pw::Manager* pipe_manager,
                             PipelineType pipe_type,
                             QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::deepfilternet,
                 tags::plugin_package::Package::deepfilternet,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(db::Manager::self().get_plugin_db<db::DeepFilterNet>(
          pipe_type,
          tags::plugin_name::BaseName::deepfilternet + "#" + instance_id)) {
  ladspa_wrapper = std::make_unique<ladspa::LadspaWrapper>("libdeep_filter_ladspa.so", "deep_filter_stereo");

  packageInstalled = ladspa_wrapper->found_plugin();

  if (!packageInstalled) {
    util::debug(std::format("{}libdeep_filter_ladspa is not installed", log_tag));
  }

  init_common_controls<db::DeepFilterNet>(settings);

  BIND_LADSPA_PORT("Post Filter Beta", postFilterBeta, setPostFilterBeta, db::DeepFilterNet::postFilterBetaChanged);
  BIND_LADSPA_PORT("Min Processing Buffer (frames)", minProcessingBuffer, setMinProcessingBuffer,
                   db::DeepFilterNet::minProcessingBufferChanged);

  BIND_LADSPA_PORT_DB_EXPONENTIAL("Attenuation Limit (dB)", attenuationLimit, setAttenuationLimit,
                                  db::DeepFilterNet::attenuationLimitChanged, false);
  BIND_LADSPA_PORT_DB_EXPONENTIAL("Min processing threshold (dB)", minProcessingThreshold, setMinProcessingThreshold,
                                  db::DeepFilterNet::minProcessingThresholdChanged, false);
  BIND_LADSPA_PORT_DB_EXPONENTIAL("Max ERB processing threshold (dB)", maxErbProcessingThreshold,
                                  setMaxErbProcessingThreshold, db::DeepFilterNet::maxErbProcessingThresholdChanged,
                                  false);
  BIND_LADSPA_PORT_DB_EXPONENTIAL("Max DF processing threshold (dB)", maxDfProcessingThreshold,
                                  setMaxDfProcessingThreshold, db::DeepFilterNet::maxDfProcessingThresholdChanged,
                                  false);
}

DeepFilterNet::~DeepFilterNet() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void DeepFilterNet::reset() {
  settings->setDefaults();
}

void DeepFilterNet::setup() {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (rate == 0 || n_samples == 0) {  // some database signals may be emitted before pipewire calls our setup function
    return;
  }

  ready = false;

  if (!ladspa_wrapper->found_plugin()) {
    return;
  }

  resample = rate != 48000;
  resampler_ready = !resample;

  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)

  QMetaObject::invokeMethod(
      baseWorker,
      [this] {
        if (ladspa_wrapper->has_instance()) {
          ladspa_wrapper->deactivate();

          ladspa_wrapper = std::make_unique<ladspa::LadspaWrapper>("libdeep_filter_ladspa.so", "deep_filter_stereo");
        }

        ladspa_wrapper->n_samples = n_samples;
        ladspa_wrapper->create_instance(48000);

        if (resample && !resampler_ready) {
          resampler_inL = std::make_unique<Resampler>(rate, 48000);
          resampler_inR = std::make_unique<Resampler>(rate, 48000);
          resampler_outL = std::make_unique<Resampler>(48000, rate);
          resampler_outR = std::make_unique<Resampler>(48000, rate);

          std::vector<float> dummy(n_samples);

          const auto resampled_inL = resampler_inL->process(dummy);
          const auto resampled_inR = resampler_inR->process(dummy);

          resampled_outL.resize(resampled_inL.size());
          resampled_outR.resize(resampled_inR.size());

          resampler_outL->process(resampled_inL);
          resampler_outR->process(resampled_inR);

          carryover_l.clear();
          carryover_r.clear();
          carryover_l.reserve(4);  // chosen by fair dice roll.
          carryover_r.reserve(4);  // guaranteed to be random.
          carryover_l.push_back(0.0F);
          carryover_r.push_back(0.0F);

          resampler_ready = true;
        }

        std::scoped_lock<std::mutex> lock(data_mutex);

        ready = true;
      },
      Qt::QueuedConnection);

  // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
}

void DeepFilterNet::process(std::span<float>& left_in,
                            std::span<float>& right_in,
                            std::span<float>& left_out,
                            std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (!ready || bypass) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  if (resample) {
    const auto& resampled_inL = resampler_inL->process(left_in);
    const auto& resampled_inR = resampler_inR->process(right_in);

    resampled_outL.resize(resampled_inL.size());
    resampled_outR.resize(resampled_inR.size());

    ladspa_wrapper->n_samples = resampled_inL.size();
    ladspa_wrapper->connect_data_ports(resampled_inL, resampled_inR, resampled_outL, resampled_outR);
  } else {
    ladspa_wrapper->connect_data_ports(left_in, right_in, left_out, right_out);
  }

  ladspa_wrapper->run();

  if (resample) {
    const auto& outL = resampler_outL->process(resampled_outL);
    const auto& outR = resampler_outR->process(resampled_outR);

    const auto carryover_end_l = std::min(carryover_l.size(), left_out.size());
    const auto carryover_end_r = std::min(carryover_r.size(), right_out.size());

    const auto left_offset =
        carryover_end_l + outL.size() > left_out.size() ? carryover_end_l : left_out.size() - outL.size();
    const auto right_offset =
        carryover_end_r + outR.size() > right_out.size() ? carryover_end_r : right_out.size() - outR.size();

    const auto left_count = std::min(outL.size(), left_out.size() - left_offset);
    const auto right_count = std::min(outR.size(), right_out.size() - right_offset);

    std::copy(carryover_l.begin(), carryover_l.begin() + carryover_end_l, left_out.begin());
    std::copy(carryover_r.begin(), carryover_r.begin() + carryover_end_r, right_out.begin());

    carryover_l.erase(carryover_l.begin(), carryover_l.begin() + carryover_end_l);
    carryover_r.erase(carryover_r.begin(), carryover_r.begin() + carryover_end_r);

    std::fill(left_out.begin() + carryover_end_l, left_out.begin() + left_offset, 0);
    std::fill(right_out.begin() + carryover_end_r, right_out.begin() + right_offset, 0);

    std::copy(outL.begin(), outL.begin() + left_count, left_out.begin() + left_offset);
    std::copy(outR.begin(), outR.begin() + right_count, right_out.begin() + right_offset);

    carryover_l.insert(carryover_l.end(), outL.begin() + left_count, outL.end());
    carryover_r.insert(carryover_r.end(), outR.begin() + right_count, outR.end());

    std::fill(left_out.begin() + left_offset + left_count, left_out.end(), 0);
    std::fill(right_out.begin() + right_offset + right_count, right_out.end(), 0);
  }

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void DeepFilterNet::process([[maybe_unused]] std::span<float>& left_in,
                            [[maybe_unused]] std::span<float>& right_in,
                            [[maybe_unused]] std::span<float>& left_out,
                            [[maybe_unused]] std::span<float>& right_out,
                            [[maybe_unused]] std::span<float>& probe_left,
                            [[maybe_unused]] std::span<float>& probe_right) {}

auto DeepFilterNet::get_latency_seconds() -> float {
  return 0.02F + (1.0F / rate);
}

void DeepFilterNet::resetHistory() {
  setup();
}
