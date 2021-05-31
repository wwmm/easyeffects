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

#include "echo_canceller_ui.hpp"

EchoCancellerUi::EchoCancellerUi(BaseObjectType* cobject,
                                 const Glib::RefPtr<Gtk::Builder>& builder,
                                 const std::string& schema,
                                 const std::string& schema_path)
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = plugin_name::echo_canceller;

  // loading builder widgets

  input_gain = builder->get_widget<Gtk::Scale>("input_gain");
  output_gain = builder->get_widget<Gtk::Scale>("output_gain");

  // fcut = builder->get_widget<Gtk::SpinButton>("fcut");
  // feed = builder->get_widget<Gtk::SpinButton>("feed");

  // gsettings bindings

  settings->bind("input-gain", input_gain->get_adjustment().get(), "value");
  settings->bind("output-gain", output_gain->get_adjustment().get(), "value");
  // settings->bind("fcut", fcut->get_adjustment().get(), "value");
  // settings->bind("feed", feed->get_adjustment().get(), "value");
}

EchoCancellerUi::~EchoCancellerUi() {
  util::debug(name + " ui destroyed");
}

auto EchoCancellerUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> EchoCancellerUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/echo_canceller.ui");

  auto* ui = Gtk::Builder::get_widget_derived<EchoCancellerUi>(
      builder, "top_box", "com.github.wwmm.easyeffects.echocanceller", schema_path + "echocanceller/");

  auto stack_page = stack->add(*ui, plugin_name::echo_canceller);

  return ui;
}

void EchoCancellerUi::reset() {
  bypass->set_active(false);

  settings->reset("input-gain");

  settings->reset("output-gain");

  // settings->reset("fcut");

  // settings->reset("feed");
}
