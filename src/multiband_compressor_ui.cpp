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

#include "multiband_compressor_ui.hpp"
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

    case 1:
      return g_variant_new_string("LR8");

    default:
      return g_variant_new_string("LR8");
  }
}

}  // namespace

MultibandCompressorUi::MultibandCompressorUi(BaseObjectType* cobject,
                                             const Glib::RefPtr<Gtk::Builder>& builder,
                                             const std::string& schema,
                                             const std::string& schema_path)
    : Gtk::Grid(cobject), PluginUiBase(builder, schema, schema_path) {
  name = "multiband_compressor";

  // loading glade widgets

  builder->get_widget("mode", mode);
  builder->get_widget("bypass0", bypass0);
  builder->get_widget("bypass1", bypass1);
  builder->get_widget("bypass2", bypass2);
  builder->get_widget("bypass3", bypass3);
  builder->get_widget("solo0", solo0);
  builder->get_widget("solo1", solo1);
  builder->get_widget("solo2", solo2);
  builder->get_widget("solo3", solo3);
  builder->get_widget("detection0", detection0);
  builder->get_widget("detection1", detection1);
  builder->get_widget("detection2", detection2);
  builder->get_widget("detection3", detection3);
  builder->get_widget("output0", output0);
  builder->get_widget("output0_label", output0_label);
  builder->get_widget("output1", output1);
  builder->get_widget("output1_label", output1_label);
  builder->get_widget("output2", output2);
  builder->get_widget("output2_label", output2_label);
  builder->get_widget("output3", output3);
  builder->get_widget("output3_label", output3_label);
  builder->get_widget("compression0", compression0);
  builder->get_widget("compression0_label", compression0_label);
  builder->get_widget("compression1", compression1);
  builder->get_widget("compression1_label", compression1_label);
  builder->get_widget("compression2", compression2);
  builder->get_widget("compression2_label", compression2_label);
  builder->get_widget("compression3", compression3);
  builder->get_widget("compression3_label", compression3_label);
  builder->get_widget("plugin_reset", reset_button);

  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);
  get_object(builder, "freq0", freq0);
  get_object(builder, "freq1", freq1);
  get_object(builder, "freq2", freq2);
  get_object(builder, "threshold0", threshold0);
  get_object(builder, "threshold1", threshold1);
  get_object(builder, "threshold2", threshold2);
  get_object(builder, "threshold3", threshold3);
  get_object(builder, "ratio0", ratio0);
  get_object(builder, "ratio1", ratio1);
  get_object(builder, "ratio2", ratio2);
  get_object(builder, "ratio3", ratio3);
  get_object(builder, "attack0", attack0);
  get_object(builder, "attack1", attack1);
  get_object(builder, "attack2", attack2);
  get_object(builder, "attack3", attack3);
  get_object(builder, "release0", release0);
  get_object(builder, "release1", release1);
  get_object(builder, "release2", release2);
  get_object(builder, "release3", release3);
  get_object(builder, "makeup0", makeup0);
  get_object(builder, "makeup1", makeup1);
  get_object(builder, "makeup2", makeup2);
  get_object(builder, "makeup3", makeup3);
  get_object(builder, "knee0", knee0);
  get_object(builder, "knee1", knee1);
  get_object(builder, "knee2", knee2);
  get_object(builder, "knee3", knee3);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);

  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);
  settings->bind("freq0", freq0.get(), "value", flag);
  settings->bind("freq1", freq1.get(), "value", flag);
  settings->bind("freq2", freq2.get(), "value", flag);
  settings->bind("threshold0", threshold0.get(), "value", flag);
  settings->bind("threshold1", threshold1.get(), "value", flag);
  settings->bind("threshold2", threshold2.get(), "value", flag);
  settings->bind("threshold3", threshold3.get(), "value", flag);
  settings->bind("ratio0", ratio0.get(), "value", flag);
  settings->bind("ratio1", ratio1.get(), "value", flag);
  settings->bind("ratio2", ratio2.get(), "value", flag);
  settings->bind("ratio3", ratio3.get(), "value", flag);
  settings->bind("attack0", attack0.get(), "value", flag);
  settings->bind("attack1", attack1.get(), "value", flag);
  settings->bind("attack2", attack2.get(), "value", flag);
  settings->bind("attack3", attack3.get(), "value", flag);
  settings->bind("release0", release0.get(), "value", flag);
  settings->bind("release1", release1.get(), "value", flag);
  settings->bind("release2", release2.get(), "value", flag);
  settings->bind("release3", release3.get(), "value", flag);
  settings->bind("makeup0", makeup0.get(), "value", flag);
  settings->bind("makeup1", makeup1.get(), "value", flag);
  settings->bind("makeup2", makeup2.get(), "value", flag);
  settings->bind("makeup3", makeup3.get(), "value", flag);
  settings->bind("knee0", knee0.get(), "value", flag);
  settings->bind("knee1", knee1.get(), "value", flag);
  settings->bind("knee2", knee2.get(), "value", flag);
  settings->bind("knee3", knee3.get(), "value", flag);

  settings->bind("bypass0", bypass0, "active", flag);
  settings->bind("bypass1", bypass1, "active", flag);
  settings->bind("bypass2", bypass2, "active", flag);
  settings->bind("bypass3", bypass3, "active", flag);
  settings->bind("solo0", solo0, "active", flag);
  settings->bind("solo1", solo1, "active", flag);
  settings->bind("solo2", solo2, "active", flag);
  settings->bind("solo3", solo3, "active", flag);

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

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });
}

MultibandCompressorUi::~MultibandCompressorUi() {
  util::debug(name + " ui destroyed");
}

void MultibandCompressorUi::reset() {
  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("freq0");

  settings->reset("freq1");

  settings->reset("freq2");

  settings->reset("mode");

  // sub band

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

void MultibandCompressorUi::on_new_output0(double value) {
  output0->set_value(value);

  output0_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}

void MultibandCompressorUi::on_new_output1(double value) {
  output1->set_value(value);

  output1_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}

void MultibandCompressorUi::on_new_output2(double value) {
  output2->set_value(value);

  output2_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}

void MultibandCompressorUi::on_new_output3(double value) {
  output3->set_value(value);

  output3_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}

void MultibandCompressorUi::on_new_compression0(double value) {
  compression0->set_value(1.0 - value);

  compression0_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}

void MultibandCompressorUi::on_new_compression1(double value) {
  compression1->set_value(1.0 - value);

  compression1_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}

void MultibandCompressorUi::on_new_compression2(double value) {
  compression2->set_value(1.0 - value);

  compression2_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}

void MultibandCompressorUi::on_new_compression3(double value) {
  compression3->set_value(1.0 - value);

  compression3_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}
