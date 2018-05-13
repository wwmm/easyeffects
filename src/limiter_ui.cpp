#include "limiter_ui.hpp"

LimiterUi::LimiterUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& refBuilder,
                     std::string settings_name)
    : Gtk::Grid(cobject),
      builder(refBuilder),
      settings(Gio::Settings::create(settings_name)) {
    // loading glade widgets

    builder->get_widget("listbox_control", listbox_control);
    builder->get_widget("limiter_enable", limiter_enable);
    builder->get_widget("controls", controls);
    builder->get_widget("img_state", img_state);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;
    auto flag_get = Gio::SettingsBindFlags::SETTINGS_BIND_GET;

    settings->bind("state", limiter_enable, "active", flag);
    settings->bind("state", controls, "sensitive", flag_get);
    settings->bind("state", img_state, "visible", flag_get);
}

LimiterUi::~LimiterUi() {}

LimiterUi* LimiterUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/limiter.glade");

    LimiterUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    return grid;
}
