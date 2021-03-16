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

#include "exciter_ui.hpp"

ExciterUi::ExciterUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& builder,
                     const std::string& schema,
                     const std::string& schema_path)
    : Gtk::Grid(cobject), PluginUiBase(builder, schema, schema_path) {
  name = plugin_name::exciter;

  // loading builder widgets

  harmonics_levelbar = builder->get_widget<Gtk::LevelBar>("harmonics_levelbar");
  harmonics_levelbar_label = builder->get_widget<Gtk::Label>("harmonics_levelbar_label");
  ceil_active = builder->get_widget<Gtk::ToggleButton>("ceil_active");
  listen = builder->get_widget<Gtk::ToggleButton>("listen");
  bypass = builder->get_widget<Gtk::ToggleButton>("bypass");
  ceil = builder->get_widget<Gtk::SpinButton>("ceil");
  amount = builder->get_widget<Gtk::SpinButton>("amount");
  harmonics = builder->get_widget<Gtk::SpinButton>("harmonics");
  scope = builder->get_widget<Gtk::SpinButton>("scope");
  blend = builder->get_widget<Gtk::Scale>("blend");
  input_gain = builder->get_widget<Gtk::Scale>("input_gain");
  output_gain = builder->get_widget<Gtk::Scale>("output_gain");

  reset_button = builder->get_widget<Gtk::Button>("reset_button");

  // gsettings bindings

  settings->bind("amount", amount->get_adjustment().get(), "value");
  settings->bind("harmonics", harmonics->get_adjustment().get(), "value");
  settings->bind("scope", scope->get_adjustment().get(), "value");
  settings->bind("ceil", ceil->get_adjustment().get(), "value");
  settings->bind("blend", blend->get_adjustment().get(), "value");
  settings->bind("input-gain", input_gain->get_adjustment().get(), "value");
  settings->bind("output-gain", output_gain->get_adjustment().get(), "value");
  settings->bind("listen", listen, "active");
  settings->bind("bypass", bypass, "active");
  settings->bind("ceil-active", ceil_active, "active");
  settings->bind("ceil-active", ceil, "sensitive", Gio::Settings::BindFlags::GET);

  reset_button->signal_clicked().connect([=, this]() { reset(); });

  amount->signal_output().connect([&, this]() { return parse_spinbutton_output(amount, "dB"); }, true);
  amount->signal_input().connect([&, this](double& new_value) { return parse_spinbutton_input(amount, new_value); },
                                 true);

  scope->signal_output().connect([&, this]() { return parse_spinbutton_output(scope, "dB"); }, true);
  scope->signal_input().connect([&, this](double& new_value) { return parse_spinbutton_input(scope, new_value); },
                                true);

  ceil->signal_output().connect([&, this]() { return parse_spinbutton_output(ceil, "Hz"); }, true);
  ceil->signal_input().connect([&, this](double& new_value) { return parse_spinbutton_input(ceil, new_value); }, true);
}

ExciterUi::~ExciterUi() {
  util::debug(name + " ui destroyed");
}

auto ExciterUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> ExciterUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/exciter.ui");

  auto* ui = Gtk::Builder::get_widget_derived<ExciterUi>(builder, "top_box", "com.github.wwmm.pulseeffects.exciter",
                                                         schema_path + "exciter/");

  auto stack_page = stack->add(*ui, plugin_name::exciter);

  return ui;
}

void ExciterUi::reset() {
  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("amount");

  settings->reset("harmonics");

  settings->reset("scope");

  settings->reset("ceil");

  settings->reset("blend");

  settings->reset("ceil-active");

  settings->reset("listen");
}

void ExciterUi::on_new_harmonics_level(double value) {
  harmonics_levelbar->set_value(value);

  harmonics_levelbar_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}
