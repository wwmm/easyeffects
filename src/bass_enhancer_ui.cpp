#include "bass_enhancer_ui.hpp"

BassEnhancerUi::BassEnhancerUi(BaseObjectType* cobject,
                               const Glib::RefPtr<Gtk::Builder>& refBuilder,
                               std::string settings_name)
    : Gtk::Grid(cobject), PluginUiBase(refBuilder, settings_name) {
    name = "bass_enhancer";

    // loading glade widgets

    builder->get_widget("harmonics_levelbar", harmonics_levelbar);

    get_object("amount", amount);
    get_object("blend", blend);
    get_object("floor", floor);
    get_object("harmonics", harmonics);
    get_object("scope", scope);
    get_object("input_gain", input_gain);
    get_object("output_gain", output_gain);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("amount", amount, "value", flag);
    // settings->bind("knee", knee, "value", flag);
    // settings->bind("makeup", makeup, "value", flag);
    // settings->bind("mix", mix, "value", flag);
    // settings->bind("ratio", ratio, "value", flag);
    // settings->bind("release", release, "value", flag);
    // settings->bind("threshold", threshold, "value", flag);

    settings->set_boolean("post-messages", true);
}

BassEnhancerUi::~BassEnhancerUi() {
    settings->set_boolean("post-messages", false);
}

std::shared_ptr<BassEnhancerUi> BassEnhancerUi::create(
    std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/bass_enhancer.glade");

    BassEnhancerUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    return std::shared_ptr<BassEnhancerUi>(grid);
}

void BassEnhancerUi::on_new_harmonics_level(double value) {
    harmonics_levelbar->set_value(1 - value);

    // compression_label->set_text(level_to_str(util::linear_to_db(value)));
}

void BassEnhancerUi::reset() {
    settings->reset("state");
    settings->reset("detection");
    settings->reset("stereo-link");
    settings->reset("mix");
    settings->reset("attack");
    settings->reset("release");
    settings->reset("threshold");
    settings->reset("ratio");
    settings->reset("knee");
    settings->reset("makeup");
}
