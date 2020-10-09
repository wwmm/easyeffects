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

#include "deesser_ui.hpp"
#include <cstring>

namespace {

auto detection_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "RMS") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "Peak") == 0) {
    g_value_set_int(value, 1);
  }

  return 1;
}

auto int_to_detection_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("RMS");

    case 1:
      return g_variant_new_string("Peak");

    default:
      return g_variant_new_string("RMS");
  }
}

auto mode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "Wide") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "Split") == 0) {
    g_value_set_int(value, 1);
  }

  return 1;
}

auto int_to_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("Wide");

    case 1:
      return g_variant_new_string("Split");

    default:
      return g_variant_new_string("Wide");
  }
}

}  // namespace

DeesserUi::DeesserUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& builder,
                     const std::string& schema,
                     const std::string& schema_path)
    : Gtk::Grid(cobject), PluginUiBase(builder, schema, schema_path) {
  name = "deesser";

  // loading glade widgets

  builder->get_widget("detection", detection);
  builder->get_widget("mode", mode);
  builder->get_widget("compression", compression);
  builder->get_widget("compression_label", compression_label);
  builder->get_widget("detected", detected);
  builder->get_widget("detected_label", detected_label);
  builder->get_widget("sc_listen", sc_listen);
  builder->get_widget("plugin_reset", reset_button);

  get_object(builder, "makeup", makeup);
  get_object(builder, "ratio", ratio);
  get_object(builder, "threshold", threshold);
  get_object(builder, "f1_freq", f1_freq);
  get_object(builder, "f2_freq", f2_freq);
  get_object(builder, "f1_level", f1_level);
  get_object(builder, "f2_level", f2_level);
  get_object(builder, "f2_q", f2_q);
  get_object(builder, "laxity", laxity);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("sc-listen", sc_listen, "active", flag);
  settings->bind("makeup", makeup.get(), "value", flag);
  settings->bind("ratio", ratio.get(), "value", flag);
  settings->bind("threshold", threshold.get(), "value", flag);
  settings->bind("f1-freq", f1_freq.get(), "value", flag);
  settings->bind("f2-freq", f2_freq.get(), "value", flag);
  settings->bind("f1-level", f1_level.get(), "value", flag);
  settings->bind("f2-level", f2_level.get(), "value", flag);
  settings->bind("f2-q", f2_q.get(), "value", flag);
  settings->bind("laxity", laxity.get(), "value", flag);

  g_settings_bind_with_mapping(settings->gobj(), "detection", detection->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               detection_enum_to_int, int_to_detection_enum, nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "mode", mode->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               mode_enum_to_int, int_to_mode_enum, nullptr, nullptr);

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });
}

DeesserUi::~DeesserUi() {
  util::debug(name + " ui destroyed");
}

void DeesserUi::reset() {
  try {
    std::string section = (preset_type == PresetType::output) ? "output" : "input";

    update_default_string_key(settings, "detection", section + ".deesser.detection");

    update_default_string_key(settings, "mode", section + ".deesser.mode");

    update_default_key<double>(settings, "threshold", section + ".deesser.threshold");

    update_default_key<double>(settings, "ratio", section + ".deesser.ratio");

    update_default_key<int>(settings, "laxity", section + ".deesser.laxity");

    update_default_key<double>(settings, "makeup", section + ".deesser.makeup");

    update_default_key<double>(settings, "f1-freq", section + ".deesser.f1-freq");

    update_default_key<double>(settings, "f2-freq", section + ".deesser.f2-freq");

    update_default_key<double>(settings, "f1-level", section + ".deesser.f1-level");

    update_default_key<double>(settings, "f2-level", section + ".deesser.f2-level");

    update_default_key<double>(settings, "f2-q", section + ".deesser.f2-q");

    update_default_key<bool>(settings, "sc-listen", section + ".deesser.sc-listen");

    util::debug(name + " plugin: successfully reset");
  } catch (std::exception& e) {
    util::debug(name + " plugin: an error occurred during reset process");
  }
}

void DeesserUi::on_new_compression(double value) {
  compression->set_value(1.0 - value);

  compression_label->set_text(level_to_str(util::linear_to_db(value), 0));
}

void DeesserUi::on_new_detected(double value) {
  detected->set_value(value);

  detected_label->set_text(level_to_str(util::linear_to_db(value), 0));
}
