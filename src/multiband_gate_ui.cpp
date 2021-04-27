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

#include "multiband_gate_ui.hpp"

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

  if (std::strcmp(v, "LR4") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "LR8") == 0) {
    g_value_set_int(value, 1);
  }

  return 1;
}

auto int_to_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("LR4");

    default:
      return g_variant_new_string("LR8");
  }
}

}  // namespace

MultibandGateUi::MultibandGateUi(BaseObjectType* cobject,
                                 const Glib::RefPtr<Gtk::Builder>& builder,
                                 const std::string& schema,
                                 const std::string& schema_path)
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = plugin_name::multiband_gate;

  // loading builder widgets

  input_gain = builder->get_widget<Gtk::Scale>("input_gain");
  output_gain = builder->get_widget<Gtk::Scale>("output_gain");

  freq0 = builder->get_widget<Gtk::SpinButton>("freq0");
  freq1 = builder->get_widget<Gtk::SpinButton>("freq1");
  freq2 = builder->get_widget<Gtk::SpinButton>("freq2");
  threshold0 = builder->get_widget<Gtk::SpinButton>("threshold0");
  threshold1 = builder->get_widget<Gtk::SpinButton>("threshold1");
  threshold2 = builder->get_widget<Gtk::SpinButton>("threshold2");
  threshold3 = builder->get_widget<Gtk::SpinButton>("threshold3");
  ratio0 = builder->get_widget<Gtk::SpinButton>("ratio0");
  ratio1 = builder->get_widget<Gtk::SpinButton>("ratio1");
  ratio2 = builder->get_widget<Gtk::SpinButton>("ratio2");
  ratio3 = builder->get_widget<Gtk::SpinButton>("ratio3");
  range0 = builder->get_widget<Gtk::SpinButton>("range0");
  range1 = builder->get_widget<Gtk::SpinButton>("range1");
  range2 = builder->get_widget<Gtk::SpinButton>("range2");
  range3 = builder->get_widget<Gtk::SpinButton>("range3");
  attack0 = builder->get_widget<Gtk::SpinButton>("attack0");
  attack1 = builder->get_widget<Gtk::SpinButton>("attack1");
  attack2 = builder->get_widget<Gtk::SpinButton>("attack2");
  attack3 = builder->get_widget<Gtk::SpinButton>("attack3");
  release0 = builder->get_widget<Gtk::SpinButton>("release0");
  release1 = builder->get_widget<Gtk::SpinButton>("release1");
  release2 = builder->get_widget<Gtk::SpinButton>("release2");
  release3 = builder->get_widget<Gtk::SpinButton>("release3");
  makeup0 = builder->get_widget<Gtk::SpinButton>("makeup0");
  makeup1 = builder->get_widget<Gtk::SpinButton>("makeup1");
  makeup2 = builder->get_widget<Gtk::SpinButton>("makeup2");
  makeup3 = builder->get_widget<Gtk::SpinButton>("makeup3");
  knee0 = builder->get_widget<Gtk::SpinButton>("knee0");
  knee1 = builder->get_widget<Gtk::SpinButton>("knee1");
  knee2 = builder->get_widget<Gtk::SpinButton>("knee2");
  knee3 = builder->get_widget<Gtk::SpinButton>("knee3");

  bypass0 = builder->get_widget<Gtk::ToggleButton>("bypass0");
  bypass1 = builder->get_widget<Gtk::ToggleButton>("bypass1");
  bypass2 = builder->get_widget<Gtk::ToggleButton>("bypass2");
  bypass3 = builder->get_widget<Gtk::ToggleButton>("bypass3");
  solo0 = builder->get_widget<Gtk::ToggleButton>("solo0");
  solo1 = builder->get_widget<Gtk::ToggleButton>("solo1");
  solo2 = builder->get_widget<Gtk::ToggleButton>("solo2");
  solo3 = builder->get_widget<Gtk::ToggleButton>("solo3");
  solo0 = builder->get_widget<Gtk::ToggleButton>("solo0");

  mode = builder->get_widget<Gtk::ComboBoxText>("mode");
  detection0 = builder->get_widget<Gtk::ComboBoxText>("detection0");
  detection1 = builder->get_widget<Gtk::ComboBoxText>("detection1");
  detection2 = builder->get_widget<Gtk::ComboBoxText>("detection2");
  detection3 = builder->get_widget<Gtk::ComboBoxText>("detection3");

  output0 = builder->get_widget<Gtk::LevelBar>("output0");
  output1 = builder->get_widget<Gtk::LevelBar>("output1");
  output2 = builder->get_widget<Gtk::LevelBar>("output2");
  output3 = builder->get_widget<Gtk::LevelBar>("output3");
  gating0 = builder->get_widget<Gtk::LevelBar>("gating0");
  gating1 = builder->get_widget<Gtk::LevelBar>("gating1");
  gating2 = builder->get_widget<Gtk::LevelBar>("gating2");
  gating3 = builder->get_widget<Gtk::LevelBar>("gating3");

  output0_label = builder->get_widget<Gtk::Label>("output0_label");
  output1_label = builder->get_widget<Gtk::Label>("output1_label");
  output2_label = builder->get_widget<Gtk::Label>("output2_label");
  output3_label = builder->get_widget<Gtk::Label>("output3_label");
  gating0_label = builder->get_widget<Gtk::Label>("gating0_label");
  gating1_label = builder->get_widget<Gtk::Label>("gating1_label");
  gating2_label = builder->get_widget<Gtk::Label>("gating2_label");
  gating3_label = builder->get_widget<Gtk::Label>("gating3_label");

  // gsettings bindings

  settings->bind("input-gain", input_gain->get_adjustment().get(), "value");
  settings->bind("output-gain", output_gain->get_adjustment().get(), "value");
  settings->bind("freq0", freq0->get_adjustment().get(), "value");
  settings->bind("freq1", freq1->get_adjustment().get(), "value");
  settings->bind("freq2", freq2->get_adjustment().get(), "value");
  settings->bind("threshold0", threshold0->get_adjustment().get(), "value");
  settings->bind("threshold1", threshold1->get_adjustment().get(), "value");
  settings->bind("threshold2", threshold2->get_adjustment().get(), "value");
  settings->bind("threshold3", threshold3->get_adjustment().get(), "value");
  settings->bind("ratio0", ratio0->get_adjustment().get(), "value");
  settings->bind("ratio1", ratio1->get_adjustment().get(), "value");
  settings->bind("ratio2", ratio2->get_adjustment().get(), "value");
  settings->bind("ratio3", ratio3->get_adjustment().get(), "value");
  settings->bind("range0", range0->get_adjustment().get(), "value");
  settings->bind("range1", range1->get_adjustment().get(), "value");
  settings->bind("range2", range2->get_adjustment().get(), "value");
  settings->bind("range3", range3->get_adjustment().get(), "value");
  settings->bind("attack0", attack0->get_adjustment().get(), "value");
  settings->bind("attack1", attack1->get_adjustment().get(), "value");
  settings->bind("attack2", attack2->get_adjustment().get(), "value");
  settings->bind("attack3", attack3->get_adjustment().get(), "value");
  settings->bind("release0", release0->get_adjustment().get(), "value");
  settings->bind("release1", release1->get_adjustment().get(), "value");
  settings->bind("release2", release2->get_adjustment().get(), "value");
  settings->bind("release3", release3->get_adjustment().get(), "value");
  settings->bind("makeup0", makeup0->get_adjustment().get(), "value");
  settings->bind("makeup1", makeup1->get_adjustment().get(), "value");
  settings->bind("makeup2", makeup2->get_adjustment().get(), "value");
  settings->bind("makeup3", makeup3->get_adjustment().get(), "value");
  settings->bind("knee0", knee0->get_adjustment().get(), "value");
  settings->bind("knee1", knee1->get_adjustment().get(), "value");
  settings->bind("knee2", knee2->get_adjustment().get(), "value");
  settings->bind("knee3", knee3->get_adjustment().get(), "value");

  settings->bind("bypass0", bypass0, "active");
  settings->bind("bypass1", bypass1, "active");
  settings->bind("bypass2", bypass2, "active");
  settings->bind("bypass3", bypass3, "active");
  settings->bind("solo0", solo0, "active");
  settings->bind("solo1", solo1, "active");
  settings->bind("solo2", solo2, "active");
  settings->bind("solo3", solo3, "active");

  g_settings_bind_with_mapping(settings->gobj(), "mode", mode->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               mode_enum_to_int, int_to_mode_enum, nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "detection0", detection0->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               detection_enum_to_int, int_to_detection_enum, nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "detection1", detection1->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               detection_enum_to_int, int_to_detection_enum, nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "detection2", detection2->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               detection_enum_to_int, int_to_detection_enum, nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "detection3", detection3->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               detection_enum_to_int, int_to_detection_enum, nullptr, nullptr);

  prepare_spinbutton(range0, "dB");
  prepare_spinbutton(range1, "dB");
  prepare_spinbutton(range2, "dB");
  prepare_spinbutton(range3, "dB");

  prepare_spinbutton(attack0, "dB");
  prepare_spinbutton(attack1, "dB");
  prepare_spinbutton(attack2, "dB");
  prepare_spinbutton(attack3, "dB");

  prepare_spinbutton(threshold0, "dB");
  prepare_spinbutton(threshold1, "dB");
  prepare_spinbutton(threshold2, "dB");
  prepare_spinbutton(threshold3, "dB");

  prepare_spinbutton(knee0, "dB");
  prepare_spinbutton(knee1, "dB");
  prepare_spinbutton(knee2, "dB");
  prepare_spinbutton(knee3, "dB");

  prepare_spinbutton(makeup0, "dB");
  prepare_spinbutton(makeup1, "dB");
  prepare_spinbutton(makeup2, "dB");
  prepare_spinbutton(makeup3, "dB");

  prepare_spinbutton(release0, "ms");
  prepare_spinbutton(release1, "ms");
  prepare_spinbutton(release2, "ms");
  prepare_spinbutton(release3, "ms");

  prepare_spinbutton(freq0, "Hz");
  prepare_spinbutton(freq1, "Hz");
  prepare_spinbutton(freq2, "Hz");
}

MultibandGateUi::~MultibandGateUi() {
  util::debug(name + " ui destroyed");
}

auto MultibandGateUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> MultibandGateUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/multiband_gate.ui");

  auto* ui = Gtk::Builder::get_widget_derived<MultibandGateUi>(
      builder, "top_box", "com.github.wwmm.pulseeffects.multibandgate", schema_path + "multibandgate/");

  auto stack_page = stack->add(*ui, plugin_name::multiband_gate);

  return ui;
}

void MultibandGateUi::reset() {
  bypass->set_active(false);

  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("freq0");

  settings->reset("freq1");

  settings->reset("freq2");

  settings->reset("mode");

  // sub band

  settings->reset("range0");

  settings->reset("threshold0");

  settings->reset("ratio0");

  settings->reset("attack0");

  settings->reset("release0");

  settings->reset("makeup0");

  settings->reset("knee0");

  settings->reset("detection0");

  settings->reset("bypass0");

  settings->reset("solo0");

  // low band

  settings->reset("range1");

  settings->reset("threshold1");

  settings->reset("ratio1");

  settings->reset("attack1");

  settings->reset("release1");

  settings->reset("makeup1");

  settings->reset("knee1");

  settings->reset("detection1");

  settings->reset("bypass1");

  settings->reset("solo1");

  // mid band

  settings->reset("range2");

  settings->reset("threshold2");

  settings->reset("ratio2");

  settings->reset("attack2");

  settings->reset("release2");

  settings->reset("makeup2");

  settings->reset("knee2");

  settings->reset("detection2");

  settings->reset("bypass2");

  settings->reset("solo2");

  // high band

  settings->reset("range3");

  settings->reset("threshold3");

  settings->reset("ratio3");

  settings->reset("attack3");

  settings->reset("release3");

  settings->reset("makeup3");

  settings->reset("knee3");

  settings->reset("detection3");

  settings->reset("bypass3");

  settings->reset("solo3");
}

void MultibandGateUi::on_new_output0(double value) {
  output0->set_value(value);

  output0_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}

void MultibandGateUi::on_new_output1(double value) {
  output1->set_value(value);

  output1_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}

void MultibandGateUi::on_new_output2(double value) {
  output2->set_value(value);

  output2_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}

void MultibandGateUi::on_new_output3(double value) {
  output3->set_value(value);

  output3_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}

void MultibandGateUi::on_new_gating0(double value) {
  gating0->set_value(1.0 - value);

  gating0_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}

void MultibandGateUi::on_new_gating1(double value) {
  gating1->set_value(1.0 - value);

  gating1_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}

void MultibandGateUi::on_new_gating2(double value) {
  gating2->set_value(1.0 - value);

  gating2_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}

void MultibandGateUi::on_new_gating3(double value) {
  gating3->set_value(1.0 - value);

  gating3_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}
