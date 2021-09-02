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

#include "gate_ui.hpp"

namespace {

auto detection_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (g_strcmp0(v, "RMS") == 0) {
    g_value_set_int(value, 0);
  } else if (g_strcmp0(v, "Peak") == 0) {
    g_value_set_int(value, 1);
  }

  return 1;
}

auto int_to_detection_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  switch (g_value_get_int(value)) {
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

  if (g_strcmp0(v, "Average") == 0) {
    g_value_set_int(value, 0);
  } else if (g_strcmp0(v, "Maximum") == 0) {
    g_value_set_int(value, 1);
  }

  return 1;
}

auto int_to_stereo_link_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  switch (g_value_get_int(value)) {
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
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = plugin_name::gate;

  // loading builder widgets

  input_gain = builder->get_widget<Gtk::Scale>("input_gain");
  output_gain = builder->get_widget<Gtk::Scale>("output_gain");
  attack = builder->get_widget<Gtk::SpinButton>("attack");
  knee = builder->get_widget<Gtk::SpinButton>("knee");
  makeup = builder->get_widget<Gtk::SpinButton>("makeup");
  range = builder->get_widget<Gtk::SpinButton>("range");
  ratio = builder->get_widget<Gtk::SpinButton>("ratio");
  release = builder->get_widget<Gtk::SpinButton>("release");
  threshold = builder->get_widget<Gtk::SpinButton>("threshold");

  detection = builder->get_widget<Gtk::ComboBoxText>("detection");
  stereo_link = builder->get_widget<Gtk::ComboBoxText>("stereo_link");

  gating = builder->get_widget<Gtk::LevelBar>("gating");
  gating_label = builder->get_widget<Gtk::Label>("gating_label");

  // gsettings bindings

  settings->bind("input-gain", input_gain->get_adjustment().get(), "value");
  settings->bind("output-gain", output_gain->get_adjustment().get(), "value");
  settings->bind("attack", attack->get_adjustment().get(), "value");
  settings->bind("knee", knee->get_adjustment().get(), "value");
  settings->bind("makeup", makeup->get_adjustment().get(), "value");
  settings->bind("range", range->get_adjustment().get(), "value");
  settings->bind("ratio", ratio->get_adjustment().get(), "value");
  settings->bind("release", release->get_adjustment().get(), "value");
  settings->bind("threshold", threshold->get_adjustment().get(), "value");

  g_settings_bind_with_mapping(settings->gobj(), "detection", detection->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               detection_enum_to_int, int_to_detection_enum, nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "stereo-link", stereo_link->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               stereo_link_enum_to_int, int_to_stereo_link_enum, nullptr, nullptr);

  prepare_scale(input_gain, "");
  prepare_scale(output_gain, "");

  prepare_spinbutton(attack, "ms");
  prepare_spinbutton(release, "ms");

  prepare_spinbutton(range, "dB");
  prepare_spinbutton(threshold, "dB");
  prepare_spinbutton(knee, "dB");
  prepare_spinbutton(makeup, "dB");

  prepare_spinbutton(ratio, "");
}

GateUi::~GateUi() {
  util::debug(name + " ui destroyed");
}

auto GateUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> GateUi* {
  const auto& builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/gate.ui");

  auto* const ui = Gtk::Builder::get_widget_derived<GateUi>(builder, "top_box", "com.github.wwmm.easyeffects.gate",
                                                      schema_path + "gate/");

  stack->add(*ui, plugin_name::gate);

  return ui;
}

void GateUi::reset() {
  bypass->set_active(false);

  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("detection");

  settings->reset("stereo-link");

  settings->reset("range");

  settings->reset("attack");

  settings->reset("release");

  settings->reset("threshold");

  settings->reset("ratio");

  settings->reset("knee");

  settings->reset("makeup");
}

void GateUi::on_new_gating(const double& value) {
  gating->set_value(1.0 - value);

  gating_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}
