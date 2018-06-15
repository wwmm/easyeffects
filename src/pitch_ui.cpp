#include "pitch_ui.hpp"

PitchUi::PitchUi(BaseObjectType* cobject,
                 const Glib::RefPtr<Gtk::Builder>& builder,
                 const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
    name = "pitch";

    // loading glade widgets

    builder->get_widget("faster", faster);
    builder->get_widget("formant_preserving", formant_preserving);

    get_object(builder, "cents", cents);
    get_object(builder, "crispness", crispness);
    get_object(builder, "semitones", semitones);
    get_object(builder, "octaves", octaves);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("installed", this, "sensitive", flag);
    settings->bind("cents", cents.get(), "value", flag);
    settings->bind("crispness", crispness.get(), "value", flag);
    settings->bind("semitones", semitones.get(), "value", flag);
    settings->bind("octaves", octaves.get(), "value", flag);
    settings->bind("faster", faster, "active", flag);
    settings->bind("formant-preserving", formant_preserving, "active", flag);

    settings->set_boolean("post-messages", true);
}

PitchUi::~PitchUi() {
    settings->set_boolean("post-messages", false);

    util::debug(name + " ui destroyed");
}

void PitchUi::reset() {
    settings->reset("state");
    settings->reset("cents");
    settings->reset("crispness");
    settings->reset("semitones");
    settings->reset("octaves");
    settings->reset("faster");
    settings->reset("formant_preserving");
}
