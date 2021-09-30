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

#include "output_stream.hpp"

namespace {

void playback_destroy(void* userdata) {
  auto* os = static_cast<OutputStream*>(userdata);

  spa_hook_remove(&os->playback_listener);

  os->playback = nullptr;
}

void playback_param_changed(void* userdata, uint32_t id, const struct spa_pod* param) {
  // auto* os = static_cast<OutputStream*>(userdata);

  // switch (id) {
  //   case SPA_PARAM_Latency:
  //     param_latency_changed(os, param, os->capture);
  //     break;
  // }
}

const struct pw_stream_events out_stream_events = {
    .destroy = playback_destroy,
    .param_changed = playback_param_changed,
};

void on_process(void* userdata, spa_io_position* position) {
  auto* d = static_cast<OutputStream::data*>(userdata);

  const auto& n_samples = position->clock.duration;
  const auto& rate = position->clock.rate.denom;

  if (n_samples == 0 || rate == 0) {
    return;
  }

  if (rate != d->os->rate || n_samples != d->os->n_samples) {
    d->os->rate = rate;
    d->os->n_samples = n_samples;
    d->os->sample_duration = static_cast<float>(n_samples) / static_cast<float>(rate);

    d->os->setup();
  }

  auto* in_left = static_cast<float*>(pw_filter_get_dsp_buffer(d->in_left, n_samples));
  auto* in_right = static_cast<float*>(pw_filter_get_dsp_buffer(d->in_right, n_samples));

  std::span left_in{in_left, in_left + n_samples};
  std::span right_in{in_right, in_right + n_samples};

  d->os->process(left_in, right_in);

  pw_buffer* b = nullptr;

  if ((b = pw_stream_dequeue_buffer(d->os->playback)) == nullptr) {
    util::warning("out of buffers");

    return;
  }

  auto* buffer = b->buffer;
}

const struct pw_filter_events filter_events = {.process = on_process};

}  // namespace

OutputStream::OutputStream(std::string tag, PipeManager* pipe_manager)
    : log_tag(std::move(tag)), filter_name("output_level"), stream_name("output_stream"), pm(pipe_manager) {
  pf_data.os = this;

  const auto& f_name = "ee_" + log_tag.substr(0, log_tag.size() - 2) + "_" + filter_name;

  // initializing the filter

  pm->lock();

  auto* props_filter = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_filter, PW_KEY_NODE_NAME, f_name.c_str());
  pw_properties_set(props_filter, PW_KEY_NODE_NICK, filter_name.c_str());
  pw_properties_set(props_filter, PW_KEY_NODE_DESCRIPTION, "easyeffects_filter");
  pw_properties_set(props_filter, PW_KEY_NODE_GROUP, "easyeffects.output.stream");
  pw_properties_set(props_filter, PW_KEY_MEDIA_TYPE, "Audio");
  pw_properties_set(props_filter, PW_KEY_MEDIA_CATEGORY, "Filter");
  pw_properties_set(props_filter, PW_KEY_MEDIA_ROLE, "DSP");

  filter = pw_filter_new(pm->core, f_name.c_str(), props_filter);

  // left channel input

  auto* props_in_left = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_in_left, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_in_left, PW_KEY_PORT_NAME, "input_FL");
  pw_properties_set(props_in_left, "audio.channel", "FL");

  pf_data.in_left = static_cast<port*>(pw_filter_add_port(filter, PW_DIRECTION_INPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                                                          sizeof(port), props_in_left, nullptr, 0));

  // right channel input

  auto* props_in_right = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_in_right, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_in_right, PW_KEY_PORT_NAME, "input_FR");
  pw_properties_set(props_in_right, "audio.channel", "FR");

  pf_data.in_right = static_cast<port*>(pw_filter_add_port(filter, PW_DIRECTION_INPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                                                           sizeof(port), props_in_right, nullptr, 0));

  pm->sync_wait_unlock();

  // initializing the stream

  const auto& s_name = "ee_" + log_tag.substr(0, log_tag.size() - 2) + "_" + stream_name;

  auto* props_stream = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_filter, PW_KEY_NODE_NAME, s_name.c_str());
  pw_properties_set(props_filter, PW_KEY_NODE_NICK, stream_name.c_str());
  pw_properties_set(props_filter, PW_KEY_NODE_DESCRIPTION, "easyeffects_output_stream");
  pw_properties_set(props_filter, PW_KEY_NODE_PASSIVE, "true");
  pw_properties_set(props_filter, PW_KEY_NODE_VIRTUAL, "true");
  pw_properties_set(props_filter, PW_KEY_NODE_GROUP, "easyeffects.output.stream");
  pw_properties_set(props_filter, PW_KEY_MEDIA_TYPE, "Audio");

  playback = pw_stream_new(pm->core, "easyeffects output playback", props_stream);
}

OutputStream::~OutputStream() {
  if (filter_connected_to_pw) {
    disconnect_filter_from_pw();
  }

  if (stream_connected_to_pw) {
    disconnect_stream_from_pw();
  }

  util::debug(log_tag + filter_name + " destroyed");
}

void OutputStream::setup() {
  util::debug(log_tag + filter_name + ": new PipeWire blocksize: " + std::to_string(n_samples));
}

auto OutputStream::connect_filter_to_pw() -> bool {
  auto success = false;

  pm->lock();

  if (pw_filter_connect(filter, PW_FILTER_FLAG_RT_PROCESS, nullptr, 0) == 0) {
    filter_connected_to_pw = true;
  }

  pm->sync_wait_unlock();

  if (filter_connected_to_pw) {
    do {
      node_id = pw_filter_get_node_id(filter);

      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } while (node_id == SPA_ID_INVALID);

    pw_filter_add_listener(filter, &listener, &filter_events, &pf_data);

    success = true;

    util::debug(log_tag + filter_name + " filter successfully connected to pipewire graph");
  } else {
    util::error(log_tag + filter_name + " can not connect the filter to pipewire!");
  }

  return success;
}

auto OutputStream::connect_stream_to_pw() -> bool {
  auto success = false;
  std::array<const spa_pod*, 1> params{};
  std::array<uint8_t, 1024U> buffer{};
  struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer.data(), sizeof(buffer));

  auto info = SPA_AUDIO_INFO_RAW_INIT(.format = SPA_AUDIO_FORMAT_DSP_F32, .rate = 48000, .channels = 2);

  pm->lock();

  params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &info);

  if (pw_stream_connect(playback, PW_DIRECTION_OUTPUT, PW_ID_ANY,
                        static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS |
                                                     PW_STREAM_FLAG_RT_PROCESS),
                        params.data(), 1) == 0) {
    stream_connected_to_pw = true;
  }

  pm->sync_wait_unlock();

  if (stream_connected_to_pw) {
    do {
      node_id = pw_stream_get_node_id(playback);

      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } while (node_id == SPA_ID_INVALID);

    pw_stream_add_listener(playback, &playback_listener, &out_stream_events, this);

    success = true;

    util::debug(log_tag + stream_name + " stream successfully connected to pipewire graph");
  } else {
    util::error(log_tag + stream_name + " can not connect stream to pipewire!");
  }

  return success;
}

void OutputStream::disconnect_filter_from_pw() {
  pm->lock();

  pw_filter_disconnect(filter);

  pm->sync_wait_unlock();
}

void OutputStream::disconnect_stream_from_pw() {
  pm->lock();

  pw_stream_disconnect(playback);

  pm->sync_wait_unlock();
}

void OutputStream::process(std::span<float>& left_in, std::span<float>& right_in) {
  // std::copy(left_in.begin(), left_in.end(), left_out.begin());
  // std::copy(right_in.begin(), right_in.end(), right_out.begin());

  if (post_messages) {
    // get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += sample_duration;

    if (notification_dt >= notification_time_window) {
      // notify();

      notification_dt = 0.0F;
    }
  }
}
