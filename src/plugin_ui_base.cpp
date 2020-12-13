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
  try {
    global_locale = std::locale("");
  } catch (const std::exception& e) {
    global_locale = std::locale();
  }

  builder->get_widget("enable", enable);
  builder->get_widget("listbox_control", listbox_control);
  builder->get_widget("controls", controls);
  builder->get_widget("plugin_up", plugin_up);
  builder->get_widget("plugin_down", plugin_down);

  builder->get_widget("input_level_left", input_level_left);
  builder->get_widget("input_level_right", input_level_right);
  builder->get_widget("input_level_left_label", input_level_left_label);
  builder->get_widget("input_level_right_label", input_level_right_label);

  builder->get_widget("output_level_left", output_level_left);
  builder->get_widget("output_level_right", output_level_right);
  builder->get_widget("output_level_left_label", output_level_left_label);
  builder->get_widget("output_level_right_label", output_level_right_label);

  // gsettings bindings

  connections.emplace_back(settings->signal_changed("state").connect(
      [=](auto key) { settings->set_boolean("post-messages", settings->get_boolean(key)); }));

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;
  auto flag_get = Gio::SettingsBindFlags::SETTINGS_BIND_GET;

  settings->bind("state", enable, "active", flag);
  settings->bind("state", controls, "sensitive", flag_get);

  settings->set_boolean("post-messages", settings->get_boolean("state"));
}

PluginUiBase::~PluginUiBase() {
  for (auto& c : connections) {
    c.disconnect();
  }

  settings->set_boolean("post-messages", false);
}

auto PluginUiBase::level_to_str(const double& value, const int& places) -> std::string {
  std::ostringstream msg;

  msg.imbue(global_locale);
  msg.precision(places);

  msg << std::fixed << value;

  return msg.str();
}

auto PluginUiBase::level_to_str(const float& value, const int& places) -> std::string {
  std::ostringstream msg;

  msg.imbue(global_locale);
  msg.precision(places);

  msg << std::fixed << value;

  return msg.str();
}

auto PluginUiBase::string_to_float_nolocale(const std::string& value) -> float {
  std::stringstream ss;
  ss.imbue(c_locale);

  float fv = 0.0F;

  ss << value;
  ss >> fv;

  return fv;
}

void PluginUiBase::on_new_input_level(const std::array<double, 2>& peak) {
  update_level(input_level_left, input_level_left_label, input_level_right, input_level_right_label, peak);
}

void PluginUiBase::on_new_output_level(const std::array<double, 2>& peak) {
  update_level(output_level_left, output_level_left_label, output_level_right, output_level_right_label, peak);
}

void PluginUiBase::on_new_input_level_db(const std::array<double, 2>& peak) {
  update_level_db(input_level_left, input_level_left_label, input_level_right, input_level_right_label, peak);
}

void PluginUiBase::on_new_output_level_db(const std::array<double, 2>& peak) {
  update_level_db(output_level_left, output_level_left_label, output_level_right, output_level_right_label, peak);
}
