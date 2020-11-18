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

#include "webrtc_ui.hpp"
#include <cstring>

namespace {

auto echo_suppression_level_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "low") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "moderate") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "high") == 0) {
    g_value_set_int(value, 2);
  }

  return 1;
}

auto int_to_echo_suppression_level(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("low");

    case 1:
      return g_variant_new_string("moderate");

    case 2:
      return g_variant_new_string("high");

    default:
      return g_variant_new_string("moderate");
  }
}

auto noise_suppression_level_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "low") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "moderate") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "high") == 0) {
    g_value_set_int(value, 2);
  } else if (std::strcmp(v, "very-high") == 0) {
    g_value_set_int(value, 3);
  }

  return 1;
}

auto int_to_noise_suppression_level(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("low");

    case 1:
      return g_variant_new_string("moderate");

    case 2:
      return g_variant_new_string("high");

    case 3:
      return g_variant_new_string("very-high");

    default:
      return g_variant_new_string("moderate");
  }
}

auto gain_control_mode_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "adaptive-digital") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "fixed-digital") == 0) {
    g_value_set_int(value, 1);
  }

  return 1;
}

auto int_to_gain_control_mode(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("adaptive-digital");

    case 1:
      return g_variant_new_string("fixed-digital");

    default:
      return g_variant_new_string("adaptive-digital");
  }
}

auto voice_detection_likelihood_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "very-low") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "low") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "moderate") == 0) {
    g_value_set_int(value, 2);
  } else if (std::strcmp(v, "high") == 0) {
    g_value_set_int(value, 3);
  }

  return 1;
}

auto int_to_voice_detection_likelihood(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("very-low");

    case 1:
      return g_variant_new_string("low");

    case 2:
      return g_variant_new_string("moderate");

    case 3:
      return g_variant_new_string("high");

    default:
      return g_variant_new_string("low");
  }
}

}  // namespace

WebrtcUi::WebrtcUi(BaseObjectType* cobject,
                   const Glib::RefPtr<Gtk::Builder>& builder,
                   const std::string& schema,
                   const std::string& schema_path)
    : Gtk::Grid(cobject), PluginUiBase(builder, schema, schema_path) {
  name = "webrtc";

  // loading glade widgets

  builder->get_widget("echo_cancel", echo_cancel);
  builder->get_widget("extended_filter", extended_filter);
  builder->get_widget("high_pass_filter", high_pass_filter);
  builder->get_widget("delay_agnostic", delay_agnostic);
  builder->get_widget("noise_suppression", noise_suppression);
  builder->get_widget("gain_control", gain_control);
  builder->get_widget("limiter", limiter);
  builder->get_widget("voice_detection", voice_detection);
  builder->get_widget("echo_suppression_level", echo_suppression_level);
  builder->get_widget("noise_suppression_level", noise_suppression_level);
  builder->get_widget("gain_control_mode", gain_control_mode);
  builder->get_widget("voice_detection_likelihood", voice_detection_likelihood);
  builder->get_widget("plugin_reset", reset_button);

  get_object(builder, "compression_gain_db", compression_gain_db);
  get_object(builder, "target_level_dbfs", target_level_dbfs);
  get_object(builder, "voice_detection_frame_size", voice_detection_frame_size);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);

  settings->bind("compression-gain-db", compression_gain_db.get(), "value", flag);
  settings->bind("target-level-dbfs", target_level_dbfs.get(), "value", flag);
  settings->bind("voice-detection-frame-size-ms", voice_detection_frame_size.get(), "value", flag);
  settings->bind("echo-cancel", echo_cancel, "active", flag);
  settings->bind("extended-filter", extended_filter, "active", flag);
  settings->bind("high-pass-filter", high_pass_filter, "active", flag);
  settings->bind("delay-agnostic", delay_agnostic, "active", flag);
  settings->bind("noise-suppression", noise_suppression, "active", flag);
  settings->bind("gain-control", gain_control, "active", flag);
  settings->bind("limiter", limiter, "active", flag);
  settings->bind("voice-detection", voice_detection, "active", flag);

  g_settings_bind_with_mapping(settings->gobj(), "echo-suppression-level", echo_suppression_level->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, echo_suppression_level_to_int, int_to_echo_suppression_level,
                               nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "noise-suppression-level", noise_suppression_level->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, noise_suppression_level_to_int, int_to_noise_suppression_level,
                               nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "gain-control-mode", gain_control_mode->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, gain_control_mode_to_int, int_to_gain_control_mode, nullptr,
                               nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "voice-detection-likelihood", voice_detection_likelihood->gobj(),
                               "active", G_SETTINGS_BIND_DEFAULT, voice_detection_likelihood_to_int,
                               int_to_voice_detection_likelihood, nullptr, nullptr);

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });
}

WebrtcUi::~WebrtcUi() {
  util::debug(name + " ui destroyed");
}

void WebrtcUi::reset() {
  settings->reset("high-pass-filter");

  settings->reset("echo-cancel");

  settings->reset("echo-suppression-level");

  settings->reset("noise-suppression");

  settings->reset("noise-suppression-level");

  settings->reset("gain-control");

  settings->reset("extended-filter");

  settings->reset("delay-agnostic");

  settings->reset("target-level-dbfs");

  settings->reset("compression-gain-db");

  settings->reset("limiter");

  settings->reset("gain-control-mode");

  settings->reset("voice-detection");

  settings->reset("voice-detection-frame-size-ms");

  settings->reset("voice-detection-likelihood");
}
