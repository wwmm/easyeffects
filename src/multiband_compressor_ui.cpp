/*
 *  Copyright © 2017-2020 Wellington Wallace
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

#include "multiband_compressor_ui.hpp"

namespace {

auto compressor_mode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "Classic") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "Modern") == 0) {
    g_value_set_int(value, 1);
  }

  return 1;
}

auto int_to_compressor_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("Classic");

    case 1:
      return g_variant_new_string("Modern");

    default:
      return g_variant_new_string("Classic");
  }
}

auto envelope_boost_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "None") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "Pink BT") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "Pink MT") == 0) {
    g_value_set_int(value, 2);
  } else if (std::strcmp(v, "Brown BT") == 0) {
    g_value_set_int(value, 3);
  } else if (std::strcmp(v, "Brown MT") == 0) {
    g_value_set_int(value, 4);
  }

  return 1;
}

auto int_to_envelope_boost_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("None");

    case 1:
      return g_variant_new_string("Pink BT");

    case 2:
      return g_variant_new_string("Pink MT");

    case 3:
      return g_variant_new_string("Brown BT");

    case 4:
      return g_variant_new_string("Brown MT");

    default:
      return g_variant_new_string("None");
  }
}

auto compression_mode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "Downward") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "Upward") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "Boosting") == 0) {
    g_value_set_int(value, 2);
  }

  return 1;
}

auto int_to_compression_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("Downward");

    case 1:
      return g_variant_new_string("Upward");

    case 2:
      return g_variant_new_string("Boosting");

    default:
      return g_variant_new_string("Downward");
  }
}

auto sidechain_mode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "Peak") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "RMS") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "Low-Pass") == 0) {
    g_value_set_int(value, 2);
  } else if (std::strcmp(v, "Uniform") == 0) {
    g_value_set_int(value, 3);
  }

  return 1;
}

auto int_to_sidechain_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("Peak");

    case 1:
      return g_variant_new_string("RMS");

    case 2:
      return g_variant_new_string("Low-Pass");

    case 3:
      return g_variant_new_string("Uniform");

    default:
      return g_variant_new_string("RMS");
  }
}

auto sidechain_source_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "Middle") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "Side") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "Left") == 0) {
    g_value_set_int(value, 2);
  } else if (std::strcmp(v, "Right") == 0) {
    g_value_set_int(value, 3);
  }

  return 1;
}

auto int_to_sidechain_source_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("Middle");

    case 1:
      return g_variant_new_string("Side");

    case 2:
      return g_variant_new_string("Left");

    case 3:
      return g_variant_new_string("Right");

    default:
      return g_variant_new_string("Middle");
  }
}

}  // namespace

MultibandCompressorUi::MultibandCompressorUi(BaseObjectType* cobject,
                                             const Glib::RefPtr<Gtk::Builder>& builder,
                                             const std::string& schema,
                                             const std::string& schema_path)
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = plugin_name::multiband_compressor;

  // loading builder widgets

  input_gain = builder->get_widget<Gtk::Scale>("input_gain");
  output_gain = builder->get_widget<Gtk::Scale>("output_gain");

  stack = builder->get_widget<Gtk::Stack>("stack");

  listbox = builder->get_widget<Gtk::ListBox>("listbox");

  listbox->select_row(*listbox->get_row_at_index(0));

  // gsettings bindings

  settings->bind("input-gain", input_gain->get_adjustment().get(), "value");
  settings->bind("output-gain", output_gain->get_adjustment().get(), "value");

  // settings->bind("freq0", freq0->get_adjustment().get(), "value");
  // settings->bind("freq1", freq1->get_adjustment().get(), "value");
  // settings->bind("freq2", freq2->get_adjustment().get(), "value");
  // settings->bind("threshold0", threshold0->get_adjustment().get(), "value");
  // settings->bind("threshold1", threshold1->get_adjustment().get(), "value");
  // settings->bind("threshold2", threshold2->get_adjustment().get(), "value");
  // settings->bind("threshold3", threshold3->get_adjustment().get(), "value");
  // settings->bind("ratio0", ratio0->get_adjustment().get(), "value");
  // settings->bind("ratio1", ratio1->get_adjustment().get(), "value");
  // settings->bind("ratio2", ratio2->get_adjustment().get(), "value");
  // settings->bind("ratio3", ratio3->get_adjustment().get(), "value");
  // settings->bind("attack0", attack0->get_adjustment().get(), "value");
  // settings->bind("attack1", attack1->get_adjustment().get(), "value");
  // settings->bind("attack2", attack2->get_adjustment().get(), "value");
  // settings->bind("attack3", attack3->get_adjustment().get(), "value");
  // settings->bind("release0", release0->get_adjustment().get(), "value");
  // settings->bind("release1", release1->get_adjustment().get(), "value");
  // settings->bind("release2", release2->get_adjustment().get(), "value");
  // settings->bind("release3", release3->get_adjustment().get(), "value");
  // settings->bind("makeup0", makeup0->get_adjustment().get(), "value");
  // settings->bind("makeup1", makeup1->get_adjustment().get(), "value");
  // settings->bind("makeup2", makeup2->get_adjustment().get(), "value");
  // settings->bind("makeup3", makeup3->get_adjustment().get(), "value");
  // settings->bind("knee0", knee0->get_adjustment().get(), "value");
  // settings->bind("knee1", knee1->get_adjustment().get(), "value");
  // settings->bind("knee2", knee2->get_adjustment().get(), "value");
  // settings->bind("knee3", knee3->get_adjustment().get(), "value");

  // settings->bind("bypass0", bypass0, "active");
  // settings->bind("bypass1", bypass1, "active");
  // settings->bind("bypass2", bypass2, "active");
  // settings->bind("bypass3", bypass3, "active");
  // settings->bind("solo0", solo0, "active");
  // settings->bind("solo1", solo1, "active");
  // settings->bind("solo2", solo2, "active");
  // settings->bind("solo3", solo3, "active");

  // g_settings_bind_with_mapping(settings->gobj(), "compressor-mode", mode->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
  //                              compressor_mode_enum_to_int, int_to_compressor_mode_enum, nullptr, nullptr);

  // g_settings_bind_with_mapping(settings->gobj(), "envelope-boost", envelope_boost->gobj(), "active",
  //                              G_SETTINGS_BIND_DEFAULT, envelope_boost_enum_to_int, int_to_envelope_boost_enum,
  //                              nullptr, nullptr);

  // g_settings_bind_with_mapping(settings->gobj(), "detection0", detection0->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
  //                              detection_enum_to_int, int_to_detection_enum, nullptr, nullptr);

  // g_settings_bind_with_mapping(settings->gobj(), "detection1", detection1->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
  //                              detection_enum_to_int, int_to_detection_enum, nullptr, nullptr);

  // g_settings_bind_with_mapping(settings->gobj(), "detection2", detection2->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
  //                              detection_enum_to_int, int_to_detection_enum, nullptr, nullptr);

  // g_settings_bind_with_mapping(settings->gobj(), "detection3", detection3->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
  //                              detection_enum_to_int, int_to_detection_enum, nullptr, nullptr);

  prepare_scale(input_gain, "");
  prepare_scale(output_gain, "");

  // prepare_spinbutton(attack0, "dB");
  // prepare_spinbutton(attack1, "dB");
  // prepare_spinbutton(attack2, "dB");
  // prepare_spinbutton(attack3, "dB");

  // prepare_spinbutton(threshold0, "dB");
  // prepare_spinbutton(threshold1, "dB");
  // prepare_spinbutton(threshold2, "dB");
  // prepare_spinbutton(threshold3, "dB");

  // prepare_spinbutton(knee0, "dB");
  // prepare_spinbutton(knee1, "dB");
  // prepare_spinbutton(knee2, "dB");
  // prepare_spinbutton(knee3, "dB");

  // prepare_spinbutton(makeup0, "dB");
  // prepare_spinbutton(makeup1, "dB");
  // prepare_spinbutton(makeup2, "dB");
  // prepare_spinbutton(makeup3, "dB");

  // prepare_spinbutton(release0, "ms");
  // prepare_spinbutton(release1, "ms");
  // prepare_spinbutton(release2, "ms");
  // prepare_spinbutton(release3, "ms");

  // prepare_spinbutton(freq0, "Hz");
  // prepare_spinbutton(freq1, "Hz");
  // prepare_spinbutton(freq2, "Hz");

  // prepare_spinbutton(ratio0, "");
  // prepare_spinbutton(ratio1, "");
  // prepare_spinbutton(ratio2, "");
  // prepare_spinbutton(ratio3, "");

  prepare_bands();
}

MultibandCompressorUi::~MultibandCompressorUi() {
  util::debug(name + " ui destroyed");
}

auto MultibandCompressorUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> MultibandCompressorUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/multiband_compressor.ui");

  auto* ui = Gtk::Builder::get_widget_derived<MultibandCompressorUi>(
      builder, "top_box", "com.github.wwmm.easyeffects.multibandcompressor", schema_path + "multibandcompressor/");

  auto stack_page = stack->add(*ui, plugin_name::multiband_compressor);

  return ui;
}

void MultibandCompressorUi::prepare_bands() {
  for (int n = 0; n < n_bands; n++) {
    auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/multiband_compressor_band.ui");

    auto* top_box = builder->get_widget<Gtk::Box>("top_box");

    stack->add(*top_box, "band" + std::to_string(n));
  }
}

void MultibandCompressorUi::reset() {
  bypass->set_active(false);

  settings->reset("input-gain");

  settings->reset("output-gain");

  // settings->reset("freq0");

  // settings->reset("freq1");

  // settings->reset("freq2");

  // settings->reset("mode");

  // // sub band

  // settings->reset("threshold0");

  // settings->reset("ratio0");

  // settings->reset("attack0");

  // settings->reset("release0");

  // settings->reset("makeup0");

  // settings->reset("knee0");

  // settings->reset("detection0");

  // settings->reset("bypass0");

  // settings->reset("solo0");

  // // low band

  // settings->reset("threshold1");

  // settings->reset("ratio1");

  // settings->reset("attack1");

  // settings->reset("release1");

  // settings->reset("makeup1");

  // settings->reset("knee1");

  // settings->reset("detection1");

  // settings->reset("bypass1");

  // settings->reset("solo1");

  // // mid band

  // settings->reset("threshold2");

  // settings->reset("ratio2");

  // settings->reset("attack2");

  // settings->reset("release2");

  // settings->reset("makeup2");

  // settings->reset("knee2");

  // settings->reset("detection2");

  // settings->reset("bypass2");

  // settings->reset("solo2");

  // // high band

  // settings->reset("threshold3");

  // settings->reset("ratio3");

  // settings->reset("attack3");

  // settings->reset("release3");

  // settings->reset("makeup3");

  // settings->reset("knee3");

  // settings->reset("detection3");

  // settings->reset("bypass3");

  // settings->reset("solo3");
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
