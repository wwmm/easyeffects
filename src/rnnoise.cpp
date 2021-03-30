/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "rnnoise.hpp"

RNNoise::RNNoise(const std::string& tag,
                 const std::string& schema,
                 const std::string& schema_path,
                 PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::rnnoise, schema, schema_path, pipe_manager),
      data_L(blocksize, 0),
      data_R(blocksize, 0) {
  //   auto* audioresample_in = gst_element_factory_make("audioresample", "rnnoise_audioresample_in");
  //   auto* audioresample_out = gst_element_factory_make("audioresample", "rnnoise_audioresample_out");
  //   adapter = gst_element_factory_make("peadapter", nullptr);
  //   adapter_out = gst_element_factory_make("peadapter", nullptr);

  //   gst_bin_add_many(GST_BIN(bin), input_gain, in_level, audioresample_in, capsfilter_in, adapter, rnnoise,
  //   adapter_out,
  //                    audioresample_out, capsfilter_out, output_gain, out_level, nullptr);

  //   gst_element_link_many(input_gain, in_level, audioresample_in, capsfilter_in, adapter, rnnoise, adapter_out,
  //                         audioresample_out, capsfilter_out, output_gain, out_level, nullptr);

  //   auto* pad_sink = gst_element_get_static_pad(input_gain, "sink");
  //   auto* pad_src = gst_element_get_static_pad(out_level, "src");

  //   gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
  //   gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

  //   gst_object_unref(GST_OBJECT(pad_sink));
  //   gst_object_unref(GST_OBJECT(pad_src));

  //   g_object_set(adapter, "blocksize", 480, nullptr);

  //   set_caps_in();

  //   g_signal_connect(adapter, "notify::n-input-samples", G_CALLBACK(on_n_input_samples_changed), this);
}

RNNoise::~RNNoise() {
  util::debug(log_tag + name + " destroyed");

  pw_thread_loop_lock(pm->thread_loop);

  pw_filter_set_active(filter, false);

  pw_filter_disconnect(filter);

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);

  std::lock_guard<std::mutex> guard(rnnoise_mutex);
}

void RNNoise::setup() {
  resample = rate != rnnoise_rate;

  resampler_inL = std::make_unique<Resampler>(rate, rnnoise_rate);
  resampler_inR = std::make_unique<Resampler>(rate, rnnoise_rate);

  resampler_outL = std::make_unique<Resampler>(rnnoise_rate, rate);
  resampler_outR = std::make_unique<Resampler>(rnnoise_rate, rate);

  std::lock_guard<std::mutex> guard(rnnoise_mutex);
}

void RNNoise::process(std::span<float>& left_in,
                      std::span<float>& right_in,
                      std::span<float>& left_out,
                      std::span<float>& right_out) {
  if (bypass) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  // if (resample) {
  auto resampled_inL = resampler_inL->process(left_in, false);
  auto resampled_inR = resampler_inR->process(right_in, false);

  auto resampled_outL = resampler_outL->process(resampled_inL, false);
  auto resampled_outR = resampler_outR->process(resampled_inR, false);

  // std::copy(resampled_outL.begin(), resampled_outL.end(), left_out.begin());
  // std::copy(resampled_outR.begin(), resampled_outR.end(), right_out.begin());

  for (auto v : resampled_outL) {
    deque_out_L.emplace_back(v);
  }

  for (auto v : resampled_outR) {
    deque_out_R.emplace_back(v);
  }

  // if (deque_out_L.size() > left_out.size()) {
  //   for (float& v : left_out) {
  //     v = deque_out_L[0];

  //     deque_out_L.pop_front();
  //   }

  //   for (float& v : right_out) {
  //     v = deque_out_R[0];

  //     deque_out_R.pop_front();
  //   }
  // } else {
  //   for (int n = 0; n < left_out.size(); n++) {
  //     if (n < left_out.size() - deque_out_L.size()) {
  //       left_out[n] = 0.0F;
  //     } else {
  //       left_out[n] = deque_out_L[0];

  //       deque_out_L.pop_front();
  //     }
  //   }
  // }

  // util::warning(std::to_string(resampled_outL.size()));
  // }

  // std::copy(left_in.begin(), left_in.end(), left_out.begin());
  // std::copy(right_in.begin(), right_in.end(), right_out.begin());

  std::lock_guard<std::mutex> guard(rnnoise_mutex);

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += sample_duration;

    if (notification_dt >= notification_time_window) {
      notify();

      notification_dt = 0.0F;
    }
  }
}

// g_settings_bind(settings, "model-path", rnnoise, "model-path", G_SETTINGS_BIND_DEFAULT);
