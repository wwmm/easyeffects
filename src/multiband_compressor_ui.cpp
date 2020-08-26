#include "multiband_compressor_ui.hpp"
#include <cstring>

namespace {

auto detection_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  auto v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "RMS") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "Peak") == 0) {
    g_value_set_int(value, 1);
  }

  return 1;
}

auto int_to_detection_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("RMS");
  }

  return g_variant_new_string("Peak");
}

auto mode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  auto v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "LR4") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "LR8") == 0) {
    g_value_set_int(value, 1);
  }

  return 1;
}

auto int_to_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("LR4");
  }

  return g_variant_new_string("LR8");
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
  try {
    std::string section = (preset_type == PresetType::output) ? "output" : "input";

    update_default_key<double>(settings, "input-gain", section + ".multiband_compressor.input-gain");

    update_default_key<double>(settings, "output-gain", section + ".multiband_compressor.output-gain");

    update_default_key<double>(settings, "freq0", section + ".multiband_compressor.freq0");

    update_default_key<double>(settings, "freq1", section + ".multiband_compressor.freq1");

    update_default_key<double>(settings, "freq2", section + ".multiband_compressor.freq2");

    update_default_string_key(settings, "mode", section + ".multiband_compressor.mode");

    // sub band

    update_default_key<double>(settings, "threshold0", section + ".multiband_compressor.subband.threshold");

    update_default_key<double>(settings, "ratio0", section + ".multiband_compressor.subband.ratio");

    update_default_key<double>(settings, "attack0", section + ".multiband_compressor.subband.attack");

    update_default_key<double>(settings, "release0", section + ".multiband_compressor.subband.release");

    update_default_key<double>(settings, "makeup0", section + ".multiband_compressor.subband.makeup");

    update_default_key<double>(settings, "knee0", section + ".multiband_compressor.subband.knee");

    update_default_string_key(settings, "detection0", section + ".multiband_compressor.subband.detection");

    update_default_key<bool>(settings, "bypass0", section + ".multiband_compressor.subband.bypass");

    update_default_key<bool>(settings, "solo0", section + ".multiband_compressor.subband.solo");

    // low band

    update_default_key<double>(settings, "threshold1", section + ".multiband_compressor.lowband.threshold");

    update_default_key<double>(settings, "ratio1", section + ".multiband_compressor.lowband.ratio");

    update_default_key<double>(settings, "attack1", section + ".multiband_compressor.lowband.attack");

    update_default_key<double>(settings, "release1", section + ".multiband_compressor.lowband.release");

    update_default_key<double>(settings, "makeup1", section + ".multiband_compressor.lowband.makeup");

    update_default_key<double>(settings, "knee1", section + ".multiband_compressor.lowband.knee");

    update_default_string_key(settings, "detection1", section + ".multiband_compressor.lowband.detection");

    update_default_key<bool>(settings, "bypass1", section + ".multiband_compressor.lowband.bypass");

    update_default_key<bool>(settings, "solo1", section + ".multiband_compressor.lowband.solo");

    // mid band

    update_default_key<double>(settings, "threshold2", section + ".multiband_compressor.midband.threshold");

    update_default_key<double>(settings, "ratio2", section + ".multiband_compressor.midband.ratio");

    update_default_key<double>(settings, "attack2", section + ".multiband_compressor.midband.attack");

    update_default_key<double>(settings, "release2", section + ".multiband_compressor.midband.release");

    update_default_key<double>(settings, "makeup2", section + ".multiband_compressor.midband.makeup");

    update_default_key<double>(settings, "knee2", section + ".multiband_compressor.midband.knee");

    update_default_string_key(settings, "detection2", section + ".multiband_compressor.midband.detection");

    update_default_key<bool>(settings, "bypass2", section + ".multiband_compressor.midband.bypass");

    update_default_key<bool>(settings, "solo2", section + ".multiband_compressor.midband.solo");

    // high band

    update_default_key<double>(settings, "threshold3", section + ".multiband_compressor.highband.threshold");

    update_default_key<double>(settings, "ratio3", section + ".multiband_compressor.highband.ratio");

    update_default_key<double>(settings, "attack3", section + ".multiband_compressor.highband.attack");

    update_default_key<double>(settings, "release3", section + ".multiband_compressor.highband.release");

    update_default_key<double>(settings, "makeup3", section + ".multiband_compressor.highband.makeup");

    update_default_key<double>(settings, "knee3", section + ".multiband_compressor.highband.knee");

    update_default_string_key(settings, "detection3", section + ".multiband_compressor.highband.detection");

    update_default_key<bool>(settings, "bypass3", section + ".multiband_compressor.highband.bypass");

    update_default_key<bool>(settings, "solo3", section + ".multiband_compressor.highband.solo");

    util::debug(name + " plugin: successfully reset");
  } catch (std::exception& e) {
    util::debug(name + " plugin: an error occurred during reset process");
  }
}

void MultibandCompressorUi::on_new_output0(double value) {
  output0->set_value(value);

  output0_label->set_text(level_to_str(util::linear_to_db(static_cast<float>(value)), 0));
}

void MultibandCompressorUi::on_new_output1(double value) {
  output1->set_value(value);

  output1_label->set_text(level_to_str(util::linear_to_db(static_cast<float>(value)), 0));
}

void MultibandCompressorUi::on_new_output2(double value) {
  output2->set_value(value);

  output2_label->set_text(level_to_str(util::linear_to_db(static_cast<float>(value)), 0));
}

void MultibandCompressorUi::on_new_output3(double value) {
  output3->set_value(value);

  output3_label->set_text(level_to_str(util::linear_to_db(static_cast<float>(value)), 0));
}

void MultibandCompressorUi::on_new_compression0(double value) {
  compression0->set_value(1.0 - value);

  compression0_label->set_text(level_to_str(util::linear_to_db(static_cast<float>(value)), 0));
}

void MultibandCompressorUi::on_new_compression1(double value) {
  compression1->set_value(1.0 - value);

  compression1_label->set_text(level_to_str(util::linear_to_db(static_cast<float>(value)), 0));
}

void MultibandCompressorUi::on_new_compression2(double value) {
  compression2->set_value(1.0 - value);

  compression2_label->set_text(level_to_str(util::linear_to_db(static_cast<float>(value)), 0));
}

void MultibandCompressorUi::on_new_compression3(double value) {
  compression3->set_value(1.0 - value);

  compression3_label->set_text(level_to_str(util::linear_to_db(static_cast<float>(value)), 0));
}
