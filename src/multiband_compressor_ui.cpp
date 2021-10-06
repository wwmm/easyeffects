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

#include "multiband_compressor_ui.hpp"

namespace {

auto compressor_mode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (g_strcmp0(v, "Classic") == 0) {
    g_value_set_int(value, 0);
  } else if (g_strcmp0(v, "Modern") == 0) {
    g_value_set_int(value, 1);
  }

  return 1;
}

auto int_to_compressor_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  switch (g_value_get_int(value)) {
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

  if (g_strcmp0(v, "None") == 0) {
    g_value_set_int(value, 0);
  } else if (g_strcmp0(v, "Pink BT") == 0) {
    g_value_set_int(value, 1);
  } else if (g_strcmp0(v, "Pink MT") == 0) {
    g_value_set_int(value, 2);
  } else if (g_strcmp0(v, "Brown BT") == 0) {
    g_value_set_int(value, 3);
  } else if (g_strcmp0(v, "Brown MT") == 0) {
    g_value_set_int(value, 4);
  }

  return 1;
}

auto int_to_envelope_boost_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  switch (g_value_get_int(value)) {
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

  if (g_strcmp0(v, "Downward") == 0) {
    g_value_set_int(value, 0);
  } else if (g_strcmp0(v, "Upward") == 0) {
    g_value_set_int(value, 1);
  } else if (g_strcmp0(v, "Boosting") == 0) {
    g_value_set_int(value, 2);
  }

  return 1;
}

auto int_to_compression_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  switch (g_value_get_int(value)) {
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

  if (g_strcmp0(v, "Peak") == 0) {
    g_value_set_int(value, 0);
  } else if (g_strcmp0(v, "RMS") == 0) {
    g_value_set_int(value, 1);
  } else if (g_strcmp0(v, "Low-Pass") == 0) {
    g_value_set_int(value, 2);
  } else if (g_strcmp0(v, "Uniform") == 0) {
    g_value_set_int(value, 3);
  }

  return 1;
}

auto int_to_sidechain_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  switch (g_value_get_int(value)) {
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

  if (g_strcmp0(v, "Middle") == 0) {
    g_value_set_int(value, 0);
  } else if (g_strcmp0(v, "Side") == 0) {
    g_value_set_int(value, 1);
  } else if (g_strcmp0(v, "Left") == 0) {
    g_value_set_int(value, 2);
  } else if (g_strcmp0(v, "Right") == 0) {
    g_value_set_int(value, 3);
  }

  return 1;
}

auto int_to_sidechain_source_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  switch (g_value_get_int(value)) {
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

  compressor_mode = builder->get_widget<Gtk::ComboBoxText>("compressor_mode");

  envelope_boost = builder->get_widget<Gtk::ComboBoxText>("envelope_boost");

  stack = builder->get_widget<Gtk::Stack>("stack");

  listbox = builder->get_widget<Gtk::ListBox>("listbox");

  listbox->select_row(*listbox->get_row_at_index(0));

  listbox->signal_selected_rows_changed().connect([=, this]() {
    // some core dumps happened here
    // checking pointer and int row integrity might fix them

    if (const auto* selected_row = listbox->get_selected_row(); selected_row != nullptr) {
      if (const auto& row = selected_row->get_index(); row > -1) {
        stack->set_visible_child("band" + std::to_string(row));
      }
    }
  });

  // gsettings bindings

  g_settings_bind_with_mapping(settings->gobj(), "compressor-mode", compressor_mode->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, compressor_mode_enum_to_int, int_to_compressor_mode_enum,
                               nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "envelope-boost", envelope_boost->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, envelope_boost_enum_to_int, int_to_envelope_boost_enum, nullptr,
                               nullptr);

  // band checkbuttons

  for (uint n = 1U; n < n_bands; n++) {
    const auto& nstr = std::to_string(n);

    auto* const enable_band = builder->get_widget<Gtk::CheckButton>("enable_band" + nstr);

    settings->bind("enable-band" + nstr, enable_band, "active");
  }

  prepare_bands();

  setup_input_output_gain(builder);
}

MultibandCompressorUi::~MultibandCompressorUi() {
  util::debug(name + " ui destroyed");
}

auto MultibandCompressorUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> MultibandCompressorUi* {
  const auto& builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/multiband_compressor.ui");

  auto* const ui = Gtk::Builder::get_widget_derived<MultibandCompressorUi>(
      builder, "top_box", "com.github.wwmm.easyeffects.multibandcompressor", schema_path + "multibandcompressor/");

  stack->add(*ui, plugin_name::multiband_compressor);

  return ui;
}

void MultibandCompressorUi::prepare_bands() {
  for (uint n = 0U; n < n_bands; n++) {
    const auto& nstr = std::to_string(n);

    const auto& builder =
        Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/multiband_compressor_band.ui");

    if (n > 0U) {
      auto* const split_frequency = builder->get_widget<Gtk::SpinButton>("split_frequency");

      settings->bind("split-frequency" + nstr, split_frequency->get_adjustment().get(), "value");

      prepare_spinbutton(split_frequency, "Hz");
    } else {
      // removing split frequency from band 0

      auto* const sf_box = builder->get_widget<Gtk::Box>("split_frequency_box");

      for (auto* child = sf_box->get_last_child(); child != nullptr; child = sf_box->get_last_child()) {
        sf_box->remove(*child);
      }

      auto sf_label = Gtk::Label("0 Hz", Gtk::Align::CENTER);

      sf_box->append(sf_label);
    }

    // loading builder widgets

    bands_end.at(n) = builder->get_widget<Gtk::Label>("band_end");

    bands_gain_label.at(n) = builder->get_widget<Gtk::Label>("band_gain_label");

    bands_envelope_label.at(n) = builder->get_widget<Gtk::Label>("band_envelope_label");

    bands_curve_label.at(n) = builder->get_widget<Gtk::Label>("band_curve_label");

    auto* const band_bypass = builder->get_widget<Gtk::ToggleButton>("bypass");

    auto* const mute = builder->get_widget<Gtk::ToggleButton>("mute");

    auto* const solo = builder->get_widget<Gtk::ToggleButton>("solo");

    auto* const lowcut_filter = builder->get_widget<Gtk::CheckButton>("lowcut_filter");

    auto* const highcut_filter = builder->get_widget<Gtk::CheckButton>("highcut_filter");

    auto* const lowcut_filter_frequency = builder->get_widget<Gtk::SpinButton>("lowcut_filter_frequency");

    auto* const highcut_filter_frequency = builder->get_widget<Gtk::SpinButton>("highcut_filter_frequency");

    auto* const attack_time = builder->get_widget<Gtk::SpinButton>("attack_time");

    auto* const attack_threshold = builder->get_widget<Gtk::SpinButton>("attack_threshold");

    auto* const release_time = builder->get_widget<Gtk::SpinButton>("release_time");

    auto* const release_threshold = builder->get_widget<Gtk::SpinButton>("release_threshold");

    auto* const ratio = builder->get_widget<Gtk::SpinButton>("ratio");

    auto* const knee = builder->get_widget<Gtk::SpinButton>("knee");

    auto* const makeup = builder->get_widget<Gtk::SpinButton>("makeup");

    auto* const sidechain_preamp = builder->get_widget<Gtk::SpinButton>("sidechain_preamp");

    auto* const sidechain_reactivity = builder->get_widget<Gtk::SpinButton>("sidechain_reactivity");

    auto* const sidechain_lookahead = builder->get_widget<Gtk::SpinButton>("sidechain_lookahead");

    auto* const boost_amount = builder->get_widget<Gtk::SpinButton>("boost_amount");

    auto* const boost_threshold = builder->get_widget<Gtk::SpinButton>("boost_threshold");

    auto* const compression_mode = builder->get_widget<Gtk::ComboBoxText>("compression_mode");

    auto* const sidechain_mode = builder->get_widget<Gtk::ComboBoxText>("sidechain_mode");

    auto* const sidechain_source = builder->get_widget<Gtk::ComboBoxText>("sidechain_source");

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
                                 compression_mode->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                                 compression_mode_enum_to_int, int_to_compression_mode_enum, nullptr, nullptr);

    g_settings_bind_with_mapping(settings->gobj(), std::string("sidechain-mode" + nstr).c_str(), sidechain_mode->gobj(),
                                 "active", G_SETTINGS_BIND_DEFAULT, sidechain_mode_enum_to_int,
                                 int_to_sidechain_mode_enum, nullptr, nullptr);

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

    // set boost spinbuttons sensitivity on compression mode

    auto set_boost_spinbuttons_sensitivity = [=, this]() {
      const auto& row_id = compression_mode->get_active_id();

      if (row_id == "downward_mode") {
        boost_threshold->set_sensitive(false);
        boost_amount->set_sensitive(false);
      } else if (row_id == "upward_mode") {
        boost_threshold->set_sensitive(true);
        boost_amount->set_sensitive(false);
      } else if (row_id == "boosting_mode") {
        boost_threshold->set_sensitive(false);
        boost_amount->set_sensitive(true);
      } else {
        boost_threshold->set_sensitive(true);
        boost_amount->set_sensitive(true);
      }
    };

    set_boost_spinbuttons_sensitivity();

    compression_mode->signal_changed().connect(set_boost_spinbuttons_sensitivity);

    // add to stack

    auto* const top_box = builder->get_widget<Gtk::Box>("top_box");

    stack->add(*top_box, "band" + nstr);
  }
}

void MultibandCompressorUi::reset() {
  bypass->set_active(false);

  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("compressor-mode");

  settings->reset("envelope-boost");

  for (uint n = 0U; n < n_bands; n++) {
    const auto& nstr = std::to_string(n);

    if (n > 0U) {
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

void MultibandCompressorUi::on_new_frequency_range(const std::array<float, n_bands>& values) {
  for (size_t n = 0U; n < values.size(); n++) {
    bands_end.at(n)->set_text(level_to_localized_string(values.at(n), 0));
  }
}

void MultibandCompressorUi::on_new_envelope(const std::array<float, n_bands>& values) {
  for (size_t n = 0U; n < values.size(); n++) {
    bands_envelope_label.at(n)->set_text(level_to_localized_string(util::linear_to_db(values.at(n)), 0));
  }
}

void MultibandCompressorUi::on_new_curve(const std::array<float, n_bands>& values) {
  for (size_t n = 0U; n < values.size(); n++) {
    bands_curve_label.at(n)->set_text(level_to_localized_string(util::linear_to_db(values.at(n)), 0));
  }
}

void MultibandCompressorUi::on_new_reduction(const std::array<float, n_bands>& values) {
  for (size_t n = 0U; n < values.size(); n++) {
    bands_gain_label.at(n)->set_text(level_to_localized_string(util::linear_to_db(values.at(n)), 0));
  }
}
