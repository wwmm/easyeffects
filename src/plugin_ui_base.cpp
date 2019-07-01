#include "plugin_ui_base.hpp"

PluginUiBase::PluginUiBase(const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name)
    : settings(Gio::Settings::create(settings_name)) {
  builder->get_widget("enable", enable);
  builder->get_widget("listbox_control", listbox_control);
  builder->get_widget("controls", controls);
  builder->get_widget("img_state", img_state);
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

  connections.push_back(settings->signal_changed("state").connect(
      [=](auto key) { settings->set_boolean("post-messages", settings->get_boolean(key)); }));

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;
  auto flag_get = Gio::SettingsBindFlags::SETTINGS_BIND_GET;

  settings->bind("state", enable, "active", flag);
  settings->bind("state", controls, "sensitive", flag_get);
  settings->bind("state", img_state, "visible", flag_get);

  settings->set_boolean("post-messages", settings->get_boolean("state"));
}

PluginUiBase::~PluginUiBase() {
  for (auto c : connections) {
    c.disconnect();
  }

  settings->set_boolean("post-messages", false);
}

std::string PluginUiBase::level_to_str(const double& value, const int& places) {
  std::ostringstream msg;

  msg.precision(places);
  msg << std::fixed << value;

  return msg.str();
}

void PluginUiBase::on_new_input_level(const std::array<double, 2>& peak) {
  update_level(input_level_left, input_level_left_label, input_level_right, input_level_right_label, peak);

  if (peak[0] > 1 || peak[1] > 1) {
    input_saturated = true;

    img_state->set_from_icon_name("dialog-warning-symbolic", Gtk::ICON_SIZE_BUTTON);
  } else if (peak[0] <= 1 && peak[1] <= 1) {
    if (input_saturated) {
      img_state->set_from_icon_name("emblem-ok-symbolic", Gtk::ICON_SIZE_BUTTON);
    }

    input_saturated = false;
  }
}

void PluginUiBase::on_new_output_level(const std::array<double, 2>& peak) {
  update_level(output_level_left, output_level_left_label, output_level_right, output_level_right_label, peak);
}

void PluginUiBase::on_new_input_level_db(const std::array<double, 2>& peak) {
  update_level_db(input_level_left, input_level_left_label, input_level_right, input_level_right_label, peak);

  if (peak[0] > 0 || peak[1] > 0) {
    input_saturated = true;

    img_state->set_from_icon_name("dialog-warning-symbolic", Gtk::ICON_SIZE_BUTTON);
  } else if (peak[0] <= 0 && peak[1] <= 0) {
    if (input_saturated) {
      img_state->set_from_icon_name("emblem-ok-symbolic", Gtk::ICON_SIZE_BUTTON);
    }

    input_saturated = false;
  }
}

void PluginUiBase::on_new_output_level_db(const std::array<double, 2>& peak) {
  update_level_db(output_level_left, output_level_left_label, output_level_right, output_level_right_label, peak);
}
