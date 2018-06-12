#include "crossfeed_ui.hpp"

CrossfeedUi::CrossfeedUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
    name = "crossfeed";

    // loading glade widgets

    builder->get_widget("preset_cmoy", preset_cmoy);
    builder->get_widget("preset_default", preset_default);
    builder->get_widget("preset_jmeier", preset_jmeier);

    get_object(builder, "fcut", fcut);
    get_object(builder, "feed", feed);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("installed", this, "sensitive", flag);
    settings->bind("fcut", fcut.get(), "value", flag);
    settings->bind("feed", feed.get(), "value", flag);

    init_presets_buttons();

    settings->set_boolean("post-messages", true);
}

CrossfeedUi::~CrossfeedUi() {
    settings->set_boolean("post-messages", false);
}

std::shared_ptr<CrossfeedUi> CrossfeedUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/crossfeed.glade");

    CrossfeedUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    grid->reference();

    return std::shared_ptr<CrossfeedUi>(grid);
}

void CrossfeedUi::init_presets_buttons() {
    preset_cmoy->signal_clicked().connect([=]() {
        fcut->set_value(700);
        feed->set_value(6);
    });

    preset_default->signal_clicked().connect([=]() {
        fcut->set_value(700);
        feed->set_value(4.5);
    });

    preset_jmeier->signal_clicked().connect([=]() {
        fcut->set_value(650);
        feed->set_value(9);
    });
}

void CrossfeedUi::reset() {
    settings->reset("state");
    settings->reset("fcut");
    settings->reset("feed");
}
