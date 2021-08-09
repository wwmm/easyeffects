/*
 *  Copyright © 2017-2022 Wellington Wallace
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

    d->ts->sine_phase = 0.0F;
  }

  // util::warning("processing: " + std::to_string(n_samples));

  auto* out_left = static_cast<float*>(pw_filter_get_dsp_buffer(d->out_left, n_samples));
  auto* out_right = static_cast<float*>(pw_filter_get_dsp_buffer(d->out_right, n_samples));

  std::span left_out{out_left, out_left + n_samples};
  std::span right_out{out_right, out_right + n_samples};

  for (uint n = 0U; n < n_samples; n++) {
    float signal = 0.0F;
    left_out[n] = 0.0F;
    right_out[n] = 0.0F;

    switch (d->ts->signal_type) {
      case TestSignalType::sine_wave: {
        d->ts->sine_phase += 2.0F * std::numbers::pi_v<float> * d->ts->sine_frequency / static_cast<float>(rate);

        signal = 0.5F * sinf(d->ts->sine_phase);

        break;
      }
      case TestSignalType::gaussian: {
        signal = d->ts->white_noise();

        break;
      }
      case TestSignalType::pink: {
        break;
      }
    }

    if (d->ts->create_left_channel) {
      left_out[n] = signal;
    }

    if (d->ts->create_right_channel) {
      right_out[n] = signal;
    }
  }

  if (d->ts->sine_phase > 2.0F * std::numbers::pi_v<float>) {
    d->ts->sine_phase -= 2.0F * std::numbers::pi_v<float>;
  }
}

const struct pw_filter_events filter_events = {.process = on_process};

}  // namespace

TestSignals::TestSignals(PipeManager* pipe_manager) : pm(pipe_manager), random_generator(rd()) {
  pf_data.ts = this;

  const auto* filter_name = "pe_test_signals";

  pw_thread_loop_lock(pm->thread_loop);

  auto* props_filter = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_filter, PW_KEY_NODE_NAME, filter_name);
  pw_properties_set(props_filter, PW_KEY_NODE_DESCRIPTION, "easyeffects_filter");
  pw_properties_set(props_filter, PW_KEY_NODE_DRIVER, "true");
  pw_properties_set(props_filter, PW_KEY_MEDIA_TYPE, "Audio");
  pw_properties_set(props_filter, PW_KEY_MEDIA_CATEGORY, "Source");
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

void TestSignals::set_state(const bool& state) {
  sine_phase = 0.0F;

  if (state) {
    auto links = pm->link_nodes(node_id, pm->pe_sink_node.id, false, false);

    for (const auto& link : links) {
      list_proxies.emplace_back(link);
    }
  } else {
    pm->destroy_links(list_proxies);

    list_proxies.clear();
  }
}

void TestSignals::set_frequency(const float& value) {
  sine_frequency = value;

  sine_phase = 0.0F;
}

auto TestSignals::white_noise() -> float {
  auto v = normal_distribution(random_generator);

  v = (v > 1.0F) ? 1.0F : v;

  v = (v < -1.0F) ? -1.0F : v;

  return v;
}
