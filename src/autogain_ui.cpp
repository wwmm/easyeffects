#include "autogain_ui.hpp"

AutoGainUi::AutoGainUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
    name = "autogain";

    // loading glade widgets

    get_object(builder, "target", target);
    get_object(builder, "window", window);
    get_object(builder, "input_gain", input_gain);
    get_object(builder, "output_gain", output_gain);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("installed", this, "sensitive", flag);
    settings->bind("input-gain", input_gain.get(), "value", flag);
    settings->bind("output-gain", output_gain.get(), "value", flag);
    settings->bind("target", target.get(), "value", flag);
    settings->bind("window", window.get(), "value", flag);

    settings->set_boolean("post-messages", true);
}

AutoGainUi::~AutoGainUi() {
    settings->set_boolean("post-messages", false);

    util::debug(name + " ui destroyed");
}

void AutoGainUi::reset() {
    settings->reset("state");
    settings->reset("input-gain");
    settings->reset("output-gain");
    settings->reset("target");
    settings->reset("window");
}
