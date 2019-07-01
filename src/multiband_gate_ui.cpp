#include "multiband_gate_ui.hpp"

namespace {

gboolean detection_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("RMS")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("Peak")) {
    g_value_set_int(value, 1);
  }

  return true;
}

GVariant* int_to_detection_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("RMS");
  } else {
    return g_variant_new_string("Peak");
  }
}

gboolean mode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("LR4")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("LR8")) {
    g_value_set_int(value, 1);
  }

  return true;
}

GVariant* int_to_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("LR4");
  } else {
    return g_variant_new_string("LR8");
  }
}

}  // namespace

MultibandGateUi::MultibandGateUi(BaseObjectType* cobject,
                                 const Glib::RefPtr<Gtk::Builder>& builder,
                                 const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
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
}

MultibandGateUi::~MultibandGateUi() {
  util::debug(name + " ui destroyed");
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
  gating0->set_value(1 - value);

  gating0_label->set_text(level_to_str(util::linear_to_db(value), 0));
}

void MultibandGateUi::on_new_gating1(double value) {
  gating1->set_value(1 - value);

  gating1_label->set_text(level_to_str(util::linear_to_db(value), 0));
}

void MultibandGateUi::on_new_gating2(double value) {
  gating2->set_value(1 - value);

  gating2_label->set_text(level_to_str(util::linear_to_db(value), 0));
}

void MultibandGateUi::on_new_gating3(double value) {
  gating3->set_value(1 - value);

  gating3_label->set_text(level_to_str(util::linear_to_db(value), 0));
}
