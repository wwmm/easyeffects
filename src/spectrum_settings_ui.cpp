#include "spectrum_settings_ui.hpp"
#include "util.hpp"

SpectrumSettingsUi::SpectrumSettingsUi(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& builder,
    const Glib::RefPtr<Gio::Settings>& refSettings,
    Application* application)
    : Gtk::Grid(cobject), settings(refSettings), app(application) {
  // loading glade widgets

  builder->get_widget("show_spectrum", show_spectrum);
  builder->get_widget("spectrum_fill", spectrum_fill);
  builder->get_widget("spectrum_color_button", spectrum_color_button);
  builder->get_widget("use_custom_color", use_custom_color);

  get_object(builder, "spectrum_n_points", spectrum_n_points);
  get_object(builder, "spectrum_height", spectrum_height);
  get_object(builder, "spectrum_scale", spectrum_scale);
  get_object(builder, "spectrum_exponent", spectrum_exponent);
  get_object(builder, "spectrum_sampling_freq", spectrum_sampling_freq);

  // signals connection

  connections.push_back(
      settings->signal_changed("spectrum-color").connect([&](auto key) {
        Glib::Variant<std::vector<double>> v;

        settings->get_value("spectrum-color", v);

        auto rgba = v.get();

        Gdk::RGBA spectrum_color;

        spectrum_color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);

        spectrum_color_button->set_rgba(spectrum_color);
      }));

  show_spectrum->signal_state_set().connect(
      sigc::mem_fun(*this, &SpectrumSettingsUi::on_show_spectrum), false);

  spectrum_color_button->signal_color_set().connect([&]() {
    auto spectrum_color = spectrum_color_button->get_rgba();

    auto v = Glib::Variant<std::vector<double>>::create(std::vector<double>{
        spectrum_color.get_red(), spectrum_color.get_green(),
        spectrum_color.get_blue(), spectrum_color.get_alpha()});

    settings->set_value("spectrum-color", v);
  });

  use_custom_color->signal_state_set().connect(
      sigc::mem_fun(*this, &SpectrumSettingsUi::on_use_custom_color), false);

  spectrum_sampling_freq->signal_value_changed().connect(
      sigc::mem_fun(*this, &SpectrumSettingsUi::on_spectrum_sampling_freq_set),
      false);

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("show-spectrum", show_spectrum, "active", flag);

  settings->bind("spectrum-fill", spectrum_fill, "active", flag);
  settings->bind("spectrum-n-points", spectrum_n_points.get(), "value", flag);
  settings->bind("spectrum-height", spectrum_height.get(), "value", flag);
  settings->bind("spectrum-scale", spectrum_scale.get(), "value", flag);
  settings->bind("spectrum-exponent", spectrum_exponent.get(), "value", flag);
  settings->bind("spectrum-sampling-freq", spectrum_sampling_freq.get(),
                 "value", flag);
  settings->bind("use-custom-color", use_custom_color, "active", flag);
  settings->bind("use-custom-color", spectrum_color_button, "sensitive", flag);
}

SpectrumSettingsUi::~SpectrumSettingsUi() {
  for (auto c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

void SpectrumSettingsUi::add_to_stack(Gtk::Stack* stack, Application* app) {
  auto builder = Gtk::Builder::create_from_resource(
      "/com/github/wwmm/pulseeffects/ui/spectrum_settings.glade");

  auto settings = Gio::Settings::create("com.github.wwmm.pulseeffects");

  SpectrumSettingsUi* ui;

  builder->get_widget_derived("widgets_grid", ui, settings, app);

  stack->add(*ui, "settings_spectrum", _("Spectrum"));
}

bool SpectrumSettingsUi::on_show_spectrum(bool state) {
  if (state) {
    app->sie->enable_spectrum();
    app->soe->enable_spectrum();
  } else {
    app->sie->disable_spectrum();
    app->soe->disable_spectrum();
  }

  return false;
}

bool SpectrumSettingsUi::on_use_custom_color(bool state) {
  if (state) {
    Glib::Variant<std::vector<double>> v;

    settings->get_value("spectrum-color", v);

    auto rgba = v.get();

    Gdk::RGBA spectrum_color;

    spectrum_color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);

    spectrum_color_button->set_rgba(spectrum_color);
  }

  return false;
}

void SpectrumSettingsUi::on_spectrum_sampling_freq_set() {
  app->sie->update_spectrum_interval(spectrum_sampling_freq->get_value());
  app->soe->update_spectrum_interval(spectrum_sampling_freq->get_value());
}
