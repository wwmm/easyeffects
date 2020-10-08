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

#include "equalizer_ui.hpp"
#include <glibmm/i18n.h>
#include <gtkmm/filechoosernative.h>
#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cstddef>
#include <cstring>
#include <regex>
#include "gtkmm/dialog.h"
#include "gtkmm/window.h"

namespace {

auto bandtype_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "Off") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "Bell") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "Hi-pass") == 0) {
    g_value_set_int(value, 2);
  } else if (std::strcmp(v, "Hi-shelf") == 0) {
    g_value_set_int(value, 3);
  } else if (std::strcmp(v, "Lo-pass") == 0) {
    g_value_set_int(value, 4);
  } else if (std::strcmp(v, "Lo-shelf") == 0) {
    g_value_set_int(value, 5);
  } else if (std::strcmp(v, "Notch") == 0) {
    g_value_set_int(value, 6);
  } else if (std::strcmp(v, "Resonance") == 0) {
    g_value_set_int(value, 7);
  }

  return 1;
}

auto int_to_bandtype_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("Off");

    case 1:
      return g_variant_new_string("Bell");

    case 2:
      return g_variant_new_string("Hi-pass");

    case 3:
      return g_variant_new_string("Hi-shelf");

    case 4:
      return g_variant_new_string("Lo-pass");

    case 5:
      return g_variant_new_string("Lo-shelf");

    case 6:
      return g_variant_new_string("Notch");

    case 7:
      return g_variant_new_string("Resonance");

    default:
      return g_variant_new_string("Bell");
  }
}

auto mode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "IIR") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "FIR") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "FFT") == 0) {
    g_value_set_int(value, 2);
  }

  return 1;
}

auto int_to_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("IIR");

    case 1:
      return g_variant_new_string("FIR");

    case 3:
      return g_variant_new_string("FFT");

    default:
      return g_variant_new_string("IIR");
  }
}

auto bandmode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "RLC (BT)") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "RLC (MT)") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "BWC (BT)") == 0) {
    g_value_set_int(value, 2);
  } else if (std::strcmp(v, "BWC (MT)") == 0) {
    g_value_set_int(value, 3);
  } else if (std::strcmp(v, "LRX (BT)") == 0) {
    g_value_set_int(value, 4);
  } else if (std::strcmp(v, "LRX (MT)") == 0) {
    g_value_set_int(value, 5);
  } else if (std::strcmp(v, "APO (DR)") == 0) {
    g_value_set_int(value, 6);
  }

  return 1;
}

auto int_to_bandmode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("RLC (BT)");

    case 1:
      return g_variant_new_string("RLC (MT)");

    case 2:
      return g_variant_new_string("BWC (BT)");

    case 3:
      return g_variant_new_string("BWC (MT)");

    case 4:
      return g_variant_new_string("LRX (BT)");

    case 5:
      return g_variant_new_string("LRX (MT)");

    case 6:
      return g_variant_new_string("APO (DR)");

    default:
      return g_variant_new_string("RLC (BT)");
  }
}

auto bandslope_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "x1") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "x2") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "x3") == 0) {
    g_value_set_int(value, 2);
  } else if (std::strcmp(v, "x4") == 0) {
    g_value_set_int(value, 3);
  }

  return 1;
}

auto int_to_bandslope_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("x1");

    case 1:
      return g_variant_new_string("x2");

    case 2:
      return g_variant_new_string("x3");

    case 3:
      return g_variant_new_string("x4");

    default:
      return g_variant_new_string("x1");
  }
}

}  // namespace

EqualizerUi::EqualizerUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& schema,
                         const std::string& schema_path,
                         const std::string& schema_channel,
                         const std::string& schema_channel_left_path,
                         const std::string& schema_channel_right_path)
    : Gtk::Grid(cobject),
      PluginUiBase(builder, schema, schema_path),
      settings_left(Gio::Settings::create(schema_channel, schema_channel_left_path)),
      settings_right(Gio::Settings::create(schema_channel, schema_channel_right_path)) {
  name = "equalizer";

  // loading glade widgets

  builder->get_widget("bands_grid_left", bands_grid_left);
  builder->get_widget("bands_grid_right", bands_grid_right);
  builder->get_widget("flat_response", flat_response);
  builder->get_widget("import_apo", import_apo);
  builder->get_widget("calculate_freqs", calculate_freqs);
  builder->get_widget("presets_listbox", presets_listbox);
  builder->get_widget("split_channels", split_channels);
  builder->get_widget("stack", stack);
  builder->get_widget("stack_switcher", stack_switcher);
  builder->get_widget("mode", mode);
  builder->get_widget("plugin_reset", reset_button);

  get_object(builder, "nbands", nbands);
  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);

  // signals connections

  nbands->signal_value_changed().connect(sigc::mem_fun(*this, &EqualizerUi::on_nbands_changed));

  // reset equalizer
  reset_button->signal_clicked().connect(sigc::mem_fun(*this, &EqualizerUi::reset));

  flat_response->signal_clicked().connect(sigc::mem_fun(*this, &EqualizerUi::on_flat_response));

  calculate_freqs->signal_clicked().connect(sigc::mem_fun(*this, &EqualizerUi::on_calculate_frequencies));

  presets_listbox->set_sort_func(sigc::ptr_fun(&EqualizerUi::on_listbox_sort));

  import_apo->signal_clicked().connect(sigc::mem_fun(*this, &EqualizerUi::on_import_apo_preset_clicked));

  connections.emplace_back(settings->signal_changed("split-channels").connect([&](auto key) {
    for (auto& c : connections_bands) {
      c.disconnect();
    }

    connections_bands.clear();

    stack->set_visible_child("left_channel");

    bool split = settings->get_boolean("split-channels");

    if (split) {
      build_bands(bands_grid_left, settings_left, static_cast<int>(nbands->get_value()));
      build_bands(bands_grid_right, settings_right, static_cast<int>(nbands->get_value()));
    } else {
      build_unified_bands(static_cast<int>(nbands->get_value()));
    }
  }));

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;
  auto flag_get = Gio::SettingsBindFlags::SETTINGS_BIND_GET;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("num-bands", nbands.get(), "value", flag);
  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);
  settings->bind("split-channels", split_channels, "active", flag);
  settings->bind("split-channels", stack_switcher, "visible", flag_get);

  g_settings_bind_with_mapping(settings->gobj(), "mode", mode->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               mode_enum_to_int, int_to_mode_enum, nullptr, nullptr);

  populate_presets_listbox();
}

EqualizerUi::~EqualizerUi() {
  for (auto& c : connections_bands) {
    c.disconnect();
  }

  util::debug(name + " ui destroyed");
}

void EqualizerUi::on_nbands_changed() {
  for (auto& c : connections_bands) {
    c.disconnect();
  }

  connections_bands.clear();

  bool split = settings->get_boolean("split-channels");

  if (split) {
    build_bands(bands_grid_left, settings_left, static_cast<int>(nbands->get_value()));
    build_bands(bands_grid_right, settings_right, static_cast<int>(nbands->get_value()));
  } else {
    build_unified_bands(static_cast<int>(nbands->get_value()));
  }
}

void EqualizerUi::build_bands(Gtk::Grid* bands_grid, const Glib::RefPtr<Gio::Settings>& cfg, const int& nbands) {
  for (const auto& c : bands_grid->get_children()) {
    bands_grid->remove(*c);

    delete c;
  }

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  for (int n = 0; n < nbands; n++) {
    auto B = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/equalizer_band.glade");

    Gtk::Grid* band_grid = nullptr;
    Gtk::ComboBoxText* band_type = nullptr;
    Gtk::ComboBoxText* band_mode = nullptr;
    Gtk::ComboBoxText* band_slope = nullptr;
    Gtk::Label* band_width = nullptr;
    Gtk::Label* band_label = nullptr;
    Gtk::Label* band_quality_label = nullptr;
    Gtk::Label* band_gain_label = nullptr;
    Gtk::Button* reset_frequency = nullptr;
    Gtk::Button* reset_quality = nullptr;
    Gtk::ToggleButton* band_solo = nullptr;
    Gtk::ToggleButton* band_mute = nullptr;
    Gtk::Scale* band_scale = nullptr;

    B->get_widget("band_grid", band_grid);
    B->get_widget("band_type", band_type);
    B->get_widget("band_mode", band_mode);
    B->get_widget("band_slope", band_slope);
    B->get_widget("band_width", band_width);
    B->get_widget("band_label", band_label);
    B->get_widget("band_quality_label", band_quality_label);
    B->get_widget("band_gain_label", band_gain_label);
    B->get_widget("band_solo", band_solo);
    B->get_widget("band_mute", band_mute);
    B->get_widget("band_scale", band_scale);
    B->get_widget("reset_frequency", reset_frequency);
    B->get_widget("reset_quality", reset_quality);

    auto band_gain = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_gain"));
    auto band_frequency = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_frequency"));
    auto band_quality = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_quality"));

    auto update_quality_width = [=]() {
      auto q = band_quality->get_value();

      band_quality_label->set_text("Q " + level_to_str(q, 2));

      if (q > 0.0) {
        auto f = band_frequency->get_value();

        band_width->set_text(level_to_str(f / q, 1) + " Hz");
      } else {
        band_width->set_text(_("infinity"));
      }
    };

    auto update_band_label = [=]() {
      auto f = band_frequency->get_value();

      if (f > 1000.0) {
        band_label->set_text(level_to_str(f / 1000.0, 1) + " kHz");
      } else {
        band_label->set_text(level_to_str(f, 0) + " Hz");
      }
    };

    auto update_gain = [=]() {
      auto g = band_gain->get_value();

      band_gain_label->set_text(level_to_str_showpos(g, 2));
    };

    // set initial band gain in relative label

    band_gain_label->set_text(level_to_str_showpos(band_gain->get_value(), 2));

    // connections

    connections_bands.emplace_back(band_frequency->signal_value_changed().connect(update_quality_width));

    connections_bands.emplace_back(band_frequency->signal_value_changed().connect(update_band_label));

    connections_bands.emplace_back(band_quality->signal_value_changed().connect(update_quality_width));

    connections_bands.emplace_back(band_gain->signal_value_changed().connect(update_gain));

    connections_bands.emplace_back(reset_frequency->signal_clicked().connect(
        [=]() { cfg->reset(std::string("band" + std::to_string(n) + "-frequency")); }));

    connections_bands.emplace_back(
        reset_quality->signal_clicked().connect([=]() { cfg->reset(std::string("band" + std::to_string(n) + "-q")); }));

    connections_bands.emplace_back(band_type->signal_changed().connect([=]() {
      if (band_type->get_active_row_number() == 1 || band_type->get_active_row_number() == 3 ||
          band_type->get_active_row_number() == 5 || band_type->get_active_row_number() == 7) {
        band_scale->set_sensitive(true);
      } else {
        band_scale->set_sensitive(false);
      }
    }));

    cfg->bind(std::string("band" + std::to_string(n) + "-gain"), band_gain.get(), "value", flag);
    cfg->bind(std::string("band" + std::to_string(n) + "-frequency"), band_frequency.get(), "value", flag);
    cfg->bind(std::string("band" + std::to_string(n) + "-q"), band_quality.get(), "value", flag);
    cfg->bind(std::string("band" + std::to_string(n) + "-solo"), band_solo, "active", flag);
    cfg->bind(std::string("band" + std::to_string(n) + "-mute"), band_mute, "active", flag);

    g_settings_bind_with_mapping(cfg->gobj(), std::string("band" + std::to_string(n) + "-type").c_str(),
                                 band_type->gobj(), "active", G_SETTINGS_BIND_DEFAULT, bandtype_enum_to_int,
                                 int_to_bandtype_enum, nullptr, nullptr);

    g_settings_bind_with_mapping(cfg->gobj(), std::string("band" + std::to_string(n) + "-mode").c_str(),
                                 band_mode->gobj(), "active", G_SETTINGS_BIND_DEFAULT, bandmode_enum_to_int,
                                 int_to_bandmode_enum, nullptr, nullptr);

    g_settings_bind_with_mapping(cfg->gobj(), std::string("band" + std::to_string(n) + "-slope").c_str(),
                                 band_slope->gobj(), "active", G_SETTINGS_BIND_DEFAULT, bandslope_enum_to_int,
                                 int_to_bandslope_enum, nullptr, nullptr);

    bands_grid->add(*band_grid);
  }

  bands_grid->show_all();
}

void EqualizerUi::build_unified_bands(const int& nbands) {
  for (const auto& c : bands_grid_left->get_children()) {
    bands_grid_left->remove(*c);

    delete c;
  }

  for (const auto& c : bands_grid_right->get_children()) {
    bands_grid_right->remove(*c);

    delete c;
  }

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  for (int n = 0; n < nbands; n++) {
    auto B = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/equalizer_band.glade");

    Gtk::Grid* band_grid = nullptr;
    Gtk::ComboBoxText* band_type = nullptr;
    Gtk::ComboBoxText* band_mode = nullptr;
    Gtk::ComboBoxText* band_slope = nullptr;
    Gtk::Label* band_width = nullptr;
    Gtk::Label* band_label = nullptr;
    Gtk::Label* band_quality_label = nullptr;
    Gtk::Label* band_gain_label = nullptr;
    Gtk::Button* reset_frequency = nullptr;
    Gtk::Button* reset_quality = nullptr;
    Gtk::ToggleButton* band_solo = nullptr;
    Gtk::ToggleButton* band_mute = nullptr;
    Gtk::Scale* band_scale = nullptr;

    B->get_widget("band_grid", band_grid);
    B->get_widget("band_type", band_type);
    B->get_widget("band_mode", band_mode);
    B->get_widget("band_slope", band_slope);
    B->get_widget("band_width", band_width);
    B->get_widget("band_label", band_label);
    B->get_widget("band_quality_label", band_quality_label);
    B->get_widget("band_gain_label", band_gain_label);
    B->get_widget("band_solo", band_solo);
    B->get_widget("band_mute", band_mute);
    B->get_widget("band_scale", band_scale);
    B->get_widget("reset_frequency", reset_frequency);
    B->get_widget("reset_quality", reset_quality);

    auto band_gain = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_gain"));
    auto band_frequency = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_frequency"));
    auto band_quality = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_quality"));

    auto update_quality_width = [=]() {
      auto q = band_quality->get_value();

      band_quality_label->set_text("Q " + level_to_str(q, 2));

      if (q > 0.0) {
        auto f = band_frequency->get_value();

        band_width->set_text(level_to_str(f / q, 1) + " Hz");
      } else {
        band_width->set_text(_("infinity"));
      }
    };

    auto update_band_label = [=]() {
      auto f = band_frequency->get_value();

      if (f > 1000.0) {
        band_label->set_text(level_to_str(f / 1000.0, 1) + " kHz");
      } else {
        band_label->set_text(level_to_str(f, 0) + " Hz");
      }
    };

    auto update_gain = [=]() {
      auto g = band_gain->get_value();

      band_gain_label->set_text(level_to_str_showpos(g, 2));
    };

    // set initial band gain in relative label

    band_gain_label->set_text(level_to_str_showpos(band_gain->get_value(), 2));

    // connections

    connections_bands.emplace_back(band_frequency->signal_value_changed().connect(update_quality_width));

    connections_bands.emplace_back(band_frequency->signal_value_changed().connect(update_band_label));

    connections_bands.emplace_back(band_quality->signal_value_changed().connect(update_quality_width));

    connections_bands.emplace_back(band_gain->signal_value_changed().connect(update_gain));

    /*right channel
      we need the bindgins below for the right channel equalizer to be updated
      they have to be before the bindings for the left channel.
     */

    connections_bands.emplace_back(band_gain->signal_value_changed().connect([=]() {
      settings_right->set_double(std::string("band" + std::to_string(n) + "-gain"), band_gain->get_value());
    }));

    connections_bands.emplace_back(band_frequency->signal_value_changed().connect([=]() {
      settings_right->set_double(std::string("band" + std::to_string(n) + "-frequency"), band_frequency->get_value());
    }));

    connections_bands.emplace_back(band_quality->signal_value_changed().connect([=]() {
      settings_right->set_double(std::string("band" + std::to_string(n) + "-q"), band_quality->get_value());
    }));

    connections_bands.emplace_back(band_type->signal_changed().connect([=]() {
      settings_right->set_enum(std::string("band" + std::to_string(n) + "-type"), band_type->get_active_row_number());
    }));

    connections_bands.emplace_back(band_mode->signal_changed().connect([=]() {
      settings_right->set_enum(std::string("band" + std::to_string(n) + "-mode"), band_mode->get_active_row_number());
    }));

    connections_bands.emplace_back(band_slope->signal_changed().connect([=]() {
      settings_right->set_enum(std::string("band" + std::to_string(n) + "-slope"), band_slope->get_active_row_number());
    }));

    connections_bands.emplace_back(band_solo->signal_toggled().connect([=]() {
      settings_right->set_boolean(std::string("band" + std::to_string(n) + "-solo"), band_solo->get_active());
    }));

    connections_bands.emplace_back(band_mute->signal_toggled().connect([=]() {
      settings_right->set_boolean(std::string("band" + std::to_string(n) + "-mute"), band_mute->get_active());
    }));

    // left channel

    connections_bands.emplace_back(band_type->signal_changed().connect([=]() {
      if (band_type->get_active_row_number() == 1 || band_type->get_active_row_number() == 3 ||
          band_type->get_active_row_number() == 5 || band_type->get_active_row_number() == 7) {
        band_scale->set_sensitive(true);
      } else {
        band_scale->set_sensitive(false);
      }
    }));

    connections_bands.emplace_back(reset_frequency->signal_clicked().connect([=]() {
      settings_left->reset(std::string("band" + std::to_string(n) + "-frequency"));

      settings_right->reset(std::string("band" + std::to_string(n) + "-frequency"));
    }));

    connections_bands.emplace_back(reset_quality->signal_clicked().connect([=]() {
      settings_left->reset(std::string("band" + std::to_string(n) + "-q"));

      settings_right->reset(std::string("band" + std::to_string(n) + "-q"));
    }));

    settings_left->bind(std::string("band" + std::to_string(n) + "-gain"), band_gain.get(), "value", flag);
    settings_left->bind(std::string("band" + std::to_string(n) + "-frequency"), band_frequency.get(), "value", flag);
    settings_left->bind(std::string("band" + std::to_string(n) + "-q"), band_quality.get(), "value", flag);
    settings_left->bind(std::string("band" + std::to_string(n) + "-solo"), band_solo, "active", flag);
    settings_left->bind(std::string("band" + std::to_string(n) + "-mute"), band_mute, "active", flag);

    g_settings_bind_with_mapping(settings_left->gobj(), std::string("band" + std::to_string(n) + "-type").c_str(),
                                 band_type->gobj(), "active", G_SETTINGS_BIND_DEFAULT, bandtype_enum_to_int,
                                 int_to_bandtype_enum, nullptr, nullptr);

    g_settings_bind_with_mapping(settings_left->gobj(), std::string("band" + std::to_string(n) + "-mode").c_str(),
                                 band_mode->gobj(), "active", G_SETTINGS_BIND_DEFAULT, bandmode_enum_to_int,
                                 int_to_bandmode_enum, nullptr, nullptr);

    g_settings_bind_with_mapping(settings_left->gobj(), std::string("band" + std::to_string(n) + "-slope").c_str(),
                                 band_slope->gobj(), "active", G_SETTINGS_BIND_DEFAULT, bandslope_enum_to_int,
                                 int_to_bandslope_enum, nullptr, nullptr);

    bands_grid_left->add(*band_grid);
  }

  bands_grid_left->show_all();
}

void EqualizerUi::on_flat_response() {
  for (int n = 0; n < max_bands; n++) {
    // left channel

    settings_left->reset(std::string("band" + std::to_string(n) + "-gain"));

    // right channel

    settings_right->reset(std::string("band" + std::to_string(n) + "-gain"));
  }
}

void EqualizerUi::on_calculate_frequencies() {
  const double min_freq = 20.0;
  const double max_freq = 20000.0;
  double freq0 = 0.0;
  double freq1 = 0.0;
  double step = 0.0;

  int nbands = settings->get_int("num-bands");

  // code taken from gstreamer equalizer sources: gstiirequalizer.c
  // function: gst_iir_equalizer_compute_frequencies

  step = pow(max_freq / min_freq, 1.0 / nbands);
  freq0 = min_freq;

  auto config_band = [&](auto cfg, auto n, auto freq, auto q) {
    cfg->set_double(std::string("band" + std::to_string(n) + "-frequency"), freq);

    cfg->set_double(std::string("band" + std::to_string(n) + "-q"), q);
  };

  for (int n = 0; n < nbands; n++) {
    freq1 = freq0 * step;

    double freq = freq0 + 0.5 * (freq1 - freq0);
    double width = freq1 - freq0;
    double q = freq / width;

    // std::cout << n << "\t" << freq << "\t" << width << std::endl;

    config_band(settings_left, n, freq, q);
    config_band(settings_right, n, freq, q);

    freq0 = freq1;
  }
}

void EqualizerUi::load_preset(const std::string& file_name) {
  gsize dsize = 0;
  std::stringstream ss;
  boost::property_tree::ptree root;

  auto bytes = Gio::Resource::lookup_data_global(presets_path + file_name);

  const auto* rdata = static_cast<const char*>(bytes->get_data(dsize));

  auto file_contents = std::string(rdata);

  // std::cout << file_contents << std::endl;

  ss << file_contents;

  boost::property_tree::read_json(ss, root);

  int nbands = root.get<int>("equalizer.num-bands");

  settings->set_int("num-bands", nbands);

  settings->set_string("mode", root.get<std::string>("equalizer.mode"));

  settings->set_double("input-gain", root.get<double>("equalizer.input-gain"));

  settings->set_double("output-gain", root.get<double>("equalizer.output-gain"));

  auto config_band = [&](auto cfg, auto n) {
    double q = 0.0;

    auto f = root.get<double>("equalizer.band" + std::to_string(n) + ".frequency");

    try {
      q = root.get<double>("equalizer.band" + std::to_string(n) + ".q");
    } catch (const boost::property_tree::ptree_error& e) {
      try {
        auto w = root.get<double>("equalizer.band" + std::to_string(n) + ".width");

        q = f / w;
      } catch (const boost::property_tree::ptree_error& e) {
      }
    }

    cfg->set_double(std::string("band" + std::to_string(n) + "-gain"),
                    root.get<double>("equalizer.band" + std::to_string(n) + ".gain"));

    cfg->set_double(std::string("band" + std::to_string(n) + "-frequency"), f);

    cfg->set_double(std::string("band" + std::to_string(n) + "-q"), q);

    cfg->set_string(std::string("band" + std::to_string(n) + "-type"),
                    root.get<std::string>("equalizer.band" + std::to_string(n) + ".type"));

    cfg->set_string(std::string("band" + std::to_string(n) + "-mode"),
                    root.get<std::string>("equalizer.band" + std::to_string(n) + ".mode"));

    cfg->set_string(std::string("band" + std::to_string(n) + "-slope"),
                    root.get<std::string>("equalizer.band" + std::to_string(n) + ".slope"));

    cfg->set_boolean(std::string("band" + std::to_string(n) + "-solo"),
                     root.get<bool>("equalizer.band" + std::to_string(n) + ".solo"));

    cfg->set_boolean(std::string("band" + std::to_string(n) + "-mute"),
                     root.get<bool>("equalizer.band" + std::to_string(n) + ".mute"));
  };

  for (int n = 0; n < nbands; n++) {
    config_band(settings_left, n);
    config_band(settings_right, n);
  }
}

auto EqualizerUi::on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) -> int {
  auto name1 = row1->get_name();
  auto name2 = row2->get_name();

  std::vector<std::string> names = {name1, name2};

  std::sort(names.begin(), names.end());

  if (name1 == names[0]) {
    return -1;
  }

  if (name2 == names[0]) {
    return 1;
  }

  return 0;
}

void EqualizerUi::populate_presets_listbox() {
  auto children = presets_listbox->get_children();

  for (const auto& c : children) {
    presets_listbox->remove(*c);
  }

  auto names = Gio::Resource::enumerate_children_global(presets_path);

  for (const auto& file_name : names) {
    auto name = file_name.substr(0, file_name.find('.'));

    auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/equalizer_preset_row.glade");

    Gtk::ListBoxRow* row = nullptr;
    Gtk::Button* apply_btn = nullptr;
    Gtk::Label* label = nullptr;

    b->get_widget("preset_row", row);
    b->get_widget("apply", apply_btn);
    b->get_widget("name", label);

    row->set_name(name);

    label->set_text(name);

    connections.emplace_back(apply_btn->signal_clicked().connect([=]() { load_preset(row->get_name() + ".json"); }));

    presets_listbox->add(*row);

    presets_listbox->show_all();
  }
}

void EqualizerUi::reset() {
  try {
    settings->reset("mode");
    settings->reset("num-bands");
    settings->reset("split-channels");
    settings->reset("input-gain");
    settings->reset("output-gain");

    for (int n = 0; n < max_bands; n++) {
      // left channel

      settings_left->reset(std::string("band" + std::to_string(n) + "-gain"));
      settings_left->reset(std::string("band" + std::to_string(n) + "-frequency"));
      settings_left->reset(std::string("band" + std::to_string(n) + "-q"));
      settings_left->reset(std::string("band" + std::to_string(n) + "-type"));
      settings_left->reset(std::string("band" + std::to_string(n) + "-mode"));
      settings_left->reset(std::string("band" + std::to_string(n) + "-slope"));
      settings_left->reset(std::string("band" + std::to_string(n) + "-solo"));
      settings_left->reset(std::string("band" + std::to_string(n) + "-mute"));

      // right channel

      settings_right->reset(std::string("band" + std::to_string(n) + "-gain"));
      settings_right->reset(std::string("band" + std::to_string(n) + "-frequency"));
      settings_right->reset(std::string("band" + std::to_string(n) + "-q"));
      settings_right->reset(std::string("band" + std::to_string(n) + "-type"));
      settings_right->reset(std::string("band" + std::to_string(n) + "-mode"));
      settings_right->reset(std::string("band" + std::to_string(n) + "-slope"));
      settings_right->reset(std::string("band" + std::to_string(n) + "-solo"));
      settings_right->reset(std::string("band" + std::to_string(n) + "-mute"));
    }

    util::debug(name + " plugin: successfully reset");
  } catch (std::exception& e) {
    util::debug(name + " plugin: an error occurred during reset process");
  }
}

void EqualizerUi::on_import_apo_preset_clicked() {
  auto* main_window = dynamic_cast<Gtk::Window*>(this->get_toplevel());

  auto dialog =
      Gtk::FileChooserNative::create(_("Import APO Preset File"), *main_window,
                                     Gtk::FileChooserAction::FILE_CHOOSER_ACTION_OPEN, _("Open"), _("Cancel"));

  auto dialog_filter = Gtk::FileFilter::create();

  dialog_filter->set_name(_("APO Presets"));
  dialog_filter->add_pattern("*.txt");

  dialog->add_filter(dialog_filter);

  dialog->signal_response().connect([=](auto response_id) {
    switch (response_id) {
      case Gtk::ResponseType::RESPONSE_ACCEPT: {
        import_apo_preset(dialog->get_file()->get_path());

        break;
      }
      default:
        break;
    }
  });

  dialog->set_modal(true);
  dialog->show();
}

// returns false if we cannot parse given line successfully
auto EqualizerUi::parse_apo_filter(const std::string& line, struct ImportedBand& filter) -> bool {
  std::smatch matches;
  std::regex re_filter_type(R"(Filter[\s]+[\d]*:[\s]*ON[\s]+([\w]+))");
  std::regex re_freq(R"([\s]+Fc[\s]*([\d]*[.]?[\d]*)[\s]*Hz)");
  std::regex re_dB_per_octave(R"(Filter[\s]+[\d]*:[\s]*ON[\s]+[\w]+[\s]+([+-]?[\d]*[.]?[\d]*)[\s]*dB)");
  std::regex re_gain(R"(Gain[\s]*([+-]?[\d]*[.]?[\d]*)[\s]*dB)");
  std::regex re_quality_factor(R"([\s]+Q[\s]+([\d]*[.]?[\d]*))");

  // get filter type
  std::regex_search(line, matches, re_filter_type);

  if (matches.size() != 2U) {
    return false;
  }

  try {
    filter.type = EqualizerUi::FilterTypeMap.at(matches.str(1));
  } catch (...) {
    return false;
  }

  // get center frequency
  std::regex_search(line, matches, re_freq);

  if (matches.size() != 2U) {
    return false;
  }

  filter.freq = std::stof(matches.str(1));

  // get slope

  if ((filter.type & (LOW_SHELF_xdB | HIGH_SHELF_xdB | LOW_SHELF | HIGH_SHELF)) != 0U) {
    std::regex_search(line, matches, re_dB_per_octave);
    // _xdB variants require the dB parameter
    if (((filter.type & (LOW_SHELF_xdB | HIGH_SHELF_xdB)) != 0U) && (matches.size() != 2U)) {
      return false;
    }

    if (matches.size() == 2U) {
      // we satisfied the condition, now assign the paramater if given
      filter.slope_dB = std::stof(matches.str(1));
    }
  }

  // get gain

  if ((filter.type & (PEAKING | LOW_SHELF_xdB | HIGH_SHELF_xdB | LOW_SHELF | HIGH_SHELF)) != 0U) {
    std::regex_search(line, matches, re_gain);
    // all Shelf types (i.e. all above except for Peaking) require the gain parameter
    if (((filter.type & PEAKING) == 0U) && (matches.size() != 2U)) {
      return false;
    }

    if (matches.size() == 2U) {
      filter.gain = std::stof(matches.str(1));
    }
  }

  // get quality factor
  if ((filter.type & (PEAKING | LOW_PASS_Q | HIGH_PASS_Q | LOW_SHELF_xdB | HIGH_SHELF_xdB | NOTCH | ALL_PASS)) != 0U) {
    std::regex_search(line, matches, re_quality_factor);
    // Peaking and All-Pass filter types require the quality factor parameter
    if (((filter.type & (PEAKING | ALL_PASS)) != 0U) && (matches.size() != 2U)) {
      return false;
    }

    if (matches.size() == 2U) {
      filter.quality_factor = std::stof(matches.str(1));
    }
  }

  return true;
}

void EqualizerUi::import_apo_preset(const std::string& file_path) {
  boost::filesystem::path p{file_path};

  if (boost::filesystem::is_regular_file(p)) {
    std::ifstream eq_file;
    std::vector<struct ImportedBand> bands;

    eq_file.open(p.string());

    if (eq_file.is_open()) {
      std::string line;

      while (getline(eq_file, line)) {
        struct ImportedBand filter {};
        bool parsed = this->parse_apo_filter(line, filter);

        if (parsed) {
          bands.push_back(filter);
        }
      }
    }

    eq_file.close();

    if (bands.empty()) {
      return;
    }

    settings->set_int("num-bands", bands.size());

    for (int n = 0; n < max_bands; n++) {
      if (n < static_cast<int>(bands.size())) {
        settings_left->set_string(std::string("band" + std::to_string(n) + "-mode"), "APO (DR)");

        settings_left->set_string(std::string("band" + std::to_string(n) + "-type"), "Bell");
        settings_left->set_double(std::string("band" + std::to_string(n) + "-gain"), bands[n].gain);
        settings_left->set_double(std::string("band" + std::to_string(n) + "-frequency"), bands[n].freq);
        settings_left->set_double(std::string("band" + std::to_string(n) + "-q"), bands[n].quality_factor);

        settings_right->set_string(std::string("band" + std::to_string(n) + "-type"), "Bell");
        settings_right->set_double(std::string("band" + std::to_string(n) + "-gain"), bands[n].gain);
        settings_right->set_double(std::string("band" + std::to_string(n) + "-frequency"), bands[n].freq);
        settings_right->set_double(std::string("band" + std::to_string(n) + "-q"), bands[n].quality_factor);
      } else {
        settings_left->set_string(std::string("band" + std::to_string(n) + "-type"), "Off");

        settings_right->set_string(std::string("band" + std::to_string(n) + "-type"), "Off");
      }
    }
  }
}
