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

  compressor_mode = builder->get_widget<Gtk::ComboBoxText>("compressor_mode");

  envelope_boost = builder->get_widget<Gtk::ComboBoxText>("envelope_boost");

  stack = builder->get_widget<Gtk::Stack>("stack");

  listbox = builder->get_widget<Gtk::ListBox>("listbox");

  listbox->select_row(*listbox->get_row_at_index(0));

  listbox->signal_selected_rows_changed().connect([=, this]() {
    // some core dumps happened here
    // checking pointer and int row integrity might fix them

    auto* selected_row = listbox->get_selected_row();

    if (selected_row != nullptr) {
      int row = selected_row->get_index();

      if (row > -1) {
        stack->set_visible_child("band" + std::to_string(row));
      }
    }
  });

  // gsettings bindings

  settings->bind("input-gain", input_gain->get_adjustment().get(), "value");
  settings->bind("output-gain", output_gain->get_adjustment().get(), "value");

  g_settings_bind_with_mapping(settings->gobj(), "compressor-mode", compressor_mode->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, compressor_mode_enum_to_int, int_to_compressor_mode_enum,
                               nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "envelope-boost", envelope_boost->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, envelope_boost_enum_to_int, int_to_envelope_boost_enum,
                               nullptr, nullptr);

  prepare_scale(input_gain, "");
  prepare_scale(output_gain, "");

  // band checkbuttons

  for (uint n = 1; n < n_bands; n++) {
    auto nstr = std::to_string(n);

    auto* enable_band = builder->get_widget<Gtk::CheckButton>("enable_band" + nstr);

    settings->bind("enable-band" + nstr, enable_band, "active");
  }

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
  for (uint n = 0; n < n_bands; n++) {
    auto nstr = std::to_string(n);

    auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/multiband_compressor_band.ui");


    if (n > 0U) {
      auto* split_frequency = builder->get_widget<Gtk::SpinButton>("split_frequency");

      settings->bind("split-frequency" + nstr, split_frequency->get_adjustment().get(), "value");

      prepare_spinbutton(split_frequency, "Hz");
    } else {
      // removing split frequency from band 0

      auto* sf_box = builder->get_widget<Gtk::Box>("split_frequency_box");

      for (auto* child = sf_box->get_last_child(); child != nullptr; child = sf_box->get_last_child()) {
        sf_box->remove(*child);
      }

      auto sf_label = Gtk::Label("0 Hz", Gtk::Align::CENTER);

      sf_box->append(sf_label);
    }

    // loading builder widgets

    auto* band_bypass = builder->get_widget<Gtk::ToggleButton>("bypass");

    auto* mute = builder->get_widget<Gtk::ToggleButton>("mute");

    auto* solo = builder->get_widget<Gtk::ToggleButton>("solo");

    auto* lowcut_filter = builder->get_widget<Gtk::CheckButton>("lowcut_filter");

    auto* highcut_filter = builder->get_widget<Gtk::CheckButton>("highcut_filter");

    auto* lowcut_filter_frequency = builder->get_widget<Gtk::SpinButton>("lowcut_filter_frequency");

    auto* highcut_filter_frequency = builder->get_widget<Gtk::SpinButton>("highcut_filter_frequency");

    auto* attack_time = builder->get_widget<Gtk::SpinButton>("attack_time");

    auto* attack_threshold = builder->get_widget<Gtk::SpinButton>("attack_threshold");

    auto* release_time = builder->get_widget<Gtk::SpinButton>("release_time");

    auto* release_threshold = builder->get_widget<Gtk::SpinButton>("release_threshold");

    auto* ratio = builder->get_widget<Gtk::SpinButton>("ratio");

    auto* knee = builder->get_widget<Gtk::SpinButton>("knee");

    auto* makeup = builder->get_widget<Gtk::SpinButton>("makeup");

    auto* sidechain_preamp = builder->get_widget<Gtk::SpinButton>("sidechain_preamp");

    auto* sidechain_reactivity = builder->get_widget<Gtk::SpinButton>("sidechain_reactivity");

    auto* sidechain_lookahead = builder->get_widget<Gtk::SpinButton>("sidechain_lookahead");

    auto* boost_amount = builder->get_widget<Gtk::SpinButton>("boost_amount");

    auto* boost_threshold = builder->get_widget<Gtk::SpinButton>("boost_threshold");

    auto* compression_mode = builder->get_widget<Gtk::ComboBoxText>("compression_mode");

    auto* sidechain_mode = builder->get_widget<Gtk::ComboBoxText>("sidechain_mode");

    auto* sidechain_source = builder->get_widget<Gtk::ComboBoxText>("sidechain_source");

    // gsettings bindings

    settings->bind("compressor-enable" + nstr, band_bypass, "active", Gio::Settings::BindFlags::INVERT_BOOLEAN);

    settings->bind("mute" + nstr, mute, "active");

    settings->bind("solo" + nstr, solo, "active");

    settings->bind("sidechain-custom-lowcut-filter" + nstr, lowcut_filter, "active");

    settings->bind("sidechain-custom-highcut-filter" + nstr, highcut_filter, "active");

    settings->bind("sidechain-lowcut-frequency" + nstr, lowcut_filter_frequency->get_adjustment().get(), "value");

    settings->bind("sidechain-highcut-frequency" + nstr, highcut_filter_frequency->get_adjustment().get(), "value");

    settings->bind("attack-time" + nstr, attack_time->get_adjustment().get(), "value");

    settings->bind("attack-threshold" + nstr, attack_threshold->get_adjustment().get(), "value");

    settings->bind("release-time" + nstr, release_time->get_adjustment().get(), "value");

    settings->bind("release-threshold" + nstr, release_threshold->get_adjustment().get(), "value");

    settings->bind("ratio" + nstr, ratio->get_adjustment().get(), "value");

    settings->bind("knee" + nstr, knee->get_adjustment().get(), "value");

    settings->bind("makeup" + nstr, makeup->get_adjustment().get(), "value");

    settings->bind("sidechain-preamp" + nstr, sidechain_preamp->get_adjustment().get(), "value");

    settings->bind("sidechain-reactivity" + nstr, sidechain_reactivity->get_adjustment().get(), "value");

    settings->bind("sidechain-lookahead" + nstr, sidechain_lookahead->get_adjustment().get(), "value");

    settings->bind("boost-amount" + nstr, boost_amount->get_adjustment().get(), "value");

    settings->bind("boost-threshold" + nstr, boost_threshold->get_adjustment().get(), "value");

    g_settings_bind_with_mapping(settings->gobj(), std::string("compression-mode" + nstr).c_str(),
                                 compression_mode->gobj(), "active", G_SETTINGS_BIND_DEFAULT, compression_mode_enum_to_int, int_to_compression_mode_enum, nullptr, nullptr);

    g_settings_bind_with_mapping(settings->gobj(), std::string("sidechain-mode" + nstr).c_str(),
                                 sidechain_mode->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                                 sidechain_mode_enum_to_int, int_to_sidechain_mode_enum, nullptr, nullptr);

    g_settings_bind_with_mapping(settings->gobj(), std::string("sidechain-source" + nstr).c_str(),
                                 sidechain_source->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                                 sidechain_source_enum_to_int, int_to_sidechain_source_enum, nullptr, nullptr);

    // prepare widgets

    prepare_spinbutton(lowcut_filter_frequency, "Hz");

    prepare_spinbutton(highcut_filter_frequency, "Hz");

    prepare_spinbutton(attack_time, "ms");

    prepare_spinbutton(attack_threshold, "db");

    prepare_spinbutton(release_time, "ms");

    prepare_spinbutton(release_threshold, "db");

    prepare_spinbutton(ratio, "");

    prepare_spinbutton(knee, "db");

    prepare_spinbutton(makeup, "db");

    prepare_spinbutton(sidechain_preamp, "db");

    prepare_spinbutton(sidechain_reactivity, "ms");

    prepare_spinbutton(sidechain_lookahead, "ms");

    prepare_spinbutton(boost_amount, "db");

    prepare_spinbutton(boost_threshold, "db");

    // add to stack

    auto* top_box = builder->get_widget<Gtk::Box>("top_box");

    stack->add(*top_box, "band" + nstr);
  }
}

void MultibandCompressorUi::reset() {
  bypass->set_active(false);

  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("compressor-mode");

  settings->reset("envelope-boost");

  for (uint n = 0; n < n_bands; n++) {
    auto nstr = std::to_string(n);

    if (n > 0) {
      settings->reset("enable-band" + nstr);

      settings->reset("split-frequency" + nstr);
    }

    settings->reset("compressor-enable" + nstr);

    settings->reset("solo" + nstr);

    settings->reset("mute" + nstr);

    settings->reset("attack-threshold" + nstr);

    settings->reset("attack-time" + nstr);

    settings->reset("release-threshold" + nstr);

    settings->reset("release-time" + nstr);

    settings->reset("ratio" + nstr);

    settings->reset("knee" + nstr);

    settings->reset("makeup" + nstr);

    settings->reset("compression-mode" + nstr);

    settings->reset("sidechain-mode" + nstr);

    settings->reset("sidechain-source" + nstr);

    settings->reset("sidechain-lookahead" + nstr);

    settings->reset("sidechain-reactivity" + nstr);

    settings->reset("sidechain-preamp" + nstr);

    settings->reset("sidechain-custom-lowcut-filter" + nstr);

    settings->reset("sidechain-custom-highcut-filter" + nstr);

    settings->reset("sidechain-lowcut-frequency" + nstr);

    settings->reset("sidechain-highcut-frequency" + nstr);

    settings->reset("boost-threshold" + nstr);

    settings->reset("boost-amount" + nstr);
  }
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
