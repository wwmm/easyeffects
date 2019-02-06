#include "equalizer_ui.hpp"
#include <glibmm/i18n.h>
#include <gtkmm/label.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace {

gboolean bandtype_enum_to_int(GValue* value,
                              GVariant* variant,
                              gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("Off")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("Bell")) {
    g_value_set_int(value, 1);
  } else if (v == std::string("Hi-pass")) {
    g_value_set_int(value, 2);
  } else if (v == std::string("Hi-shelf")) {
    g_value_set_int(value, 3);
  } else if (v == std::string("Lo-pass")) {
    g_value_set_int(value, 4);
  } else if (v == std::string("Lo-shelf")) {
    g_value_set_int(value, 5);
  } else if (v == std::string("Notch")) {
    g_value_set_int(value, 6);
  } else if (v == std::string("Resonance")) {
    g_value_set_int(value, 7);
  }

  return true;
}

GVariant* int_to_bandtype_enum(const GValue* value,
                               const GVariantType* expected_type,
                               gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("Off");
  } else if (v == 1) {
    return g_variant_new_string("Bell");
  } else if (v == 2) {
    return g_variant_new_string("Hi-pass");
  } else if (v == 3) {
    return g_variant_new_string("Hi-shelf");
  } else if (v == 4) {
    return g_variant_new_string("Lo-pass");
  } else if (v == 5) {
    return g_variant_new_string("Lo-shelf");
  } else if (v == 6) {
    return g_variant_new_string("Notch");
  } else {
    return g_variant_new_string("Resonance");
  }
}

gboolean mode_enum_to_int(GValue* value,
                          GVariant* variant,
                          gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("IIR")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("FIR")) {
    g_value_set_int(value, 1);
  } else if (v == std::string("FFT")) {
    g_value_set_int(value, 2);
  }

  return true;
}

GVariant* int_to_mode_enum(const GValue* value,
                           const GVariantType* expected_type,
                           gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("IIR");
  } else if (v == 1) {
    return g_variant_new_string("FIR");
  } else {
    return g_variant_new_string("FFT");
  }
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
  builder->get_widget("presets_menu_button", presets_menu_button);
  builder->get_widget("presets_scrolled_window", presets_scrolled_window);
  builder->get_widget("split_channels", split_channels);
  builder->get_widget("stack", stack);
  builder->get_widget("stack_switcher", stack_switcher);
  builder->get_widget("mode", mode);

  get_object(builder, "nbands", nbands);
  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);

  // signals connections

  nbands->signal_value_changed().connect(
      sigc::mem_fun(*this, &EqualizerUi::on_nbands_changed));

  reset_eq->signal_clicked().connect(sigc::mem_fun(*this, &EqualizerUi::reset));

  flat_response->signal_clicked().connect(
      sigc::mem_fun(*this, &EqualizerUi::on_flat_response));

  calculate_freqs->signal_clicked().connect(
      sigc::mem_fun(*this, &EqualizerUi::on_calculate_frequencies));

  presets_menu_button->signal_clicked().connect(
      sigc::mem_fun(*this, &EqualizerUi::on_presets_menu_button_clicked));

  presets_listbox->set_sort_func(
      sigc::mem_fun(*this, &EqualizerUi::on_listbox_sort));

  presets_listbox->signal_row_activated().connect(
      [&](auto row) { load_preset(row->get_name() + ".json"); });

  connections.push_back(
      settings->signal_changed("split-channels").connect([&](auto key) {
        for (auto c : connections_bands) {
          c.disconnect();
        }

        connections_bands.clear();

        stack->set_visible_child("left_channel");

        bool split = settings->get_boolean("split-channels");

        if (split) {
          build_bands(bands_grid_left, settings_left, nbands->get_value());
          build_bands(bands_grid_right, settings_right, nbands->get_value());
        } else {
          build_unified_bands(nbands->get_value());
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

  settings->set_boolean("post-messages", true);

  g_settings_bind_with_mapping(settings->gobj(), "mode", mode->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, mode_enum_to_int,
                               int_to_mode_enum, nullptr, nullptr);

  populate_presets_listbox();
}

EqualizerUi::~EqualizerUi() {
  settings->set_boolean("post-messages", false);

  for (auto c : connections) {
    c.disconnect();
  }

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
    build_bands(bands_grid_left, settings_left, nbands->get_value());
    build_bands(bands_grid_right, settings_right, nbands->get_value());
  } else {
    build_unified_bands(nbands->get_value());
  }
}

void EqualizerUi::build_bands(Gtk::Grid* bands_grid,
                              Glib::RefPtr<Gio::Settings> cfg,
                              const int& nbands) {
  for (auto c : bands_grid->get_children()) {
    bands_grid->remove(*c);

    delete c;
  }

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  for (int n = 0; n < nbands; n++) {
    auto B = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/ui/equalizer_band.glade");

    Gtk::Grid* band_grid;
    Gtk::ComboBoxText* band_t;
    Gtk::Label *band_w, *band_label;
    Gtk::Button *reset_f, *reset_q;

    B->get_widget("band_grid", band_grid);
    B->get_widget("band_t", band_t);
    B->get_widget("band_w", band_w);
    B->get_widget("band_label", band_label);
    B->get_widget("reset_f", reset_f);
    B->get_widget("reset_q", reset_q);

    auto band_g =
        Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_g"));
    auto band_f =
        Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_f"));
    auto band_q =
        Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_q"));

    auto update_w = [=]() {
      auto q = band_q->get_value();

      if (q > 0) {
        auto f = band_f->get_value();

        std::ostringstream msg;

        msg.precision(2);
        msg << std::fixed << f / q << " Hz";

        band_w->set_text(msg.str());
      } else {
        band_w->set_text(_("infinity"));
      }
    };

    auto update_band_label = [=]() {
      auto f = band_f->get_value();

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

    connections_bands.push_back(
        band_f->signal_value_changed().connect(update_w));

    connections_bands.push_back(
        band_f->signal_value_changed().connect(update_band_label));

    connections_bands.push_back(
        band_q->signal_value_changed().connect(update_w));

    cfg->bind(std::string("band" + std::to_string(n) + "-gain"), band_g.get(),
              "value", flag);
    cfg->bind(std::string("band" + std::to_string(n) + "-frequency"),
              band_f.get(), "value", flag);
    cfg->bind(std::string("band" + std::to_string(n) + "-q"), band_q.get(),
              "value", flag);

    g_settings_bind_with_mapping(
        cfg->gobj(), std::string("band" + std::to_string(n) + "-type").c_str(),
        band_t->gobj(), "active", G_SETTINGS_BIND_DEFAULT, bandtype_enum_to_int,
        int_to_bandtype_enum, nullptr, nullptr);

    connections_bands.push_back(reset_f->signal_clicked().connect([=]() {
      cfg->reset(std::string("band" + std::to_string(n) + "-frequency"));
    }));

    connections_bands.push_back(reset_q->signal_clicked().connect(
        [=]() { cfg->reset(std::string("band" + std::to_string(n) + "-q")); }));

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
    auto B = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/ui/equalizer_band.glade");

    Gtk::Grid* band_grid;
    Gtk::ComboBoxText* band_t;
    Gtk::Label *band_w, *band_label;
    Gtk::Button *reset_f, *reset_q;

    B->get_widget("band_grid", band_grid);
    B->get_widget("band_t", band_t);
    B->get_widget("band_w", band_w);
    B->get_widget("band_label", band_label);
    B->get_widget("reset_f", reset_f);
    B->get_widget("reset_q", reset_q);

    auto band_g =
        Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_g"));
    auto band_f =
        Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_f"));
    auto band_q =
        Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_q"));

    auto update_w = [=]() {
      auto q = band_q->get_value();

      if (q > 0) {
        auto f = band_f->get_value();

        std::ostringstream msg;

        msg.precision(2);
        msg << std::fixed << f / q << " Hz";

        band_w->set_text(msg.str());
      } else {
        band_w->set_text(_("infinity"));
      }
    };

    auto update_band_label = [=]() {
      auto f = band_f->get_value();

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

    connections_bands.push_back(
        band_f->signal_value_changed().connect(update_w));

    connections_bands.push_back(
        band_f->signal_value_changed().connect(update_band_label));

    connections_bands.push_back(
        band_q->signal_value_changed().connect(update_w));

    /*right channel
      we need the bindgins below for the right channel equalizer to be updated
      they have to be before the bindings for the left channel.
     */

    connections_bands.push_back(band_g->signal_value_changed().connect([=]() {
      settings_right->set_double(
          std::string("band" + std::to_string(n) + "-gain"),
          band_g->get_value());
    }));

    connections_bands.push_back(band_f->signal_value_changed().connect([=]() {
      settings_right->set_double(
          std::string("band" + std::to_string(n) + "-frequency"),
          band_f->get_value());
    }));

    connections_bands.push_back(band_q->signal_value_changed().connect([=]() {
      settings_right->set_double(std::string("band" + std::to_string(n) + "-q"),
                                 band_q->get_value());
    }));

    connections_bands.push_back(band_t->signal_changed().connect([=]() {
      settings_right->set_enum(
          std::string("band" + std::to_string(n) + "-type"),
          band_t->get_active_row_number());
    }));

    // left channel

    settings_left->bind(std::string("band" + std::to_string(n) + "-gain"),
                        band_g.get(), "value", flag);
    settings_left->bind(std::string("band" + std::to_string(n) + "-frequency"),
                        band_f.get(), "value", flag);
    settings_left->bind(std::string("band" + std::to_string(n) + "-q"),
                        band_q.get(), "value", flag);

    g_settings_bind_with_mapping(
        settings_left->gobj(),
        std::string("band" + std::to_string(n) + "-type").c_str(),
        band_t->gobj(), "active", G_SETTINGS_BIND_DEFAULT, bandtype_enum_to_int,
        int_to_bandtype_enum, nullptr, nullptr);

    connections_bands.push_back(reset_f->signal_clicked().connect([=]() {
      settings_left->reset(
          std::string("band" + std::to_string(n) + "-frequency"));

      settings_right->reset(
          std::string("band" + std::to_string(n) + "-frequency"));
    }));

    connections_bands.push_back(reset_q->signal_clicked().connect([=]() {
      settings_left->reset(std::string("band" + std::to_string(n) + "-q"));

      settings_right->reset(std::string("band" + std::to_string(n) + "-q"));
    }));

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
  double freq0, freq1, step;

  int nbands = settings->get_int("num-bands");

  // code taken from gstreamer equalizer sources: gstiirequalizer.c
  // function: gst_iir_equalizer_compute_frequencies

  step = pow(max_freq / min_freq, 1.0 / nbands);
  freq0 = min_freq;

  for (int n = 0; n < nbands; n++) {
    freq1 = freq0 * step;

    double freq = freq0 + ((freq1 - freq0) / 2.0);
    double width = freq1 - freq0;
    double q = freq / width;

    // std::cout << n << "\t" << freq << "\t" << width << std::endl;

    // left channel

    settings_left->set_double(
        std::string("band" + std::to_string(n) + "-frequency"), freq);

    settings_left->set_double(std::string("band" + std::to_string(n) + "-q"),
                              q);

    // right channel

    settings_right->set_double(
        std::string("band" + std::to_string(n) + "-frequency"), freq);

    settings_right->set_double(std::string("band" + std::to_string(n) + "-q"),
                               q);

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

  settings->set_double("input-gain", root.get<double>("equalizer.input-gain"));

  settings->set_double("output-gain",
                       root.get<double>("equalizer.output-gain"));

  // settings->set_boolean("split-channels", false);

  for (int n = 0; n < nbands; n++) {
    // left channel

    double f =
        root.get<double>("equalizer.band" + std::to_string(n) + ".frequency");
    double w =
        root.get<double>("equalizer.band" + std::to_string(n) + ".width");
    double q = f / w;

    settings_left->set_double(
        std::string("band" + std::to_string(n) + "-gain"),
        root.get<double>("equalizer.band" + std::to_string(n) + ".gain"));

    settings_left->set_double(
        std::string("band" + std::to_string(n) + "-frequency"), f);

    settings_left->set_double(std::string("band" + std::to_string(n) + "-q"),
                              q);

    settings_left->set_string(
        std::string("band" + std::to_string(n) + "-type"),
        root.get<std::string>("equalizer.band" + std::to_string(n) + ".type"));

    // right channel

    settings_right->set_double(
        std::string("band" + std::to_string(n) + "-gain"),
        root.get<double>("equalizer.band" + std::to_string(n) + ".gain"));

    settings_right->set_double(
        std::string("band" + std::to_string(n) + "-frequency"), f);

    settings_right->set_double(std::string("band" + std::to_string(n) + "-q"),
                               q);

    settings_right->set_string(
        std::string("band" + std::to_string(n) + "-type"),
        root.get<std::string>("equalizer.band" + std::to_string(n) + ".type"));
  }
}

int EqualizerUi::on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) {
  auto name1 = row1->get_name();
  auto name2 = row2->get_name();

  std::vector<std::string> names = {name1, name2};

  std::sort(names.begin(), names.end());

  if (name1 == names[0]) {
    return -1;
  } else if (name2 == names[0]) {
    return 1;
  } else {
    return 0;
  }
}

void EqualizerUi::populate_presets_listbox() {
  auto children = presets_listbox->get_children();

  for (auto c : children) {
    presets_listbox->remove(*c);
  }

  auto names = Gio::Resource::enumerate_children_global(presets_path);

  for (auto file_name : names) {
    Gtk::ListBoxRow* row = Gtk::manage(new Gtk::ListBoxRow());
    Gtk::Label* label = Gtk::manage(new Gtk::Label());

    auto name = file_name.substr(0, file_name.find("."));

    row->set_name(name);
    label->set_text(name);
    label->set_halign(Gtk::ALIGN_START);

    row->add(*label);

    presets_listbox->add(*row);
    presets_listbox->show_all();
  }
}

void EqualizerUi::on_presets_menu_button_clicked() {
  int height = get_allocated_height();

  presets_scrolled_window->set_max_content_height(height);
}

void EqualizerUi::reset() {
  settings->reset("state");
  settings->reset("num-bands");
  settings->reset("split-channels");
  settings->reset("input-gain");
  settings->reset("output-gain");

  for (int n = 0; n < 30; n++) {
    // left channel

    settings_left->reset(std::string("band" + std::to_string(n) + "-gain"));
    settings_left->reset(
        std::string("band" + std::to_string(n) + "-frequency"));
    // settings_left->reset(std::string("band" + std::to_string(n) + "-width"));
    settings_left->reset(std::string("band" + std::to_string(n) + "-type"));

    // right channel

    settings_right->reset(std::string("band" + std::to_string(n) + "-gain"));
    settings_right->reset(
        std::string("band" + std::to_string(n) + "-frequency"));
    // settings_right->reset(std::string("band" + std::to_string(n) +
    // "-width"));
    settings_right->reset(std::string("band" + std::to_string(n) + "-type"));
  }
}
