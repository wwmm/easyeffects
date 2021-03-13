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

#include "plugin_base.hpp"

namespace {

void on_process(void* userdata, spa_io_position* position) {
  auto* d = static_cast<PluginBase::data*>(userdata);

  auto n_samples = position->clock.duration;
  auto rate = position->clock.rate.denom;

  if (rate != d->pb->rate || n_samples != d->pb->n_samples) {
    d->pb->rate = rate;
    d->pb->n_samples = n_samples;
    d->pb->sample_duration = static_cast<float>(n_samples) / rate;

    d->pb->setup();
  }

  // util::warning("processing: " + std::to_string(n_samples));

  auto* in_left = static_cast<float*>(pw_filter_get_dsp_buffer(d->in_left, n_samples));
  auto* in_right = static_cast<float*>(pw_filter_get_dsp_buffer(d->in_right, n_samples));

  auto* out_left = static_cast<float*>(pw_filter_get_dsp_buffer(d->out_left, n_samples));
  auto* out_right = static_cast<float*>(pw_filter_get_dsp_buffer(d->out_right, n_samples));

  std::vector<float> left_in{in_left, in_left + n_samples};
  std::span left_out{out_left, out_left + n_samples};
  std::vector<float> right_in{in_right, in_right + n_samples};
  std::span right_out{out_right, out_right + n_samples};

  d->pb->process(left_in, right_in, left_out, right_out);
}

const struct pw_filter_events filter_events = {.process = on_process};

}  // namespace

PluginBase::PluginBase(std::string tag,
                       std::string plugin_name,
                       const std::string& schema,
                       const std::string& schema_path,
                       PipeManager* pipe_manager)
    : log_tag(std::move(tag)),
      name(std::move(plugin_name)),
      settings(Gio::Settings::create(schema.c_str(), schema_path.c_str())),
      pm(pipe_manager) {
  pf_data.pb = this;

  auto* props_filter = pw_properties_new(nullptr, nullptr);

  name.insert(0, "pe_filter_");

  pw_properties_set(props_filter, PW_KEY_NODE_NAME, name.c_str());
  pw_properties_set(props_filter, PW_KEY_NODE_NICK, log_tag.c_str());
  pw_properties_set(props_filter, PW_KEY_NODE_DESCRIPTION, "pulseeffects_filter");
  pw_properties_set(props_filter, PW_KEY_MEDIA_TYPE, "Audio");
  pw_properties_set(props_filter, PW_KEY_MEDIA_CATEGORY, "Filter");
  pw_properties_set(props_filter, PW_KEY_MEDIA_ROLE, "DSP");

  filter = pw_filter_new(pm->core, plugin_name.c_str(), props_filter);

  pw_filter_add_listener(filter, &listener, &filter_events, &pf_data);

  // left channel input

  auto* props_in_left = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_in_left, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_in_left, PW_KEY_PORT_NAME, "input_fl");
  pw_properties_set(props_in_left, "audio.channel", "FL");

  pf_data.in_left = static_cast<port*>(pw_filter_add_port(filter, PW_DIRECTION_INPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                                                          sizeof(port), props_in_left, nullptr, 0));

  // right channel input

  auto* props_in_right = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_in_right, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_in_right, PW_KEY_PORT_NAME, "input_fr");
  pw_properties_set(props_in_right, "audio.channel", "FR");

  pf_data.in_right = static_cast<port*>(pw_filter_add_port(filter, PW_DIRECTION_INPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                                                           sizeof(port), props_in_right, nullptr, 0));

  // left channel output

  auto* props_out_left = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_out_left, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_out_left, PW_KEY_PORT_NAME, "output_fl");
  pw_properties_set(props_out_left, "audio.channel", "FL");

  pf_data.out_left = static_cast<port*>(pw_filter_add_port(filter, PW_DIRECTION_OUTPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                                                           sizeof(port), props_out_left, nullptr, 0));

  // right channel output

  auto* props_out_right = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_out_right, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_out_right, PW_KEY_PORT_NAME, "output_fr");
  pw_properties_set(props_out_right, "audio.channel", "FR");

  pf_data.out_right = static_cast<port*>(pw_filter_add_port(
      filter, PW_DIRECTION_OUTPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS, sizeof(port), props_out_right, nullptr, 0));

  pw_thread_loop_lock(pm->thread_loop);

  if (pw_filter_connect(filter, PW_FILTER_FLAG_RT_PROCESS, nullptr, 0) < 0) {
    util::error(log_tag + "can not connect the filter to pipewire!");
  }

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);

  do {
    node_id = pw_filter_get_node_id(filter);

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  } while (node_id == SPA_ID_INVALID);
}

PluginBase::~PluginBase() {
  spa_hook_remove(&listener);
}

auto PluginBase::get_node_id() const -> uint {
  return node_id;
}

void PluginBase::setup() {}

void PluginBase::process(const std::vector<float>& left_in,
                         const std::vector<float>& right_in,
                         std::span<float>& left_out,
                         std::span<float>& right_out) {}

void PluginBase::get_peaks(const std::vector<float>& left_in,
                           const std::vector<float>& right_in,
                           std::span<float>& left_out,
                           std::span<float>& right_out) {
  if (!post_messages) {
    return;
  }

  // input level

  float peak_l = *std::max_element(left_in.begin(), left_in.end());
  float peak_r = *std::max_element(right_in.begin(), right_in.end());

  input_peak_left = (peak_l > input_peak_left) ? peak_l : input_peak_left;
  input_peak_right = (peak_r > input_peak_right) ? peak_r : input_peak_right;

  // output level

  peak_l = *std::max_element(left_out.begin(), left_out.end());
  peak_r = *std::max_element(right_out.begin(), right_out.end());

  output_peak_left = (peak_l > output_peak_left) ? peak_l : output_peak_left;
  output_peak_right = (peak_r > output_peak_right) ? peak_r : output_peak_right;
}

void PluginBase::notify() {
  float input_peak_db_l = util::linear_to_db(input_peak_left);
  float input_peak_db_r = util::linear_to_db(input_peak_right);

  float output_peak_db_l = util::linear_to_db(output_peak_left);
  float output_peak_db_r = util::linear_to_db(output_peak_right);

  Glib::signal_idle().connect_once([=, this] { input_level.emit(input_peak_db_l, input_peak_db_r); });
  Glib::signal_idle().connect_once([=, this] { output_level.emit(output_peak_db_l, output_peak_db_r); });

  input_peak_left = util::minimum_linear_level;
  input_peak_right = util::minimum_linear_level;
  output_peak_left = util::minimum_linear_level;
  output_peak_right = util::minimum_linear_level;
}