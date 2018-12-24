#include "equalizer_ui.hpp"
#include <gtkmm/comboboxtext.h>
#include <gtkmm/label.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace {

gboolean bandtype_enum_to_int(GValue* value,
                              GVariant* variant,
                              gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("peak")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("low-shelf")) {
    g_value_set_int(value, 1);
  } else if (v == std::string("high-shelf")) {
    g_value_set_int(value, 2);
  }

  return true;
}

GVariant* int_to_bandtype_enum(const GValue* value,
                               const GVariantType* expected_type,
                               gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("peak");
  } else if (v == 1) {
    return g_variant_new_string("low-shelf");
  } else {
    return g_variant_new_string("high-shelf");
  }
}

}  // namespace

EqualizerUi::EqualizerUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "equalizer";

  // loading glade widgets

  builder->get_widget("bands_grid", bands_grid);
  builder->get_widget("reset_eq", reset_eq);
  builder->get_widget("flat_response", flat_response);
  builder->get_widget("calculate_freqs", calculate_freqs);
  builder->get_widget("presets_listbox", presets_listbox);
  builder->get_widget("presets_menu_button", presets_menu_button);
  builder->get_widget("presets_scrolled_window", presets_scrolled_window);

  get_object(builder, "nbands", nbands);
  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);

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

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("num-bands", nbands.get(), "value", flag);
  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);

  settings->set_boolean("post-messages", true);

  populate_presets_listbox();
}

EqualizerUi::~EqualizerUi() {
  settings->set_boolean("post-messages", false);

  for (auto c : connections) {
    c.disconnect();
  }

  util::debug(name + " ui destroyed");
}

void EqualizerUi::on_nbands_changed() {
  int N = nbands->get_value();

  for (auto c : connections) {
    c.disconnect();
  }

  connections.clear();

  for (auto c : bands_grid->get_children()) {
    bands_grid->remove(*c);

    delete c;
  }

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  for (int n = 0; n < N; n++) {
    auto B = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/ui/equalizer_band.glade");

    Gtk::Grid* band_grid;
    Gtk::ComboBoxText* band_t;
    Gtk::Label *band_q, *band_label;
    Gtk::Button *reset_f, *reset_w;

    B->get_widget("band_grid", band_grid);
    B->get_widget("band_t", band_t);
    B->get_widget("band_q", band_q);
    B->get_widget("band_label", band_label);
    B->get_widget("reset_f", reset_f);
    B->get_widget("reset_w", reset_w);

    auto band_g =
        Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_g"));
    auto band_f =
        Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_f"));
    auto band_w =
        Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_w"));

    auto update_q = [=]() {
      auto w = band_w->get_value();

      if (w > 0) {
        auto f = band_f->get_value();

        std::ostringstream msg;

        msg.precision(2);
        msg << std::fixed << f / w;

        band_q->set_text(msg.str());
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

    connections.push_back(band_f->signal_value_changed().connect(update_q));
    connections.push_back(
        band_f->signal_value_changed().connect(update_band_label));
    connections.push_back(band_w->signal_value_changed().connect(update_q));

    settings->bind(std::string("band" + std::to_string(n) + "-gain"),
                   band_g.get(), "value", flag);
    settings->bind(std::string("band" + std::to_string(n) + "-frequency"),
                   band_f.get(), "value", flag);
    settings->bind(std::string("band" + std::to_string(n) + "-width"),
                   band_w.get(), "value", flag);

    g_settings_bind_with_mapping(
        settings->gobj(),
        std::string("band" + std::to_string(n) + "-type").c_str(),
        band_t->gobj(), "active", G_SETTINGS_BIND_DEFAULT, bandtype_enum_to_int,
        int_to_bandtype_enum, nullptr, nullptr);

    connections.push_back(reset_f->signal_clicked().connect([=]() {
      settings->reset(std::string("band" + std::to_string(n) + "-frequency"));
    }));

    connections.push_back(reset_w->signal_clicked().connect([=]() {
      settings->reset(std::string("band" + std::to_string(n) + "-width"));
    }));

    bands_grid->add(*band_grid);
  }

  bands_grid->show_all();
}

void EqualizerUi::on_flat_response() {
  for (int n = 0; n < 30; n++) {
    settings->reset(std::string("band" + std::to_string(n) + "-gain"));
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

    // std::cout << n << "\t" << freq << "\t" << width << std::endl;

    settings->set_double(std::string("band" + std::to_string(n) + "-frequency"),
                         freq);

    settings->set_double(std::string("band" + std::to_string(n) + "-width"),
                         width);

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

  for (int n = 0; n < nbands; n++) {
    settings->set_double(
        std::string("band" + std::to_string(n) + "-gain"),
        root.get<double>("equalizer.band" + std::to_string(n) + ".gain"));

    settings->set_double(
        std::string("band" + std::to_string(n) + "-frequency"),
        root.get<double>("equalizer.band" + std::to_string(n) + ".frequency"));

    settings->set_double(
        std::string("band" + std::to_string(n) + "-width"),
        root.get<double>("equalizer.band" + std::to_string(n) + ".width"));

    settings->set_string(
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

  for (int n = 0; n < 30; n++) {
    settings->reset(std::string("band" + std::to_string(n) + "-gain"));
    settings->reset(std::string("band" + std::to_string(n) + "-frequency"));
    settings->reset(std::string("band" + std::to_string(n) + "-width"));
    settings->reset(std::string("band" + std::to_string(n) + "-type"));
  }
}
