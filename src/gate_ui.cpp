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

#include "gate_ui.hpp"
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

auto stereo_link_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "Average") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "Maximum") == 0) {
    g_value_set_int(value, 1);
  }

  return 1;
}

auto int_to_stereo_link_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("Average");

    case 1:
      return g_variant_new_string("Maximum");

    default:
      return g_variant_new_string("Average");
  }
}

}  // namespace

GateUi::GateUi(BaseObjectType* cobject,
               const Glib::RefPtr<Gtk::Builder>& builder,
               const std::string& schema,
               const std::string& schema_path)
    : Gtk::Grid(cobject), PluginUiBase(builder, schema, schema_path) {
  name = "gate";

  // loading glade widgets

  builder->get_widget("detection", detection);
  builder->get_widget("stereo_link", stereo_link);
  builder->get_widget("gating", gating);
  builder->get_widget("gating_label", gating_label);
  builder->get_widget("plugin_reset", reset_button);

  get_object(builder, "attack", attack);
  get_object(builder, "knee", knee);
  get_object(builder, "input", input);
  get_object(builder, "makeup", makeup);
  get_object(builder, "range", range);
  get_object(builder, "ratio", ratio);
  get_object(builder, "release", release);
  get_object(builder, "threshold", threshold);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("attack", attack.get(), "value", flag);
  settings->bind("knee", knee.get(), "value", flag);
  settings->bind("input", input.get(), "value", flag);
  settings->bind("makeup", makeup.get(), "value", flag);
  settings->bind("range", range.get(), "value", flag);
  settings->bind("ratio", ratio.get(), "value", flag);
  settings->bind("release", release.get(), "value", flag);
  settings->bind("threshold", threshold.get(), "value", flag);

  g_settings_bind_with_mapping(settings->gobj(), "detection", detection->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               detection_enum_to_int, int_to_detection_enum, nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "stereo-link", stereo_link->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               stereo_link_enum_to_int, int_to_stereo_link_enum, nullptr, nullptr);

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });
}

GateUi::~GateUi() {
  util::debug(name + " ui destroyed");
}

void GateUi::reset() {
  settings->reset("detection");

  settings->reset("stereo-link");

  settings->reset("range");

  settings->reset("attack");

  settings->reset("release");

  settings->reset("threshold");

  settings->reset("ratio");

  settings->reset("knee");

  settings->reset("input");

  settings->reset("makeup");
}

void GateUi::on_new_gating(double value) {
  gating->set_value(1.0 - value);

  gating_label->set_text(level_to_str(util::linear_to_db(value), 0));
}
