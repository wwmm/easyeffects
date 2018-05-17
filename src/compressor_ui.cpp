#include "compressor_ui.hpp"

CompressorUi::CompressorUi(BaseObjectType* cobject,
                           const Glib::RefPtr<Gtk::Builder>& refBuilder,
                           std::string settings_name)
    : Gtk::Grid(cobject), PluginUiBase(refBuilder, settings_name) {
    name = "compressor";

    // loading glade widgets

    builder->get_widget("compression", compression);
    builder->get_widget("compression_label", compression_label);

    get_object("attack", attack);
    get_object("knee", knee);
    get_object("makeup", makeup);
    get_object("mix", mix);
    get_object("ratio", ratio);
    get_object("release", release);
    get_object("threshold", threshold);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("attack", attack, "value", flag);
    settings->bind("knee", knee, "value", flag);
    settings->bind("makeup", makeup, "value", flag);
    settings->bind("mix", mix, "value", flag);
    settings->bind("ratio", ratio, "value", flag);
    settings->bind("release", release, "value", flag);
    settings->bind("threshold", threshold, "value", flag);

    settings->set_boolean("post-messages", true);
}

CompressorUi::~CompressorUi() {
    settings->set_boolean("post-messages", false);
}

std::shared_ptr<CompressorUi> CompressorUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/compressor.glade");

    CompressorUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    return std::shared_ptr<CompressorUi>(grid);
}

void CompressorUi::on_new_compression(double value) {
    compression->set_value(1 - value);

    compression_label->set_text(level_to_str(util::linear_to_db(value)));
}

void CompressorUi::reset() {
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
