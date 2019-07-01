#include "crystalizer_ui.hpp"
#include <gtkmm/scale.h>
#include <gtkmm/togglebutton.h>

CrystalizerUi::CrystalizerUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& builder,
                             const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "crystalizer";

  // loading glade widgets

  builder->get_widget("bands_grid", bands_grid);
  builder->get_widget("range_before", range_before);
  builder->get_widget("range_after", range_after);
  builder->get_widget("range_before_label", range_before_label);
  builder->get_widget("range_after_label", range_after_label);
  builder->get_widget("aggressive", aggressive);

  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);

  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);
  settings->bind("aggressive", aggressive, "active", flag);

  build_bands(13);
}

CrystalizerUi::~CrystalizerUi() {
  util::debug(name + " ui destroyed");
}

void CrystalizerUi::build_bands(const int& nbands) {
  for (auto c : bands_grid->get_children()) {
    bands_grid->remove(*c);

    delete c;
  }

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  for (int n = 0; n < nbands; n++) {
    auto B = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/crystalizer_band.glade");

    Gtk::Grid* band_grid;
    Gtk::Label* band_label;
    Gtk::ToggleButton *band_mute, *band_bypass;
    Gtk::Scale* band_scale;

    B->get_widget("band_grid", band_grid);
    B->get_widget("band_label", band_label);
    B->get_widget("band_mute", band_mute);
    B->get_widget("band_bypass", band_bypass);
    B->get_widget("band_scale", band_scale);

    auto band_intensity = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(B->get_object("band_intensity"));

    connections.push_back(band_mute->signal_toggled().connect([=]() {
      if (band_mute->get_active()) {
        band_scale->set_sensitive(false);
      } else {
        band_scale->set_sensitive(true);
      }
    }));

    settings->bind(std::string("intensity-band" + std::to_string(n)), band_intensity.get(), "value", flag);
    settings->bind(std::string("mute-band" + std::to_string(n)), band_mute, "active", flag);
    settings->bind(std::string("bypass-band" + std::to_string(n)), band_bypass, "active", flag);

    bands_grid->add(*band_grid);

    switch (n) {
      case 0:
        band_label->set_text("250 Hz");

        break;
      case 1:
        band_label->set_text("750 Hz");

        break;
      case 2:
        band_label->set_text("1.5 kHz");

        break;
      case 3:
        band_label->set_text("2.5 kHz");

        break;
      case 4:
        band_label->set_text("3.5 kHz");

        break;

      case 5:
        band_label->set_text("4.5 kHz");

        break;
      case 6:
        band_label->set_text("5.5 kHz");

        break;
      case 7:
        band_label->set_text("6.5 kHz");

        break;
      case 8:
        band_label->set_text("7.5 kHz");

        break;
      case 9:
        band_label->set_text("8.5 kHz");

        break;
      case 10:
        band_label->set_text("9.5 kHz");

        break;
      case 11:
        band_label->set_text("12.5 kHz");

        break;
      case 12:
        band_label->set_text("17.5 kHz");

        break;
    }
  }

  bands_grid->show_all();
}

void CrystalizerUi::on_new_range_before(double value) {
  range_before->set_value(util::db_to_linear(value));

  range_before_label->set_text(level_to_str(value, 2));
}

void CrystalizerUi::on_new_range_after(double value) {
  range_after->set_value(util::db_to_linear(value));

  range_after_label->set_text(level_to_str(value, 2));
}
