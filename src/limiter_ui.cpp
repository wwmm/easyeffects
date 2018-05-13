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
    builder->get_widget("asc", asc);

    get_object("input_gain", input_gain);
    get_object("limit", limit);
    get_object("lookahead", lookahead);
    get_object("release", release);
    get_object("oversampling", oversampling);
    get_object("asc_level", asc_level);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;
    auto flag_get = Gio::SettingsBindFlags::SETTINGS_BIND_GET;

    settings->bind("state", limiter_enable, "active", flag);
    settings->bind("state", controls, "sensitive", flag_get);
    settings->bind("state", img_state, "visible", flag_get);
    settings->bind("input-gain", input_gain, "value", flag);
    settings->bind("limit", limit, "value", flag);
    settings->bind("lookahead", lookahead, "value", flag);
    settings->bind("release", release, "value", flag);
    settings->bind("oversampling", oversampling, "value", flag);
    settings->bind("asc", asc, "active", flag);
    settings->bind("asc-level", asc_level, "value", flag);
}

LimiterUi::~LimiterUi() {}

LimiterUi* LimiterUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/limiter.glade");

    LimiterUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    return grid;
}
