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

#include "limiter.hpp"

// namespace {

// void on_post_messages_changed(GSettings* settings, gchar* key, Limiter* l) {
//   const auto post = g_settings_get_boolean(settings, key);

//   if (post) {
//     if (!l->input_level_connection.connected()) {
//       l->input_level_connection = Glib::signal_timeout().connect(
//           [l]() {
//             float inL = 0.0F;
//             float inR = 0.0F;

//             g_object_get(l->limiter, "meter-inL", &inL, nullptr);
//             g_object_get(l->limiter, "meter-inR", &inR, nullptr);

//             std::array<double, 2> in_peak = {inL, inR};

//             l->input_level.emit(in_peak);

//             return true;
//           },
//           100);
//     }

//     if (!l->output_level_connection.connected()) {
//       l->output_level_connection = Glib::signal_timeout().connect(
//           [l]() {
//             float outL = 0.0F;
//             float outR = 0.0F;

//             g_object_get(l->limiter, "meter-outL", &outL, nullptr);
//             g_object_get(l->limiter, "meter-outR", &outR, nullptr);

//             std::array<double, 2> out_peak = {outL, outR};

//             l->output_level.emit(out_peak);

//             return true;
//           },
//           100);
//     }

//     if (!l->attenuation_connection.connected()) {
//       l->attenuation_connection = Glib::signal_timeout().connect(
//           [l]() {
//             float att = 0.0F;

//             g_object_get(l->limiter, "att", &att, nullptr);

//             l->attenuation.emit(att);

//             return true;
//           },
//           100);
//     }

//   } else {
//     l->input_level_connection.disconnect();
//     l->output_level_connection.disconnect();
//     l->attenuation_connection.disconnect();
//   }
// }

// }  // namespace

Limiter::Limiter(const std::string& tag,
                 const std::string& schema,
                 const std::string& schema_path,
                 PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::limiter, schema, schema_path, pipe_manager),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://calf.sourceforge.net/plugins/Limiter")) {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  settings->signal_changed("bypass").connect([=, this](auto key) { bypass = settings->get_boolean(key); });

  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });

  lv2_wrapper->bind_key_double(settings, "lookahead", "attack");

  lv2_wrapper->bind_key_double(settings, "release", "release");

  lv2_wrapper->bind_key_double(settings, "asc-level", "asc_coeff");

  lv2_wrapper->bind_key_double_db(settings, "limit", "limit");

  lv2_wrapper->bind_key_bool(settings, "auto-level", "auto_level");

  lv2_wrapper->bind_key_bool(settings, "asc", "asc");

  lv2_wrapper->bind_key_int(settings, "oversampling", "oversampling");
}

Limiter::~Limiter() {
  util::debug(log_tag + name + " destroyed");

  pw_thread_loop_lock(pm->thread_loop);

  pw_filter_set_active(filter, false);

  pw_filter_disconnect(filter);

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);
}

void Limiter::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->create_instance(rate);
}

void Limiter::process(std::span<float>& left_in,
                      std::span<float>& right_in,
                      std::span<float>& left_out,
                      std::span<float>& right_out) {
  if (!lv2_wrapper->found_plugin || !lv2_wrapper->has_instance()) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  apply_gain(left_in, right_in, input_gain);

  if (lv2_wrapper->get_n_samples() != left_in.size()) {
    lv2_wrapper->set_n_samples(left_in.size());
  }

  lv2_wrapper->connect_data_ports(left_in, right_in, left_out, right_out);

  lv2_wrapper->run();

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
