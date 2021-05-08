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

#include "spectrum_settings_ui.hpp"

namespace {

auto spectrum_type_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "Bars") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "Lines") == 0) {
    g_value_set_int(value, 1);
  }

  return 1;
}

auto int_to_spectrum_type_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  const auto v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("Bars");
  }

  return g_variant_new_string("Lines");
}

}  // namespace

SpectrumSettingsUi::SpectrumSettingsUi(BaseObjectType* cobject,
                                       const Glib::RefPtr<Gtk::Builder>& builder,
                                       Application* application)
    : Gtk::Box(cobject), settings(Gio::Settings::create("com.github.wwmm.easyeffects.spectrum")), app(application) {
  // loading glade widgets

  show = builder->get_widget<Gtk::Switch>("show");
  fill = builder->get_widget<Gtk::Switch>("fill");
  show_bar_border = builder->get_widget<Gtk::Switch>("show_bar_border");

  spectrum_color_button = builder->get_widget<Gtk::ColorButton>("spectrum_color_button");
  axis_color_button = builder->get_widget<Gtk::ColorButton>("axis_color_button");

  spectrum_type = builder->get_widget<Gtk::ComboBoxText>("spectrum_type");

  n_points = builder->get_widget<Gtk::SpinButton>("n_points");
  height = builder->get_widget<Gtk::SpinButton>("height");
  line_width = builder->get_widget<Gtk::SpinButton>("line_width");
  minimum_frequency = builder->get_widget<Gtk::SpinButton>("minimum_frequency");
  maximum_frequency = builder->get_widget<Gtk::SpinButton>("maximum_frequency");

  // signals connection

  connections.emplace_back(settings->signal_changed("color").connect([&](auto key) {
    Glib::Variant<std::vector<double>> v;

    settings->get_value("color", v);

    auto rgba = v.get();

    Gdk::RGBA color;

    color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);

    spectrum_color_button->set_rgba(color);
  }));

  spectrum_color_button->signal_color_set().connect([&]() {
    auto spectrum_color = spectrum_color_button->get_rgba();

    auto v = Glib::Variant<std::vector<double>>::create(std::vector<double>{
        spectrum_color.get_red(), spectrum_color.get_green(), spectrum_color.get_blue(), spectrum_color.get_alpha()});

    settings->set_value("color", v);
  });

  axis_color_button->signal_color_set().connect([&]() {
    auto axis_color = axis_color_button->get_rgba();

    auto v = Glib::Variant<std::vector<double>>::create(std::vector<double>{
        axis_color.get_red(), axis_color.get_green(), axis_color.get_blue(), axis_color.get_alpha()});

    settings->set_value("color-axis-labels", v);
  });

  minimum_frequency->signal_output().connect([&, this]() { return parse_spinbutton_output(minimum_frequency, "Hz"); },
                                             true);
  minimum_frequency->signal_input().connect(
      [&, this](double& new_value) { return parse_spinbutton_input(minimum_frequency, new_value); }, true);

  maximum_frequency->signal_output().connect([&, this]() { return parse_spinbutton_output(maximum_frequency, "Hz"); },
                                             true);
  maximum_frequency->signal_input().connect(
      [&, this](double& new_value) { return parse_spinbutton_input(maximum_frequency, new_value); }, true);

  settings->bind("show", show, "active");
  settings->bind("fill", fill, "active");
  settings->bind("show-bar-border", show_bar_border, "active");
  settings->bind("n-points", n_points->get_adjustment().get(), "value");
  settings->bind("height", height->get_adjustment().get(), "value");
  settings->bind("line-width", line_width->get_adjustment().get(), "value");
  settings->bind("minimum-frequency", minimum_frequency->get_adjustment().get(), "value");
  settings->bind("maximum-frequency", maximum_frequency->get_adjustment().get(), "value");

  g_settings_bind_with_mapping(settings->gobj(), "type", spectrum_type->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               spectrum_type_enum_to_int, int_to_spectrum_type_enum, nullptr, nullptr);
}

SpectrumSettingsUi::~SpectrumSettingsUi() {
  for (auto& c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

void SpectrumSettingsUi::add_to_stack(Gtk::Stack* stack, Application* app) {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/spectrum_settings.ui");

  auto* ui = Gtk::Builder::get_widget_derived<SpectrumSettingsUi>(builder, "top_box", app);

  stack->add(*ui, "settings_spectrum", _("Spectrum"));
}
