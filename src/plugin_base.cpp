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

#include "plugin_base.hpp"

namespace {

void on_process(void* userdata, spa_io_position* position) {
  auto* d = static_cast<PluginBase::data*>(userdata);

  const auto& n_samples = position->clock.duration;
  const auto& rate = position->clock.rate.denom;

  if (n_samples == 0 || rate == 0) {
    return;
  }

  if (rate != d->pb->rate || n_samples != d->pb->n_samples) {
    d->pb->rate = rate;
    d->pb->n_samples = n_samples;
    d->pb->sample_duration = static_cast<float>(n_samples) / static_cast<float>(rate);

    d->pb->setup();
  }

  // util::warning("processing: " + std::to_string(n_samples));

  auto* in_left = static_cast<float*>(pw_filter_get_dsp_buffer(d->in_left, n_samples));
  auto* in_right = static_cast<float*>(pw_filter_get_dsp_buffer(d->in_right, n_samples));

  auto* out_left = static_cast<float*>(pw_filter_get_dsp_buffer(d->out_left, n_samples));
  auto* out_right = static_cast<float*>(pw_filter_get_dsp_buffer(d->out_right, n_samples));

  std::span left_in{in_left, in_left + n_samples};
  std::span left_out{out_left, out_left + n_samples};
  std::span right_in{in_right, in_right + n_samples};
  std::span right_out{out_right, out_right + n_samples};

  if (!d->pb->enable_probe) {
    d->pb->process(left_in, right_in, left_out, right_out);
  } else {
    auto* probe_left = static_cast<float*>(pw_filter_get_dsp_buffer(d->probe_left, n_samples));
    auto* probe_right = static_cast<float*>(pw_filter_get_dsp_buffer(d->probe_right, n_samples));

    std::span l{probe_left, probe_left + n_samples};
    std::span r{probe_right, probe_right + n_samples};

    d->pb->process(left_in, right_in, left_out, right_out, l, r);
  }
}

const struct pw_filter_events filter_events = {.process = on_process};

}  // namespace

PluginBase::PluginBase(std::string tag,
                       std::string plugin_name,
                       const std::string& schema,
                       const std::string& schema_path,
                       PipeManager* pipe_manager,
                       const bool& enable_probe)
    : log_tag(std::move(tag)),
      name(std::move(plugin_name)),
      enable_probe(enable_probe),
      settings(Gio::Settings::create(schema.c_str(), schema_path.c_str())),
      pm(pipe_manager) {
  pf_data.pb = this;

  const auto& filter_name = "pe_" + log_tag.substr(0, log_tag.size() - 2U) + "_" + name;

  pm->lock();

  auto* props_filter = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_filter, PW_KEY_NODE_NAME, filter_name.c_str());
  pw_properties_set(props_filter, PW_KEY_NODE_NICK, name.c_str());
  pw_properties_set(props_filter, PW_KEY_NODE_DESCRIPTION, "easyeffects_filter");
  pw_properties_set(props_filter, PW_KEY_MEDIA_TYPE, "Audio");
  pw_properties_set(props_filter, PW_KEY_MEDIA_CATEGORY, "Filter");
  pw_properties_set(props_filter, PW_KEY_MEDIA_ROLE, "DSP");

  filter = pw_filter_new(pm->core, filter_name.c_str(), props_filter);

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

  // left channel output

  auto* props_out_left = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_out_left, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_out_left, PW_KEY_PORT_NAME, "output_FL");
  pw_properties_set(props_out_left, "audio.channel", "FL");

  pf_data.out_left = static_cast<port*>(pw_filter_add_port(filter, PW_DIRECTION_OUTPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                                                           sizeof(port), props_out_left, nullptr, 0));

  // right channel output

  auto* props_out_right = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_out_right, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_out_right, PW_KEY_PORT_NAME, "output_FR");
  pw_properties_set(props_out_right, "audio.channel", "FR");

  pf_data.out_right = static_cast<port*>(pw_filter_add_port(
      filter, PW_DIRECTION_OUTPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS, sizeof(port), props_out_right, nullptr, 0));

  if (enable_probe) {
    // probe left input

    auto* props_left = pw_properties_new(nullptr, nullptr);

    pw_properties_set(props_left, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
    pw_properties_set(props_left, PW_KEY_PORT_NAME, "probe_FL");
    pw_properties_set(props_left, "audio.channel", "PROBE_FL");

    pf_data.probe_left = static_cast<port*>(pw_filter_add_port(
        filter, PW_DIRECTION_INPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS, sizeof(port), props_left, nullptr, 0));

    // probe right input

    auto* props_right = pw_properties_new(nullptr, nullptr);

    pw_properties_set(props_right, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
    pw_properties_set(props_right, PW_KEY_PORT_NAME, "probe_FR");
    pw_properties_set(props_right, "audio.channel", "PROBE_FR");

    pf_data.probe_right = static_cast<port*>(pw_filter_add_port(
        filter, PW_DIRECTION_INPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS, sizeof(port), props_right, nullptr, 0));
  }

  pm->sync_wait_unlock();
}

PluginBase::~PluginBase() {
  if (listener.link.next != nullptr || listener.link.prev != nullptr) {
    spa_hook_remove(&listener);
  }
}

auto PluginBase::connect_to_pw() -> bool {
  auto success = false;

  pm->lock();

  if (pw_filter_connect(filter, PW_FILTER_FLAG_RT_PROCESS, nullptr, 0) == 0) {
    connected_to_pw = true;
  }

  pm->sync_wait_unlock();

  if (connected_to_pw) {
    do {
      node_id = pw_filter_get_node_id(filter);

      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } while (node_id == SPA_ID_INVALID);

    initialize_listener();

    success = true;

    util::debug(log_tag + name + " successfully connected to PipeWire graph");
  } else {
    util::error(log_tag + name + " cannot connect the filter to PipeWire!");
  }

  return success;
}

void PluginBase::initialize_listener() {
  pw_filter_add_listener(filter, &listener, &filter_events, &pf_data);
}

auto PluginBase::get_node_id() const -> uint {
  return node_id;
}

void PluginBase::set_active(const bool& state) const {
  pw_filter_set_active(filter, state);
}

void PluginBase::disconnect_from_pw() {
  pm->lock();

  set_active(false);

  pw_filter_disconnect(filter);

  pm->sync_wait_unlock();
}

void PluginBase::setup() {}

void PluginBase::process(std::span<float>& left_in,
                         std::span<float>& right_in,
                         std::span<float>& left_out,
                         std::span<float>& right_out) {}

void PluginBase::process(std::span<float>& left_in,
                         std::span<float>& right_in,
                         std::span<float>& left_out,
                         std::span<float>& right_out,
                         std::span<float>& probe_left,
                         std::span<float>& probe_right) {}

void PluginBase::get_peaks(const std::span<float>& left_in,
                           const std::span<float>& right_in,
                           std::span<float>& left_out,
                           std::span<float>& right_out) {
  if (!post_messages) {
    return;
  }

  // input level

  float peak_l = std::ranges::max(left_in);
  float peak_r = std::ranges::max(right_in);

  input_peak_left = (peak_l > input_peak_left) ? peak_l : input_peak_left;
  input_peak_right = (peak_r > input_peak_right) ? peak_r : input_peak_right;

  // output level

  peak_l = std::ranges::max(left_out);
  peak_r = std::ranges::max(right_out);

  output_peak_left = (peak_l > output_peak_left) ? peak_l : output_peak_left;
  output_peak_right = (peak_r > output_peak_right) ? peak_r : output_peak_right;
}

void PluginBase::setup_input_output_gain() {
  input_gain = static_cast<float>(util::db_to_linear(settings->get_double("input-gain")));
  output_gain = static_cast<float>(util::db_to_linear(settings->get_double("output-gain")));

  settings->signal_changed("input-gain").connect([=, this](const auto& key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](const auto& key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });
}

void PluginBase::apply_gain(std::span<float>& left, std::span<float>& right, const float& gain) {
  if (left.empty() || right.empty()) {
    return;
  }

  std::ranges::for_each(left, [&](auto& v) { v *= gain; });
  std::ranges::for_each(right, [&](auto& v) { v *= gain; });
}

void PluginBase::notify() {
  const auto& input_peak_db_l = util::linear_to_db(input_peak_left);
  const auto& input_peak_db_r = util::linear_to_db(input_peak_right);

  const auto& output_peak_db_l = util::linear_to_db(output_peak_left);
  const auto& output_peak_db_r = util::linear_to_db(output_peak_right);

  Glib::signal_idle().connect_once([=, this] { input_level.emit(input_peak_db_l, input_peak_db_r); });
  Glib::signal_idle().connect_once([=, this] { output_level.emit(output_peak_db_l, output_peak_db_r); });

  input_peak_left = util::minimum_linear_level;
  input_peak_right = util::minimum_linear_level;
  output_peak_left = util::minimum_linear_level;
  output_peak_right = util::minimum_linear_level;
}
