#include "convolver_ui.hpp"

ConvolverUi::ConvolverUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
    name = "convolver";

    // loading glade widgets

    // builder->get_widget("mode", mode);

    get_object(builder, "input_gain", input_gain);
    get_object(builder, "output_gain", output_gain);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("installed", this, "sensitive", flag);
    settings->bind("input-gain", input_gain.get(), "value", flag);
    settings->bind("output-gain", output_gain.get(), "value", flag);

    // g_settings_bind_with_mapping(settings->gobj(), "mode", mode->gobj(),
    //                              "active", G_SETTINGS_BIND_DEFAULT,
    //                              convolver_enum_to_int,
    //                              int_to_convolver_enum, nullptr, nullptr);

    settings->set_boolean("post-messages", true);
}

ConvolverUi::~ConvolverUi() {
    settings->set_boolean("post-messages", false);

    util::debug(name + " ui destroyed");
}

void ConvolverUi::reset() {
    settings->reset("state");
    settings->reset("input-gain");
    settings->reset("output-gain");
}
