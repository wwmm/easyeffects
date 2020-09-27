#include "multiband_gate_ui.hpp"
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

MultibandGateUi::MultibandGateUi(BaseObjectType* cobject,
                                 const Glib::RefPtr<Gtk::Builder>& builder,
                                 const std::string& schema,
                                 const std::string& schema_path)
    : Gtk::Grid(cobject), PluginUiBase(builder, schema, schema_path) {
  name = "multiband_gate";

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
  builder->get_widget("gating0", gating0);
  builder->get_widget("gating0_label", gating0_label);
  builder->get_widget("gating1", gating1);
  builder->get_widget("gating1_label", gating1_label);
  builder->get_widget("gating2", gating2);
  builder->get_widget("gating2_label", gating2_label);
  builder->get_widget("gating3", gating3);
  builder->get_widget("gating3_label", gating3_label);
  builder->get_widget("plugin_reset", reset_button);

  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);
  get_object(builder, "freq0", freq0);
  get_object(builder, "freq1", freq1);
  get_object(builder, "freq2", freq2);
  get_object(builder, "range0", range0);
  get_object(builder, "range1", range1);
  get_object(builder, "range2", range2);
  get_object(builder, "range3", range3);
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
  settings->bind("range0", range0.get(), "value", flag);
  settings->bind("range1", range1.get(), "value", flag);
  settings->bind("range2", range2.get(), "value", flag);
  settings->bind("range3", range3.get(), "value", flag);
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

MultibandGateUi::~MultibandGateUi() {
  util::debug(name + " ui destroyed");
}

void MultibandGateUi::reset() {
  try {
    std::string section = (preset_type == PresetType::output) ? "output" : "input";

    update_default_key<double>(settings, "input-gain", section + ".multiband_gate.input-gain");

    update_default_key<double>(settings, "output-gain", section + ".multiband_gate.output-gain");

    update_default_key<double>(settings, "freq0", section + ".multiband_gate.freq0");

    update_default_key<double>(settings, "freq1", section + ".multiband_gate.freq1");

    update_default_key<double>(settings, "freq2", section + ".multiband_gate.freq2");

    update_default_string_key(settings, "mode", section + ".multiband_gate.mode");

    // sub band

    update_default_key<double>(settings, "range0", section + ".multiband_gate.subband.reduction");

    update_default_key<double>(settings, "threshold0", section + ".multiband_gate.subband.threshold");

    update_default_key<double>(settings, "ratio0", section + ".multiband_gate.subband.ratio");

    update_default_key<double>(settings, "attack0", section + ".multiband_gate.subband.attack");

    update_default_key<double>(settings, "release0", section + ".multiband_gate.subband.release");

    update_default_key<double>(settings, "makeup0", section + ".multiband_gate.subband.makeup");

    update_default_key<double>(settings, "knee0", section + ".multiband_gate.subband.knee");

    update_default_string_key(settings, "detection0", section + ".multiband_gate.subband.detection");

    update_default_key<bool>(settings, "bypass0", section + ".multiband_gate.subband.bypass");

    update_default_key<bool>(settings, "solo0", section + ".multiband_gate.subband.solo");

    // low band

    update_default_key<double>(settings, "range1", section + ".multiband_gate.lowband.reduction");

    update_default_key<double>(settings, "threshold1", section + ".multiband_gate.lowband.threshold");

    update_default_key<double>(settings, "ratio1", section + ".multiband_gate.lowband.ratio");

    update_default_key<double>(settings, "attack1", section + ".multiband_gate.lowband.attack");

    update_default_key<double>(settings, "release1", section + ".multiband_gate.lowband.release");

    update_default_key<double>(settings, "makeup1", section + ".multiband_gate.lowband.makeup");

    update_default_key<double>(settings, "knee1", section + ".multiband_gate.lowband.knee");

    update_default_string_key(settings, "detection1", section + ".multiband_gate.lowband.detection");

    update_default_key<bool>(settings, "bypass1", section + ".multiband_gate.lowband.bypass");

    update_default_key<bool>(settings, "solo1", section + ".multiband_gate.lowband.solo");

    // mid band

    update_default_key<double>(settings, "range2", section + ".multiband_gate.midband.reduction");

    update_default_key<double>(settings, "threshold2", section + ".multiband_gate.midband.threshold");

    update_default_key<double>(settings, "ratio2", section + ".multiband_gate.midband.ratio");

    update_default_key<double>(settings, "attack2", section + ".multiband_gate.midband.attack");

    update_default_key<double>(settings, "release2", section + ".multiband_gate.midband.release");

    update_default_key<double>(settings, "makeup2", section + ".multiband_gate.midband.makeup");

    update_default_key<double>(settings, "knee2", section + ".multiband_gate.midband.knee");

    update_default_string_key(settings, "detection2", section + ".multiband_gate.midband.detection");

    update_default_key<bool>(settings, "bypass2", section + ".multiband_gate.midband.bypass");

    update_default_key<bool>(settings, "solo2", section + ".multiband_gate.midband.solo");

    // high band

    update_default_key<double>(settings, "range3", section + ".multiband_gate.highband.reduction");

    update_default_key<double>(settings, "threshold3", section + ".multiband_gate.highband.threshold");

    update_default_key<double>(settings, "ratio3", section + ".multiband_gate.highband.ratio");

    update_default_key<double>(settings, "attack3", section + ".multiband_gate.highband.attack");

    update_default_key<double>(settings, "release3", section + ".multiband_gate.highband.release");

    update_default_key<double>(settings, "makeup3", section + ".multiband_gate.highband.makeup");

    update_default_key<double>(settings, "knee3", section + ".multiband_gate.highband.knee");

    update_default_string_key(settings, "detection3", section + ".multiband_gate.highband.detection");

    update_default_key<bool>(settings, "bypass3", section + ".multiband_gate.highband.bypass");

    update_default_key<bool>(settings, "solo3", section + ".multiband_gate.highband.solo");

    util::debug(name + " plugin: successfully reset");
  } catch (std::exception& e) {
    util::debug(name + " plugin: an error occurred during reset process");
  }
}

void MultibandGateUi::on_new_output0(double value) {
  output0->set_value(value);

  output0_label->set_text(level_to_str(util::linear_to_db(value), 0));
}

void MultibandGateUi::on_new_output1(double value) {
  output1->set_value(value);

  output1_label->set_text(level_to_str(util::linear_to_db(value), 0));
}

void MultibandGateUi::on_new_output2(double value) {
  output2->set_value(value);

  output2_label->set_text(level_to_str(util::linear_to_db(value), 0));
}

void MultibandGateUi::on_new_output3(double value) {
  output3->set_value(value);

  output3_label->set_text(level_to_str(util::linear_to_db(value), 0));
}

void MultibandGateUi::on_new_gating0(double value) {
  gating0->set_value(1.0 - value);

  gating0_label->set_text(level_to_str(util::linear_to_db(value), 0));
}

void MultibandGateUi::on_new_gating1(double value) {
  gating1->set_value(1.0 - value);

  gating1_label->set_text(level_to_str(util::linear_to_db(value), 0));
}

void MultibandGateUi::on_new_gating2(double value) {
  gating2->set_value(1.0 - value);

  gating2_label->set_text(level_to_str(util::linear_to_db(value), 0));
}

void MultibandGateUi::on_new_gating3(double value) {
  gating3->set_value(1.0 - value);

  gating3_label->set_text(level_to_str(util::linear_to_db(value), 0));
}
