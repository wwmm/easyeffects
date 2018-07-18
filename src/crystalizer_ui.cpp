#include "crystalizer_ui.hpp"

CrystalizerUi::CrystalizerUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& builder,
                             const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
    name = "crystalizer";

    // loading glade widgets

    get_object(builder, "intensity", intensity);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("installed", this, "sensitive", flag);
    settings->bind("intensity", intensity.get(), "value", flag);

    settings->set_boolean("post-messages", true);
}

CrystalizerUi::~CrystalizerUi() {
    settings->set_boolean("post-messages", false);

    util::debug(name + " ui destroyed");
}

void CrystalizerUi::reset() {
    settings->reset("state");
    settings->reset("intensity");
}
