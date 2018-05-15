#include "plugin_ui_base.hpp"

PluginUiBase::PluginUiBase(const Glib::RefPtr<Gtk::Builder>& refBuilder,
                           std::string settings_name)
    : builder(refBuilder), settings(Gio::Settings::create(settings_name)) {
    builder->get_widget("listbox_control", listbox_control);
    builder->get_widget("controls", controls);
    builder->get_widget("img_state", img_state);

    builder->get_widget("input_level_left", input_level_left);
    builder->get_widget("input_level_right", input_level_right);
    builder->get_widget("output_level_left", output_level_left);
    builder->get_widget("output_level_right", output_level_right);
}

PluginUiBase::~PluginUiBase() {}

void PluginUiBase::on_new_input_level(const std::array<double, 2>& peak) {
    auto left = peak[0];
    auto right = peak[1];

    if (left >= -99) {
        input_level_left->set_value(pow(10, left / 10));
    } else {
        input_level_left->set_value(0);
    }

    if (right >= -99) {
        input_level_right->set_value(pow(10, left / 10));
    } else {
        input_level_right->set_value(0);
    }
}

void PluginUiBase::on_new_output_level(const std::array<double, 2>& peak) {
    auto left = peak[0];
    auto right = peak[1];

    if (left >= -99) {
        output_level_left->set_value(pow(10, left / 10));
    } else {
        output_level_left->set_value(0);
    }

    if (right >= -99) {
        output_level_right->set_value(pow(10, left / 10));
    } else {
        output_level_right->set_value(0);
    }
}
