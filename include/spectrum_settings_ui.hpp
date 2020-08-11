#ifndef SPECTRUM_SETTINGS_UI_HPP
#define SPECTRUM_SETTINGS_UI_HPP

#include <giomm/settings.h>
#include <glibmm/i18n.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/builder.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include <gtkmm/stack.h>
#include <gtkmm/switch.h>
#include "application.hpp"

class SpectrumSettingsUi : public Gtk::Grid {
 public:
  SpectrumSettingsUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Application* application);
  SpectrumSettingsUi(const SpectrumSettingsUi&) = delete;
  auto operator=(const SpectrumSettingsUi&) -> SpectrumSettingsUi& = delete;
  SpectrumSettingsUi(const SpectrumSettingsUi&&) = delete;
  auto operator=(const SpectrumSettingsUi &&) -> SpectrumSettingsUi& = delete;
  ~SpectrumSettingsUi() override;

  static void add_to_stack(Gtk::Stack* stack, Application* app);

 private:
  std::string log_tag = "spectrum_settings_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Application* app = nullptr;

  Gtk::Switch *show = nullptr, *use_custom_color = nullptr, *fill = nullptr, *show_bar_border = nullptr,
              *use_gradient = nullptr;

  Gtk::ColorButton *spectrum_color_button = nullptr, *gradient_color_button = nullptr, *axis_color_button = nullptr;

  Gtk::ComboBoxText* spectrum_type = nullptr;

  Glib::RefPtr<Gtk::Adjustment> n_points, height, scale, exponent, sampling_freq, line_width, minimum_frequency,
      maximum_frequency;

  std::vector<sigc::connection> connections;

  static void get_object(const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& name,
                         Glib::RefPtr<Gtk::Adjustment>& object) {
    object = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object(name));
  }

  auto on_show_spectrum(bool state) -> bool;

  void on_spectrum_sampling_freq_set();

  auto on_use_custom_color(bool state) -> bool;
};

#endif
