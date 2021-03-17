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

#include "filter_ui.hpp"
#include <cstring>

namespace {

auto filter_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "12dB/oct Lowpass") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "24dB/oct Lowpass") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "36dB/oct Lowpass") == 0) {
    g_value_set_int(value, 2);
  } else if (std::strcmp(v, "12dB/oct Highpass") == 0) {
    g_value_set_int(value, 3);
  } else if (std::strcmp(v, "24dB/oct Highpass") == 0) {
    g_value_set_int(value, 4);
  } else if (std::strcmp(v, "36dB/oct Highpass") == 0) {
    g_value_set_int(value, 5);
  } else if (std::strcmp(v, "6dB/oct Bandpass") == 0) {
    g_value_set_int(value, 6);
  } else if (std::strcmp(v, "12dB/oct Bandpass") == 0) {
    g_value_set_int(value, 7);
  } else if (std::strcmp(v, "18dB/oct Bandpass") == 0) {
    g_value_set_int(value, 8);
  } else if (std::strcmp(v, "6dB/oct Bandreject") == 0) {
    g_value_set_int(value, 9);
  } else if (std::strcmp(v, "12dB/oct Bandreject") == 0) {
    g_value_set_int(value, 10);
  } else if (std::strcmp(v, "18dB/oct Bandreject") == 0) {
    g_value_set_int(value, 11);
  }

  return 1;
}

auto int_to_filter_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  const auto v = g_value_get_int(value);

  switch (v) {
    case 0:
      return g_variant_new_string("12dB/oct Lowpass");

    case 1:
      return g_variant_new_string("24dB/oct Lowpass");

    case 2:
      return g_variant_new_string("36dB/oct Lowpass");

    case 3:
      return g_variant_new_string("12dB/oct Highpass");

    case 4:
      return g_variant_new_string("24dB/oct Highpass");

    case 5:
      return g_variant_new_string("36dB/oct Highpass");

    case 6:
      return g_variant_new_string("6dB/oct Bandpass");

    case 7:
      return g_variant_new_string("12dB/oct Bandpass");

    case 8:
      return g_variant_new_string("18dB/oct Bandpass");

    case 9:
      return g_variant_new_string("6dB/oct Bandreject");

    case 10:
      return g_variant_new_string("12dB/oct Bandreject");

    case 11:
      return g_variant_new_string("18dB/oct Bandreject");

    default:
      return g_variant_new_string("12dB/oct Lowpass");
  }
}

}  // namespace

FilterUi::FilterUi(BaseObjectType* cobject,
                   const Glib::RefPtr<Gtk::Builder>& builder,
                   const std::string& schema,
                   const std::string& schema_path)
    : Gtk::Grid(cobject), PluginUiBase(builder, schema, schema_path) {
  name = plugin_name::filter;

  // loading builder widgets

  bypass = builder->get_widget<Gtk::ToggleButton>("bypass");
  input_gain = builder->get_widget<Gtk::Scale>("input_gain");
  output_gain = builder->get_widget<Gtk::Scale>("output_gain");
  frequency = builder->get_widget<Gtk::SpinButton>("frequency");
  resonance = builder->get_widget<Gtk::SpinButton>("resonance");
  inertia = builder->get_widget<Gtk::SpinButton>("inertia");
  mode = builder->get_widget<Gtk::ComboBoxText>("mode");

  reset_button = builder->get_widget<Gtk::Button>("reset_button");

  // gsettings bindings

  settings->bind("bypass", bypass, "active");
  settings->bind("input-gain", input_gain->get_adjustment().get(), "value");
  settings->bind("output-gain", output_gain->get_adjustment().get(), "value");
  settings->bind("frequency", frequency->get_adjustment().get(), "value");
  settings->bind("resonance", resonance->get_adjustment().get(), "value");
  settings->bind("inertia", inertia->get_adjustment().get(), "value");

  g_settings_bind_with_mapping(settings->gobj(), "mode", mode->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               filter_enum_to_int, int_to_filter_enum, nullptr, nullptr);

  reset_button->signal_clicked().connect([this]() { reset(); });

  frequency->signal_output().connect([&, this]() { return parse_spinbutton_output(frequency, "Hz"); }, true);
  frequency->signal_input().connect(
      [&, this](double& new_value) { return parse_spinbutton_input(frequency, new_value); }, true);

  resonance->signal_output().connect([&, this]() { return parse_spinbutton_output(resonance, "dB"); }, true);
  resonance->signal_input().connect(
      [&, this](double& new_value) { return parse_spinbutton_input(resonance, new_value); }, true);

  inertia->signal_output().connect([&, this]() { return parse_spinbutton_output(inertia, "ms"); }, true);
  inertia->signal_input().connect([&, this](double& new_value) { return parse_spinbutton_input(inertia, new_value); },
                                  true);

  settings->set_boolean("bypass", false);
}

FilterUi::~FilterUi() {
  util::debug(name + " ui destroyed");
}

auto FilterUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> FilterUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/filter.ui");

  auto* ui = Gtk::Builder::get_widget_derived<FilterUi>(builder, "top_box", "com.github.wwmm.pulseeffects.filter",
                                                        schema_path + "filter/");

  auto stack_page = stack->add(*ui, plugin_name::filter);

  return ui;
}

void FilterUi::reset() {
  settings->reset("bypass");

  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("frequency");

  settings->reset("resonance");

  settings->reset("mode");

  settings->reset("inertia");
}
