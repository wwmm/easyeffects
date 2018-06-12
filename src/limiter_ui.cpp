#include "limiter_ui.hpp"

LimiterUi::LimiterUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& builder,
                     const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
    name = "limiter";

    // loading glade widgets

    builder->get_widget("asc", asc);
    builder->get_widget("autovolume_enable", autovolume_enable);
    builder->get_widget("autovolume_controls", autovolume_controls);
    builder->get_widget("limiter_controls", limiter_controls);
    builder->get_widget("attenuation", attenuation);
    builder->get_widget("attenuation_label", attenuation_label);

    get_object(builder, "input_gain", input_gain);
    get_object(builder, "limit", limit);
    get_object(builder, "lookahead", lookahead);
    get_object(builder, "release", release);
    get_object(builder, "oversampling", oversampling);
    get_object(builder, "asc_level", asc_level);
    get_object(builder, "autovolume_window", autovolume_window);
    get_object(builder, "autovolume_target", autovolume_target);
    get_object(builder, "autovolume_tolerance", autovolume_tolerance);
    get_object(builder, "autovolume_threshold", autovolume_threshold);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;
    auto flag_get = Gio::SettingsBindFlags::SETTINGS_BIND_GET;
    auto flag_invert_boolean =
        Gio::SettingsBindFlags::SETTINGS_BIND_INVERT_BOOLEAN;

    settings->bind("installed", this, "sensitive", flag);

    settings->bind("input-gain", input_gain.get(), "value", flag);
    settings->bind("limit", limit.get(), "value", flag);
    settings->bind("lookahead", lookahead.get(), "value", flag);
    settings->bind("release", release.get(), "value", flag);
    settings->bind("oversampling", oversampling.get(), "value", flag);
    settings->bind("asc", asc, "active", flag);
    settings->bind("asc-level", asc_level.get(), "value", flag);

    settings->bind("autovolume-state", autovolume_enable, "active", flag);
    settings->bind("autovolume-state", autovolume_controls, "sensitive",
                   flag_get);

    settings->bind("autovolume-state", limiter_controls, "sensitive",
                   flag_get | flag_invert_boolean);

    settings->bind("autovolume-window", autovolume_window.get(), "value", flag);
    settings->bind("autovolume-target", autovolume_target.get(), "value", flag);
    settings->bind("autovolume-tolerance", autovolume_tolerance.get(), "value",
                   flag);
    settings->bind("autovolume-threshold", autovolume_threshold.get(), "value",
                   flag);

    connections.push_back(
        settings->signal_changed("autovolume-state").connect([&](auto key) {
            init_autovolume();
        }));

    connections.push_back(
        settings->signal_changed("autovolume-window").connect([&](auto key) {
            auto window = settings->get_double("autovolume-window");

            release->set_value(window);
        }));

    connections.push_back(
        settings->signal_changed("autovolume-target").connect([&](auto key) {
            auto target = settings->get_int("autovolume-target");
            auto tolerance = settings->get_int("autovolume-tolerance");

            limit->set_value(target + tolerance);
        }));

    connections.push_back(
        settings->signal_changed("autovolume-tolerance").connect([&](auto key) {
            auto target = settings->get_int("autovolume-target");
            auto tolerance = settings->get_int("autovolume-tolerance");

            limit->set_value(target + tolerance);
        }));

    init_autovolume();

    settings->set_boolean("post-messages", true);
}

LimiterUi::~LimiterUi() {
    settings->set_boolean("post-messages", false);

    for (auto c : connections) {
        c.disconnect();
    }

    util::warning("destructor");
}

LimiterUi* LimiterUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/limiter.glade");

    LimiterUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    // grid->reference();

    return grid;
}

void LimiterUi::init_autovolume() {
    auto enabled = settings->get_boolean("autovolume-state");

    if (enabled) {
        auto window = settings->get_double("autovolume-window");
        auto target = settings->get_int("autovolume-target");
        auto tolerance = settings->get_int("autovolume-tolerance");

        limit->set_value(target + tolerance);
        release->set_value(window);
        asc->set_active(true);
        asc_level->set_value(1.0);
        lookahead->set_value(10.0);  // 10 ms
    }
}

void LimiterUi::on_new_attenuation(double value) {
    attenuation->set_value(1 - value);

    attenuation_label->set_text(level_to_str(util::linear_to_db(value)));
}

void LimiterUi::reset() {
    settings->reset("state");
    settings->reset("input-gain");
    settings->reset("limit");
    settings->reset("lookahead");
    settings->reset("release");
    settings->reset("oversampling");
    settings->reset("asc");
    settings->reset("asc-level");
    settings->reset("autovolume-state");
    settings->reset("autovolume-window");
    settings->reset("autovolume-target");
    settings->reset("autovolume-tolerance");
    settings->reset("autovolume-threshold");
}
