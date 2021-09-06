/*
 *  Copyright © 2017-2022 Wellington Wallace
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

#include "exciter_ui.hpp"

ExciterUi::ExciterUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& builder,
                     const std::string& schema,
                     const std::string& schema_path)
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = plugin_name::exciter;

  // loading builder widgets

  harmonics_levelbar = builder->get_widget<Gtk::LevelBar>("harmonics_levelbar");
  harmonics_levelbar_label = builder->get_widget<Gtk::Label>("harmonics_levelbar_label");
  ceil_active = builder->get_widget<Gtk::ToggleButton>("ceil_active");
  listen = builder->get_widget<Gtk::ToggleButton>("listen");
  ceil = builder->get_widget<Gtk::SpinButton>("ceil");
  amount = builder->get_widget<Gtk::SpinButton>("amount");
  harmonics = builder->get_widget<Gtk::SpinButton>("harmonics");
  scope = builder->get_widget<Gtk::SpinButton>("scope");
  blend = builder->get_widget<Gtk::Scale>("blend");

  // gsettings bindings

  settings->bind("amount", amount->get_adjustment().get(), "value");
  settings->bind("harmonics", harmonics->get_adjustment().get(), "value");
  settings->bind("scope", scope->get_adjustment().get(), "value");
  settings->bind("ceil", ceil->get_adjustment().get(), "value");
  settings->bind("blend", blend->get_adjustment().get(), "value");
  settings->bind("listen", listen, "active");
  settings->bind("ceil-active", ceil_active, "active");
  settings->bind("ceil-active", ceil, "sensitive", Gio::Settings::BindFlags::GET);

  prepare_scale(blend, "");

  prepare_spinbutton(amount, "dB");

  prepare_spinbutton(scope, "Hz");
  prepare_spinbutton(ceil, "Hz");

  prepare_spinbutton(harmonics, "");

  setup_input_output_gain(builder);
}

ExciterUi::~ExciterUi() {
  util::debug(name + " ui destroyed");
}

auto ExciterUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> ExciterUi* {
  const auto& builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/exciter.ui");

  auto* const ui = Gtk::Builder::get_widget_derived<ExciterUi>(builder, "top_box", "com.github.wwmm.easyeffects.exciter",
                                                         schema_path + "exciter/");

  stack->add(*ui, plugin_name::exciter);

  return ui;
}

void ExciterUi::reset() {
  bypass->set_active(false);

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

void ExciterUi::on_new_harmonics_level(const double& value) {
  harmonics_levelbar->set_value(value);

  harmonics_levelbar_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}
