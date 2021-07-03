/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "test_signals.hpp"

namespace {

void on_process(void* userdata, spa_io_position* position) {
  auto* d = static_cast<TestSignals::data*>(userdata);

  auto n_samples = position->clock.duration;
  auto rate = position->clock.rate.denom;

  if (n_samples == 0 || rate == 0) {
    return;
  }

  if (rate != d->ts->rate || n_samples != d->ts->n_samples) {
    d->ts->rate = rate;
    d->ts->n_samples = n_samples;
  }

  // util::warning("processing: " + std::to_string(n_samples));

  auto* out_left = static_cast<float*>(pw_filter_get_dsp_buffer(d->out_left, n_samples));
  auto* out_right = static_cast<float*>(pw_filter_get_dsp_buffer(d->out_right, n_samples));

  std::span left_out{out_left, out_left + n_samples};
  std::span right_out{out_right, out_right + n_samples};
}

const struct pw_filter_events filter_events = {.process = on_process};

}  // namespace

TestSignals::TestSignals(PipeManager* pipe_manager) : pm(pipe_manager) {
  pf_data.ts = this;

  const auto* filter_name = "pe_test_signals";

  pw_thread_loop_lock(pm->thread_loop);

  auto* props_filter = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_filter, PW_KEY_NODE_NAME, filter_name);
  pw_properties_set(props_filter, PW_KEY_NODE_DESCRIPTION, "easyeffects_filter");
  pw_properties_set(props_filter, PW_KEY_MEDIA_TYPE, "Audio");
  pw_properties_set(props_filter, PW_KEY_MEDIA_CATEGORY, "Filter");
  pw_properties_set(props_filter, PW_KEY_MEDIA_ROLE, "DSP");
  // pw_properties_set(props_filter, PW_KEY_MEDIA_CLASS, "Stream/Output/Audio");

  filter = pw_filter_new(pm->core, filter_name, props_filter);

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

  if (pw_filter_connect(filter, PW_FILTER_FLAG_RT_PROCESS, nullptr, 0) < 0) {
    util::error(log_tag + filter_name + " can not connect the filter to pipewire!");
  }

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);

  do {
    node_id = pw_filter_get_node_id(filter);

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  } while (node_id == SPA_ID_INVALID);

  pw_filter_add_listener(filter, &listener, &filter_events, &pf_data);
}

TestSignals::~TestSignals() {
  util::debug(log_tag + " destroyed");

  spa_hook_remove(&listener);

  pw_thread_loop_lock(pm->thread_loop);

  pw_filter_set_active(filter, false);

  pw_filter_disconnect(filter);

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);
}