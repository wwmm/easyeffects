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

#include "loudness_ui.hpp"

namespace {

auto fft_size_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "256") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "512") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "1024") == 0) {
    g_value_set_int(value, 2);
  } else if (std::strcmp(v, "2048") == 0) {
    g_value_set_int(value, 3);
  } else if (std::strcmp(v, "4096") == 0) {
    g_value_set_int(value, 4);
  } else if (std::strcmp(v, "8192") == 0) {
    g_value_set_int(value, 5);
  } else if (std::strcmp(v, "16384") == 0) {
    g_value_set_int(value, 6);
  }

  return 1;
}

auto int_to_fft_size_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("256");

    case 1:
      return g_variant_new_string("512");

    case 2:
      return g_variant_new_string("1024");

    case 3:
      return g_variant_new_string("2048");

    case 4:
      return g_variant_new_string("4096");

    case 5:
      return g_variant_new_string("8192");

    case 6:
      return g_variant_new_string("16384");

    default:
      return g_variant_new_string("4096");
  }
}

auto standard_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "Flat") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "ISO226-2003") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "Fletcher-Munson") == 0) {
    g_value_set_int(value, 2);
  } else if (std::strcmp(v, "Robinson-Dadson") == 0) {
    g_value_set_int(value, 3);
  }

  return 1;
}

auto int_to_standard_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("Flat");

    case 1:
      return g_variant_new_string("ISO226-2003");

    case 2:
      return g_variant_new_string("Fletcher-Munson");

    case 3:
      return g_variant_new_string("Robinson-Dadson");

    default:
      return g_variant_new_string("ISO226-2003");
  }
}

}  // namespace

LoudnessUi::LoudnessUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       const std::string& schema,
                       const std::string& schema_path)
    : Gtk::Grid(cobject), PluginUiBase(builder, schema, schema_path) {
  name = "loudness";

  // loading glade widgets

  builder->get_widget("plugin_reset", reset_button);
  builder->get_widget("fft_size", fft_size);
  builder->get_widget("standard", standard);

  get_object(builder, "input", input);
  get_object(builder, "volume", volume);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("input", input.get(), "value", flag);
  settings->bind("volume", volume.get(), "value", flag);

  g_settings_bind_with_mapping(settings->gobj(), "fft", fft_size->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               fft_size_enum_to_int, int_to_fft_size_enum, nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "std", standard->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               standard_enum_to_int, int_to_standard_enum, nullptr, nullptr);

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });
}

LoudnessUi::~LoudnessUi() {
  util::debug(name + " ui destroyed");
}

void LoudnessUi::reset() {
  try {
    std::string section = (preset_type == PresetType::output) ? "output" : "input";

    update_default_string_key(settings, "fft", section + ".loudness.fft");

    update_default_string_key(settings, "std", section + ".loudness.std");

    update_default_key<double>(settings, "input", section + ".loudness.input");

    update_default_key<double>(settings, "volume", section + ".loudness.volume");

    util::debug(name + " plugin: successfully reset");
  } catch (std::exception& e) {
    util::debug(name + " plugin: an error occurred during reset process");
  }
}
