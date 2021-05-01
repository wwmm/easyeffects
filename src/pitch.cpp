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

#include "pitch.hpp"

Pitch::Pitch(const std::string& tag,
             const std::string& schema,
             const std::string& schema_path,
             PipeManager* pipe_manager)
    : PluginBase(tag, "pitch", schema, schema_path, pipe_manager) {
  // pitch = gst_element_factory_make("ladspa-ladspa-rubberband-so-rubberband-pitchshifter-stereo", "pitch");

  // if (is_installed(pitch)) {
  //   auto* input_gain = gst_element_factory_make("volume", nullptr);
  //   auto* in_level = gst_element_factory_make("level", "pitch_input_level");
  //   auto* output_gain = gst_element_factory_make("volume", nullptr);
  //   auto* out_level = gst_element_factory_make("level", "pitch_output_level");
  //   auto* audioconvert_in = gst_element_factory_make("audioconvert", nullptr);
  //   auto* audioconvert_out = gst_element_factory_make("audioconvert", nullptr);

  //   gst_bin_add_many(GST_BIN(bin), input_gain, in_level, audioconvert_in, pitch, audioconvert_out, output_gain,
  //                    out_level, nullptr);
  //   gst_element_link_many(input_gain, in_level, audioconvert_in, pitch, audioconvert_out, output_gain, out_level,
  //                         nullptr);

  //   auto* pad_sink = gst_element_get_static_pad(input_gain, "sink");
  //   auto* pad_src = gst_element_get_static_pad(out_level, "src");

  //   gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
  //   gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

  //   gst_object_unref(GST_OBJECT(pad_sink));
  //   gst_object_unref(GST_OBJECT(pad_src));

  //   bind_to_gsettings();

  //   g_settings_bind(settings, "post-messages", in_level, "post-messages", G_SETTINGS_BIND_DEFAULT);
  //   g_settings_bind(settings, "post-messages", out_level, "post-messages", G_SETTINGS_BIND_DEFAULT);

  //   g_settings_bind_with_mapping(settings, "input-gain", input_gain, "volume", G_SETTINGS_BIND_DEFAULT,
  //                                util::db20_gain_to_linear_double, util::linear_double_gain_to_db20, nullptr,
  //                                nullptr);

  //   g_settings_bind_with_mapping(settings, "output-gain", output_gain, "volume", G_SETTINGS_BIND_DEFAULT,
  //                                util::db20_gain_to_linear_double, util::linear_double_gain_to_db20, nullptr,
  //                                nullptr);

  //   // useless write just to force callback call

  //   auto enable = g_settings_get_boolean(settings, "state");

  //   g_settings_set_boolean(settings, "state", enable);
  // }
}

Pitch::~Pitch() {
  util::debug(log_tag + name + " destroyed");

  pw_thread_loop_lock(pm->thread_loop);

  pw_filter_set_active(filter, false);

  pw_filter_disconnect(filter);

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);
}

void Pitch::setup() {
  std::scoped_lock<std::mutex> lock(data_mutex);

  data.resize(2 * n_samples);

  init_strecher();
}

void Pitch::process(std::span<float>& left_in,
                    std::span<float>& right_in,
                    std::span<float>& left_out,
                    std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  apply_gain(left_in, right_in, input_gain);

  for (size_t n = 0; n < left_in.size(); n++) {
    data[n * 2] = left_in[n];
    data[n * 2 + 1] = right_in[n];
  }

  // bs2b.cross_feed(data.data(), n_samples);

  for (size_t n = 0; n < left_out.size(); n++) {
    left_out[n] = data[n * 2];
    right_out[n] = data[n * 2 + 1];
  }

  apply_gain(left_out, right_out, output_gain);

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += sample_duration;

    if (notification_dt >= notification_time_window) {
      notify();

      notification_dt = 0.0F;
    }
  }
}

void Pitch::init_strecher() {
  delete strecher;

  RubberBand::RubberBandStretcher::Options options =
      RubberBand::RubberBandStretcher::OptionProcessRealTime | RubberBand::RubberBandStretcher::OptionPitchHighQuality |
      RubberBand::RubberBandStretcher::OptionChannelsTogether | RubberBand::RubberBandStretcher::OptionPhaseIndependent;

  strecher = new RubberBand::RubberBandStretcher(rate, 2, options);

  strecher->setFormantOption(formant_preserving ? RubberBand::RubberBandStretcher::OptionFormantPreserved
                                                : RubberBand::RubberBandStretcher::OptionFormantShifted);

  strecher->setTimeRatio(time_ratio);
  strecher->setPitchScale(pitch_scale);
}

// g_settings_bind_with_mapping(settings, "cents", pitch, "cents", G_SETTINGS_BIND_GET, util::double_to_float,
// nullptr,
//                              nullptr, nullptr);

// g_settings_bind(settings, "semitones", pitch, "semitones", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "octaves", pitch, "octaves", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "crispness", pitch, "crispness", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "formant-preserving", pitch, "formant-preserving", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "faster", pitch, "faster", G_SETTINGS_BIND_DEFAULT);
