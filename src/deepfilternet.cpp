/*
 *  Copyright © 2023 Torge Matthies
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "deepfilternet.hpp"

// #include <algorithm>
// #include <string>

DeepFilterNet::DeepFilterNet(const std::string& tag,
                             const std::string& schema,
                             const std::string& schema_path,
                             PipeManager* pipe_manager)
    : PluginBase(tag,
                 tags::plugin_name::deepfilternet,
                 tags::plugin_package::deepfilternet,
                 schema,
                 schema_path,
                 pipe_manager) {
  ladspa_wrapper = std::make_unique<ladspa::LadspaWrapper>("libdeep_filter_ladspa.so", "deep_filter_stereo");

  package_installed = ladspa_wrapper->found_plugin();

  if (!package_installed) {
    util::debug(log_tag + "libdeep_filter_ladspa is not installed");
  }

  ladspa_wrapper->bind_key_double_db_exponential<"Attenuation Limit (dB)", "attenuation-limit", false>(settings);

  ladspa_wrapper->bind_key_double_db_exponential<"Min processing threshold (dB)", "min-processing-threshold", false>(
      settings);

  ladspa_wrapper
      ->bind_key_double_db_exponential<"Max ERB processing threshold (dB)", "max-erb-processing-threshold", false>(
          settings);

  ladspa_wrapper
      ->bind_key_double_db_exponential<"Max DF processing threshold (dB)", "max-df-processing-threshold", false>(
          settings);

  ladspa_wrapper->bind_key_int<"Min Processing Buffer (frames)", "min-processing-buffer">(settings);

  ladspa_wrapper->bind_key_double<"Post Filter Beta", "post-filter-beta">(settings);

  setup_input_output_gain();
}

DeepFilterNet::~DeepFilterNet() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  util::debug(log_tag + name + " destroyed");
}

void DeepFilterNet::setup() {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (!ladspa_wrapper->found_plugin()) {
    return;
  }

  util::idle_add([&, this] {
    ladspa_wrapper->n_samples = n_samples;

    if (ladspa_wrapper->get_rate() != 48000) {
      ladspa_wrapper->create_instance(48000);
      ladspa_wrapper->activate();
    }

    resample = rate != 48000;

    if (resample) {
      resampler_inL = std::make_unique<Resampler>(rate, 48000);
      resampler_inR = std::make_unique<Resampler>(rate, 48000);
      resampler_outL = std::make_unique<Resampler>(48000, rate);
      resampler_outR = std::make_unique<Resampler>(48000, rate);

      std::vector<float> dummy(n_samples);

      const auto resampled_inL = resampler_inL->process(dummy, false);
      const auto resampled_inR = resampler_inR->process(dummy, false);

      resampled_outL.resize(resampled_inL.size());
      resampled_outR.resize(resampled_inR.size());
      resampler_outL->process(resampled_inL, false);
      resampler_outR->process(resampled_inR, false);
    }
  });
}

void DeepFilterNet::process(std::span<float>& left_in,
                            std::span<float>& right_in,
                            std::span<float>& left_out,
                            std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (!ladspa_wrapper->found_plugin() || !ladspa_wrapper->has_instance() || bypass) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  if (resample) {
    const auto resampled_inL = resampler_inL->process(left_in, false);
    const auto resampled_inR = resampler_inR->process(right_in, false);
    ladspa_wrapper->connect_data_ports(resampled_inL, resampled_inR, resampled_outL, resampled_outR);
  } else {
    ladspa_wrapper->connect_data_ports(left_in, right_in, left_out, right_out);
  }

  ladspa_wrapper->run();

  if (resample) {
    const auto outL = resampler_outL->process(resampled_outL, false);
    const auto outR = resampler_outR->process(resampled_outR, false);
    std::copy(outL.begin(), outL.begin() + std::min(outL.size(), left_out.size()), left_out.begin());
    std::copy(outR.begin(), outR.begin() + std::min(outR.size(), right_out.size()), right_out.begin());
  }

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    if (send_notifications) {
      notify();
    }
  }
}

auto DeepFilterNet::get_latency_seconds() -> float {
  return 0.02F;
}
