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
    : Gtk::Box(cobject), settings(Gio::Settings::create("com.github.wwmm.pulseeffects.spectrum")), app(application) {
  // loading glade widgets

  show = builder->get_widget<Gtk::Switch>("show");
  fill = builder->get_widget<Gtk::Switch>("fill");
  show_bar_border = builder->get_widget<Gtk::Switch>("show_bar_border");
  use_custom_color = builder->get_widget<Gtk::Switch>("use_custom_color");
  use_gradient = builder->get_widget<Gtk::Switch>("use_gradient");

  spectrum_color_button = builder->get_widget<Gtk::ColorButton>("spectrum_color_button");
  axis_color_button = builder->get_widget<Gtk::ColorButton>("axis_color_button");
  gradient_color_button = builder->get_widget<Gtk::ColorButton>("gradient_color_button");

  spectrum_type = builder->get_widget<Gtk::ComboBoxText>("spectrum_type");

  spinbutton_n_points = builder->get_widget<Gtk::SpinButton>("spinbutton_n_points");
  spinbutton_height = builder->get_widget<Gtk::SpinButton>("spinbutton_height");
  spinbutton_line_width = builder->get_widget<Gtk::SpinButton>("spinbutton_line_width");
  spinbutton_sampling = builder->get_widget<Gtk::SpinButton>("spinbutton_sampling");
  spinbutton_minimum_frequency = builder->get_widget<Gtk::SpinButton>("spinbutton_minimum_frequency");
  spinbutton_maximum_frequency = builder->get_widget<Gtk::SpinButton>("spinbutton_maximum_frequency");

  n_points = builder->get_object<Gtk::Adjustment>("n_points");
  height = builder->get_object<Gtk::Adjustment>("height");
  sampling_freq = builder->get_object<Gtk::Adjustment>("sampling_freq");
  line_width = builder->get_object<Gtk::Adjustment>("line_width");
  maximum_frequency = builder->get_object<Gtk::Adjustment>("maximum_frequency");
  minimum_frequency = builder->get_object<Gtk::Adjustment>("minimum_frequency");

  // prepare widgets

  prepare_spin_buttons();

  // signals connection

  connections.emplace_back(settings->signal_changed("color").connect([&](auto key) {
    Glib::Variant<std::vector<double>> v;

    settings->get_value("color", v);

    auto rgba = v.get();

    Gdk::RGBA color;

    color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);

    spectrum_color_button->set_rgba(color);
  }));

  connections.emplace_back(settings->signal_changed("gradient-color").connect([&](auto key) {
    Glib::Variant<std::vector<double>> v;

    settings->get_value("gradient-color", v);

    auto rgba = v.get();

    Gdk::RGBA color;

    color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);

    gradient_color_button->set_rgba(color);
  }));

  show->signal_state_set().connect(sigc::mem_fun(*this, &SpectrumSettingsUi::on_show_spectrum), false);

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

  gradient_color_button->signal_color_set().connect([&]() {
    auto color = gradient_color_button->get_rgba();

    auto v = Glib::Variant<std::vector<double>>::create(
        std::vector<double>{color.get_red(), color.get_green(), color.get_blue(), color.get_alpha()});

    settings->set_value("gradient-color", v);
  });

  use_custom_color->signal_state_set().connect(sigc::mem_fun(*this, &SpectrumSettingsUi::on_use_custom_color), false);

  sampling_freq->signal_value_changed().connect(
      sigc::mem_fun(*this, &SpectrumSettingsUi::on_spectrum_sampling_freq_set), false);

  minimum_frequency->signal_value_changed().connect([&]() {
    if (minimum_frequency->get_value() < maximum_frequency->get_value()) {
      // app->sie->min_spectrum_freq = minimum_frequency->get_value();
      // app->soe->min_spectrum_freq = minimum_frequency->get_value();

      // app->sie->init_spectrum();
      // app->soe->init_spectrum();
    }
  });

  maximum_frequency->signal_value_changed().connect([&]() {
    if (maximum_frequency->get_value() > minimum_frequency->get_value()) {
      // app->sie->max_spectrum_freq = maximum_frequency->get_value();
      // app->soe->max_spectrum_freq = maximum_frequency->get_value();

      // app->sie->init_spectrum();
      // app->soe->init_spectrum();
    }
  });

  settings->bind("show", show, "active");

  settings->bind("fill", fill, "active");
  settings->bind("show-bar-border", show_bar_border, "active");
  settings->bind("n-points", n_points.get(), "value");
  settings->bind("height", height.get(), "value");
  settings->bind("sampling-freq", sampling_freq.get(), "value");
  settings->bind("line-width", line_width.get(), "value");
  settings->bind("use-gradient", use_gradient, "active");
  settings->bind("use-custom-color", use_custom_color, "active");
  settings->bind("use-custom-color", spectrum_color_button, "sensitive");
  settings->bind("use-custom-color", gradient_color_button, "sensitive");
  settings->bind("use-custom-color", axis_color_button, "sensitive");
  settings->bind("minimum-frequency", minimum_frequency.get(), "value");
  settings->bind("maximum-frequency", maximum_frequency.get(), "value");

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
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/spectrum_settings.ui");

  auto* ui = Gtk::Builder::get_widget_derived<SpectrumSettingsUi>(builder, "top_box", app);

  stack->add(*ui, "settings_spectrum", _("Spectrum"));
}

void SpectrumSettingsUi::prepare_spin_buttons() {
  spinbutton_n_points->get_last_child()->insert_at_start(*spinbutton_n_points);

  spinbutton_height->get_last_child()->insert_at_start(*spinbutton_height);

  spinbutton_line_width->get_last_child()->insert_at_start(*spinbutton_line_width);

  spinbutton_sampling->get_last_child()->insert_after(*spinbutton_sampling, *spinbutton_sampling->get_first_child());
  spinbutton_sampling->get_last_child()->insert_at_start(*spinbutton_sampling);

  spinbutton_minimum_frequency->get_last_child()->insert_after(*spinbutton_minimum_frequency,
                                                               *spinbutton_minimum_frequency->get_first_child());
  spinbutton_minimum_frequency->get_last_child()->insert_at_start(*spinbutton_minimum_frequency);

  spinbutton_maximum_frequency->get_last_child()->insert_after(*spinbutton_maximum_frequency,
                                                               *spinbutton_maximum_frequency->get_first_child());
  spinbutton_maximum_frequency->get_last_child()->insert_at_start(*spinbutton_maximum_frequency);

  // For some reason the spinbutton does not finish the childs we add to it

  spinbutton_n_points->signal_hide().connect([=, this]() { spinbutton_n_points->get_first_child()->unparent(); });

  spinbutton_height->signal_hide().connect([=, this]() { spinbutton_height->get_first_child()->unparent(); });

  spinbutton_line_width->signal_hide().connect([=, this]() { spinbutton_line_width->get_first_child()->unparent(); });

  spinbutton_sampling->signal_hide().connect([=, this]() {
    spinbutton_sampling->get_first_child()->unparent();

    spinbutton_sampling->get_first_child()->get_next_sibling()->unparent();
  });

  spinbutton_minimum_frequency->signal_hide().connect([=, this]() {
    spinbutton_minimum_frequency->get_first_child()->unparent();

    spinbutton_minimum_frequency->get_first_child()->get_next_sibling()->unparent();
  });

  spinbutton_maximum_frequency->signal_hide().connect([=, this]() {
    spinbutton_maximum_frequency->get_first_child()->unparent();

    spinbutton_maximum_frequency->get_first_child()->get_next_sibling()->unparent();
  });
}

auto SpectrumSettingsUi::on_show_spectrum(bool state) -> bool {
  // if (state) {
  //   app->sie->enable_spectrum();
  //   app->soe->enable_spectrum();
  // } else {
  //   app->sie->disable_spectrum();
  //   app->soe->disable_spectrum();
  // }

  return false;
}

auto SpectrumSettingsUi::on_use_custom_color(bool state) -> bool {
  if (state) {
    Glib::Variant<std::vector<double>> v;

    settings->get_value("color", v);

    auto rgba = v.get();

    Gdk::RGBA color;

    color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);

    spectrum_color_button->set_rgba(color);

    // background color

    settings->get_value("gradient-color", v);

    rgba = v.get();

    color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);

    gradient_color_button->set_rgba(color);
  }

  return false;
}

void SpectrumSettingsUi::on_spectrum_sampling_freq_set() {
  // app->sie->update_spectrum_interval(sampling_freq->get_value());
  // app->soe->update_spectrum_interval(sampling_freq->get_value());
}
