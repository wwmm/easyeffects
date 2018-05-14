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
    builder->get_widget("autovolume_enable", autovolume_enable);
    builder->get_widget("autovolume_controls", autovolume_controls);
    builder->get_widget("limiter_controls", limiter_controls);

    get_object("input_gain", input_gain);
    get_object("limit", limit);
    get_object("lookahead", lookahead);
    get_object("release", release);
    get_object("oversampling", oversampling);
    get_object("asc_level", asc_level);
    get_object("autovolume_window", autovolume_window);
    get_object("autovolume_target", autovolume_target);
    get_object("autovolume_tolerance", autovolume_tolerance);
    get_object("autovolume_threshold", autovolume_threshold);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;
    auto flag_get = Gio::SettingsBindFlags::SETTINGS_BIND_GET;
    auto flag_invert_boolean =
        Gio::SettingsBindFlags::SETTINGS_BIND_INVERT_BOOLEAN;

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

    settings->bind("autovolume-state", autovolume_enable, "active", flag);
    settings->bind("autovolume-state", autovolume_controls, "sensitive",
                   flag_get);

    settings->bind("autovolume-state", limiter_controls, "sensitive",
                   flag_get | flag_invert_boolean);

    settings->bind("autovolume-window", autovolume_window, "value", flag);
    settings->bind("autovolume-target", autovolume_target, "value", flag);
    settings->bind("autovolume-tolerance", autovolume_tolerance, "value", flag);
    settings->bind("autovolume-threshold", autovolume_threshold, "value", flag);
}

LimiterUi::~LimiterUi() {}

LimiterUi* LimiterUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/limiter.glade");

    LimiterUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    return grid;
}
