#include "pitch_ui.hpp"

PitchUi::PitchUi(BaseObjectType* cobject,
                 const Glib::RefPtr<Gtk::Builder>& refBuilder,
                 const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(refBuilder, settings_name) {
    name = "pitch";

    // loading glade widgets

    builder->get_widget("faster", faster);
    builder->get_widget("formant_preserving", formant_preserving);

    get_object("cents", cents);
    get_object("crispness", crispness);
    get_object("semitones", semitones);
    get_object("octaves", octaves);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("installed", this, "sensitive", flag);
    settings->bind("cents", cents, "value", flag);
    settings->bind("crispness", crispness, "value", flag);
    settings->bind("semitones", semitones, "value", flag);
    settings->bind("octaves", octaves, "value", flag);
    settings->bind("faster", faster, "active", flag);
    settings->bind("formant-preserving", formant_preserving, "active", flag);

    settings->set_boolean("post-messages", true);
}

PitchUi::~PitchUi() {
    settings->set_boolean("post-messages", false);
}

std::shared_ptr<PitchUi> PitchUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/pitch.glade");

    PitchUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    return std::shared_ptr<PitchUi>(grid);
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
