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

#include "delay_ui.hpp"

DelayUi::DelayUi(BaseObjectType* cobject,
                 const Glib::RefPtr<Gtk::Builder>& builder,
                 const std::string& schema,
                 const std::string& schema_path)
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = plugin_name::delay;

  // loading glade widgets

  input_gain = builder->get_widget<Gtk::Scale>("input_gain");
  output_gain = builder->get_widget<Gtk::Scale>("output_gain");
  time_l = builder->get_widget<Gtk::SpinButton>("time_l");
  time_r = builder->get_widget<Gtk::SpinButton>("time_r");

  // gsettings bindings

  settings->bind("input-gain", input_gain->get_adjustment().get(), "value");
  settings->bind("output-gain", output_gain->get_adjustment().get(), "value");
  settings->bind("time-l", time_l->get_adjustment().get(), "value");
  settings->bind("time-r", time_r->get_adjustment().get(), "value");

  prepare_scale(input_gain, "");
  prepare_scale(output_gain, "");

  prepare_spinbutton(time_l, "ms");
  prepare_spinbutton(time_r, "ms");
}

DelayUi::~DelayUi() {
  util::debug(name + " ui destroyed");
}

auto DelayUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> DelayUi* {
  const auto& builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/delay.ui");

  auto* const ui = Gtk::Builder::get_widget_derived<DelayUi>(builder, "top_box", "com.github.wwmm.easyeffects.delay",
                                                       schema_path + "delay/");

  stack->add(*ui, plugin_name::delay);

  return ui;
}

void DelayUi::reset() {
  bypass->set_active(false);

  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("time-l");

  settings->reset("time-r");
}
