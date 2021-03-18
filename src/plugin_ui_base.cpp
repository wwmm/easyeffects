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

#include "plugin_ui_base.hpp"

PluginUiBase::PluginUiBase(const Glib::RefPtr<Gtk::Builder>& builder,
                           const std::string& schema,
                           const std::string& schema_path)
    : settings(Gio::Settings::create(schema, schema_path)) {
  // get widgets that all plguins must have

  bypass = builder->get_widget<Gtk::ToggleButton>("bypass");

  input_level_left = builder->get_widget<Gtk::LevelBar>("input_level_left");
  input_level_right = builder->get_widget<Gtk::LevelBar>("input_level_right");
  input_level_left_label = builder->get_widget<Gtk::Label>("input_level_left_label");
  input_level_right_label = builder->get_widget<Gtk::Label>("input_level_right_label");

  output_level_left = builder->get_widget<Gtk::LevelBar>("output_level_left");
  output_level_right = builder->get_widget<Gtk::LevelBar>("output_level_right");
  output_level_left_label = builder->get_widget<Gtk::Label>("output_level_left_label");
  output_level_right_label = builder->get_widget<Gtk::Label>("output_level_right_label");
}

PluginUiBase::~PluginUiBase() {
  for (auto& c : connections) {
    c.disconnect();
  }
}

auto PluginUiBase::level_to_localized_string(const double& value, const int& places) -> std::string {
  std::ostringstream msg;

  msg.precision(places);

  msg << std::fixed << value;

  return msg.str();
}

auto PluginUiBase::level_to_localized_string(const float& value, const int& places) -> std::string {
  std::ostringstream msg;

  msg.precision(places);

  msg << std::fixed << value;

  return msg.str();
}

auto PluginUiBase::string_to_float(const std::string& value) -> float {
  std::stringstream ss;

  float fv = 0.0F;

  ss << value;
  ss >> fv;

  return fv;
}

void PluginUiBase::on_new_input_level(const float& left, const float& right) {
  update_level(input_level_left, input_level_left_label, input_level_right, input_level_right_label, left, right);
}

void PluginUiBase::on_new_output_level(const float& left, const float& right) {
  update_level(output_level_left, output_level_left_label, output_level_right, output_level_right_label, left, right);
}
