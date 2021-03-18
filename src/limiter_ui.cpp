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

#include "limiter_ui.hpp"

LimiterUi::LimiterUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& builder,
                     const std::string& schema,
                     const std::string& schema_path)
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = plugin_name::limiter;

  // loading builder widgets

  auto_level = builder->get_widget<Gtk::ToggleButton>("auto_level");
  asc = builder->get_widget<Gtk::ToggleButton>("asc");
  input_gain = builder->get_widget<Gtk::Scale>("input_gain");
  output_gain = builder->get_widget<Gtk::Scale>("output_gain");
  asc_level = builder->get_widget<Gtk::SpinButton>("asc_level");
  limit = builder->get_widget<Gtk::SpinButton>("limit");
  lookahead = builder->get_widget<Gtk::SpinButton>("lookahead");
  release = builder->get_widget<Gtk::SpinButton>("release");
  oversampling = builder->get_widget<Gtk::SpinButton>("oversampling");
  attenuation = builder->get_widget<Gtk::LevelBar>("attenuation");
  attenuation_label = builder->get_widget<Gtk::Label>("attenuation_label");

  // gsettings bindings

  settings->bind("input-gain", input_gain->get_adjustment().get(), "value");
  settings->bind("limit", limit->get_adjustment().get(), "value");
  settings->bind("lookahead", lookahead->get_adjustment().get(), "value");
  settings->bind("release", release->get_adjustment().get(), "value");
  settings->bind("oversampling", oversampling->get_adjustment().get(), "value");
  settings->bind("asc-level", asc_level->get_adjustment().get(), "value");
  settings->bind("output-gain", output_gain->get_adjustment().get(), "value");
  settings->bind("auto-level", auto_level, "active");
  settings->bind("asc", asc, "active");

  prepare_spinbutton(limit, "dB");
  prepare_spinbutton(release, "ms");
  prepare_spinbutton(lookahead, "ms");
}

LimiterUi::~LimiterUi() {
  util::debug(name + " ui destroyed");
}

auto LimiterUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> LimiterUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/limiter.ui");

  auto* ui = Gtk::Builder::get_widget_derived<LimiterUi>(builder, "top_box", "com.github.wwmm.pulseeffects.limiter",
                                                         schema_path + "limiter/");

  auto stack_page = stack->add(*ui, plugin_name::limiter);

  return ui;
}

void LimiterUi::reset() {
  bypass->set_active(false);

  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("limit");

  settings->reset("lookahead");

  settings->reset("release");

  settings->reset("auto-level");

  settings->reset("asc");

  settings->reset("asc-level");

  settings->reset("oversampling");
}

void LimiterUi::on_new_attenuation(double value) {
  attenuation->set_value(1.0 - value);

  attenuation_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}
