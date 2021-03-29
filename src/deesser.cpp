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

#include "deesser.hpp"

// namespace {

// void on_post_messages_changed(GSettings* settings, gchar* key, Deesser* l) {
//   const auto post = g_settings_get_boolean(settings, key);

//   if (post) {
//     if (!l->compression_connection.connected()) {
//       l->compression_connection = Glib::signal_timeout().connect(
//           [l]() {
//             float compression = 0.0F;

//             g_object_get(l->deesser, "compression", &compression, nullptr);

//             l->compression.emit(compression);

//             return true;
//           },
//           100);
//     }

//     if (!l->detected_connection.connected()) {
//       l->detected_connection = Glib::signal_timeout().connect(
//           [l]() {
//             float detected = 0.0F;

//             g_object_get(l->deesser, "detected", &detected, nullptr);

//             l->detected.emit(detected);

//             return true;
//           },
//           100);
//     }
//   } else {
//     l->compression_connection.disconnect();
//     l->detected_connection.disconnect();
//   }
// }

// }  // namespace

Deesser::Deesser(const std::string& tag,
                 const std::string& schema,
                 const std::string& schema_path,
                 PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::deesser, schema, schema_path, pipe_manager),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://calf.sourceforge.net/plugins/Deesser")) {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });
}

Deesser::~Deesser() {
  util::debug(log_tag + name + " destroyed");

  pw_thread_loop_lock(pm->thread_loop);

  pw_filter_set_active(filter, false);

  pw_filter_disconnect(filter);

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);
}

void Deesser::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);
  lv2_wrapper->create_instance(rate);
}

void Deesser::process(std::span<float>& left_in,
                      std::span<float>& right_in,
                      std::span<float>& left_out,
                      std::span<float>& right_out) {
  if (!lv2_wrapper->found_plugin || !lv2_wrapper->has_instance() || bypass) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  apply_gain(left_in, right_in, input_gain);

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

// g_settings_bind(settings, "detection", deesser, "detection", G_SETTINGS_BIND_DEFAULT);
// g_settings_bind(settings, "mode", deesser, "mode", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind_with_mapping(settings, "ratio", deesser, "ratio", G_SETTINGS_BIND_GET, util::double_to_float,
// nullptr,
//                              nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "threshold", deesser, "threshold", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind(settings, "laxity", deesser, "laxity", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind_with_mapping(settings, "makeup", deesser, "makeup", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "f1-freq", deesser, "f1-freq", G_SETTINGS_BIND_GET, util::double_to_float,
//                              nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "f2-freq", deesser, "f2-freq", G_SETTINGS_BIND_GET, util::double_to_float,
//                              nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "f1-level", deesser, "f1-level", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "f2-level", deesser, "f2-level", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "f2-q", deesser, "f2-q", G_SETTINGS_BIND_GET, util::double_to_float,
// nullptr,
//                              nullptr, nullptr);

// g_settings_bind(settings, "sc-listen", deesser, "sc-listen", G_SETTINGS_BIND_DEFAULT);
