#include "equalizer_ui.hpp"
#include <glibmm/i18n.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>
#include <gtkmm/togglebutton.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cstring>

namespace {

auto bandtype_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  auto v = g_variant_get_string(variant, nullptr);

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
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("Off");
  }

  if (v == 1) {
    return g_variant_new_string("Bell");
  }

  if (v == 2) {
    return g_variant_new_string("Hi-pass");
  }

  if (v == 3) {
    return g_variant_new_string("Hi-shelf");
  }

  if (v == 4) {
    return g_variant_new_string("Lo-pass");
  }

  if (v == 5) {
    return g_variant_new_string("Lo-shelf");
  }

  if (v == 6) {
    return g_variant_new_string("Notch");
  }

  return g_variant_new_string("Resonance");
}

auto mode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  auto v = g_variant_get_string(variant, nullptr);

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
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("IIR");
  }

  if (v == 1) {
    return g_variant_new_string("FIR");
  }

  return g_variant_new_string("FFT");
}

auto bandmode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  auto v = g_variant_get_string(variant, nullptr);

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
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("RLC (BT)");
  }

  if (v == 1) {
    return g_variant_new_string("RLC (MT)");
  }

  if (v == 2) {
    return g_variant_new_string("BWC (BT)");
  }

  if (v == 3) {
    return g_variant_new_string("BWC (MT)");
  }

  if (v == 4) {
    return g_variant_new_string("LRX (BT)");
  }

  if (v == 5) {
    return g_variant_new_string("LRX (MT)");
  }

  return g_variant_new_string("APO (DR)");
}

auto bandslope_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  auto v = g_variant_get_string(variant, nullptr);

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
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("x1");
  }

  if (v == 1) {
    return g_variant_new_string("x2");
  }

  if (v == 2) {
    return g_variant_new_string("x3");
  }

  return g_variant_new_string("x4");
}

}  // namespace

EqualizerUi::EqualizerUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& schema,
                         const std::string& schema_left,
                         const std::string& schema_right)
    : Gtk::Grid(cobject),
      PluginUiBase(builder, schema),
      settings_left(Gio::Settings::create(schema_left)),
      settings_right(Gio::Settings::create(schema_right)) {
  name = "equalizer";

  // loading glade widgets

  builder->get_widget("bands_grid_left", bands_grid_left);
  builder->get_widget("bands_grid_right", bands_grid_right);
  builder->get_widget("reset_eq", reset_eq);
  builder->get_widget("flat_response", flat_response);
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
  reset_eq->signal_clicked().connect(sigc::mem_fun(*this, &EqualizerUi::reset));
  reset_button->signal_clicked().connect(sigc::mem_fun(*this, &EqualizerUi::reset));

  flat_response->signal_clicked().connect(sigc::mem_fun(*this, &EqualizerUi::on_flat_response));

  calculate_freqs->signal_clicked().connect(sigc::mem_fun(*this, &EqualizerUi::on_calculate_frequencies));

  presets_listbox->set_sort_func(sigc::ptr_fun(&EqualizerUi::on_listbox_sort));

  connections.emplace_back(settings->signal_changed("split-channels").connect([&](auto key) {
    for (auto c : connections_bands) {
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
  for (auto c : connections_bands) {
    c.disconnect();
  }

  util::debug(name + " ui destroyed");
}

void EqualizerUi::on_nbands_changed() {
  for (auto c : connections_bands) {
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
  for (auto c : bands_grid->get_children()) {
    bands_grid->remove(*c);

    delete c;
  }

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  for (int n = 0; n < nbands; n++) {
    auto B = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/equalizer_band.glade");

    Gtk::Grid* band_grid;
    Gtk::ComboBoxText* band_type;
    Gtk::ComboBoxText* band_mode;
    Gtk::ComboBoxText* band_slope;
    Gtk::Label* band_width;
    Gtk::Label* band_label;
    Gtk::Button* reset_frequency;
    Gtk::Button* reset_quality;
    Gtk::ToggleButton* band_solo;
    Gtk::ToggleButton* band_mute;
    Gtk::Scale* band_scale;

    B->get_widget("band_grid", band_grid);
    B->get_widget("band_type", band_type);
    B->get_widget("band_mode", band_mode);
    B->get_widget("band_slope", band_slope);
    B->get_widget("band_width", band_width);
    B->get_widget("band_label", band_label);
    B->get_widget("band_solo", band_solo);
    B->get_widget("band_mute", band_mute);
    B->get_widget("band_scale", band_scale);
    B->get_widget("reset_frequency", reset_frequency);
    B->get_widget("reset_quality", reset_quality);

    auto band_gain = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_gain"));
    auto band_frequency = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_frequency"));
    auto band_quality = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_quality"));

    auto update_w = [=]() {
      auto q = band_quality->get_value();

      if (q > 0) {
        auto f = band_frequency->get_value();

        std::ostringstream msg;

        msg.precision(1);
        msg << std::fixed << f / q << " Hz";

        band_width->set_text(msg.str());
      } else {
        band_width->set_text(_("infinity"));
      }
    };

    auto update_band_label = [=]() {
      auto f = band_frequency->get_value();

      std::ostringstream msg;

      if (f > 1000) {
        msg.precision(1);
        msg << std::fixed << f / 1000 << "kHz";
      } else {
        msg.precision(0);
        msg << std::fixed << f << "Hz";
      }

      band_label->set_text(msg.str());
    };

    connections_bands.emplace_back(band_frequency->signal_value_changed().connect(update_w));

    connections_bands.emplace_back(band_frequency->signal_value_changed().connect(update_band_label));

    connections_bands.emplace_back(band_quality->signal_value_changed().connect(update_w));

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
  for (auto c : bands_grid_left->get_children()) {
    bands_grid_left->remove(*c);

    delete c;
  }

  for (auto c : bands_grid_right->get_children()) {
    bands_grid_right->remove(*c);

    delete c;
  }

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  for (int n = 0; n < nbands; n++) {
    auto B = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/equalizer_band.glade");

    Gtk::Grid* band_grid;
    Gtk::ComboBoxText* band_type;
    Gtk::ComboBoxText* band_mode;
    Gtk::ComboBoxText* band_slope;
    Gtk::Label* band_width;
    Gtk::Label* band_label;
    Gtk::Button* reset_frequency;
    Gtk::Button* reset_quality;
    Gtk::ToggleButton* band_solo;
    Gtk::ToggleButton* band_mute;
    Gtk::Scale* band_scale;

    B->get_widget("band_grid", band_grid);
    B->get_widget("band_type", band_type);
    B->get_widget("band_mode", band_mode);
    B->get_widget("band_slope", band_slope);
    B->get_widget("band_width", band_width);
    B->get_widget("band_label", band_label);
    B->get_widget("band_solo", band_solo);
    B->get_widget("band_mute", band_mute);
    B->get_widget("band_scale", band_scale);
    B->get_widget("reset_frequency", reset_frequency);
    B->get_widget("reset_quality", reset_quality);

    auto band_gain = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_gain"));
    auto band_frequency = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_frequency"));
    auto band_quality = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_quality"));

    auto update_w = [=]() {
      auto q = band_quality->get_value();

      if (q > 0) {
        auto f = band_frequency->get_value();

        std::ostringstream msg;

        msg.precision(1);
        msg << std::fixed << f / q << " Hz";

        band_width->set_text(msg.str());
      } else {
        band_width->set_text(_("infinity"));
      }
    };

    auto update_band_label = [=]() {
      auto f = band_frequency->get_value();

      std::ostringstream msg;

      if (f > 1000) {
        msg.precision(1);
        msg << std::fixed << f / 1000 << "kHz";
      } else {
        msg.precision(0);
        msg << std::fixed << f << "Hz";
      }

      band_label->set_text(msg.str());
    };

    connections_bands.emplace_back(band_frequency->signal_value_changed().connect(update_w));

    connections_bands.emplace_back(band_frequency->signal_value_changed().connect(update_band_label));

    connections_bands.emplace_back(band_quality->signal_value_changed().connect(update_w));

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
  for (int n = 0; n < 30; n++) {
    // left channel

    settings_left->reset(std::string("band" + std::to_string(n) + "-gain"));

    // right channel

    settings_right->reset(std::string("band" + std::to_string(n) + "-gain"));
  }
}

void EqualizerUi::on_calculate_frequencies() {
  const double min_freq = 20.0;
  const double max_freq = 20000.0;
  double freq0;
  double freq1;
  double step;

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
  gsize dsize;
  std::stringstream ss;
  boost::property_tree::ptree root;

  auto bytes = Gio::Resource::lookup_data_global(presets_path + file_name);

  auto rdata = static_cast<const char*>(bytes->get_data(dsize));

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
    double q = 0;

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

  for (auto c : children) {
    presets_listbox->remove(*c);
  }

  auto names = Gio::Resource::enumerate_children_global(presets_path);

  for (const auto& file_name : names) {
    auto name = file_name.substr(0, file_name.find('.'));

    auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/equalizer_preset_row.glade");

    Gtk::ListBoxRow* row;
    Gtk::Button* apply_btn;
    Gtk::Label* label;

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

    for (int n = 0; n < 30; n++) {
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
