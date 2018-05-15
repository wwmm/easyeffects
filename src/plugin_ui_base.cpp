#include "plugin_ui_base.hpp"

PluginUiBase::PluginUiBase(const Glib::RefPtr<Gtk::Builder>& refBuilder,
                           std::string settings_name)
    : builder(refBuilder), settings(Gio::Settings::create(settings_name)) {
    builder->get_widget("enable", enable);
    builder->get_widget("listbox_control", listbox_control);
    builder->get_widget("controls", controls);
    builder->get_widget("img_state", img_state);

    builder->get_widget("input_level_left", input_level_left);
    builder->get_widget("input_level_right", input_level_right);
    builder->get_widget("input_level_left_label", input_level_left_label);
    builder->get_widget("input_level_right_label", input_level_right_label);

    builder->get_widget("output_level_left", output_level_left);
    builder->get_widget("output_level_right", output_level_right);
    builder->get_widget("output_level_left_label", output_level_left_label);
    builder->get_widget("output_level_right_label", output_level_right_label);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;
    auto flag_get = Gio::SettingsBindFlags::SETTINGS_BIND_GET;

    settings->bind("state", enable, "active", flag);
    settings->bind("state", controls, "sensitive", flag_get);
    settings->bind("state", img_state, "visible", flag_get);
}

PluginUiBase::~PluginUiBase() {}

std::string PluginUiBase::level_to_str(double value) {
    std::ostringstream msg;

    msg.precision(0);
    msg << std::fixed << value;

    return msg.str();
}

void PluginUiBase::on_new_input_level(const std::array<double, 2>& peak) {
    update_level(input_level_left, input_level_left_label, input_level_right,
                 input_level_right_label, peak);
}

void PluginUiBase::on_new_output_level(const std::array<double, 2>& peak) {
    update_level(output_level_left, output_level_left_label, output_level_right,
                 output_level_right_label, peak);
}
