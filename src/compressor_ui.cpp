#include "compressor_ui.hpp"

CompressorUi::CompressorUi(BaseObjectType* cobject,
                           const Glib::RefPtr<Gtk::Builder>& refBuilder,
                           std::string settings_name)
    : Gtk::Grid(cobject), PluginUiBase(refBuilder, settings_name) {
    name = "compressor";

    // loading glade widgets

    builder->get_widget("compression", compression);
    builder->get_widget("compression_label", compression_label);

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

    settings->bind("input-gain", input_gain, "value", flag);
    settings->bind("limit", limit, "value", flag);
    settings->bind("lookahead", lookahead, "value", flag);
    settings->bind("release", release, "value", flag);
    settings->bind("oversampling", oversampling, "value", flag);
    settings->bind("asc", asc, "active", flag);
    settings->bind("asc-level", asc_level, "value", flag);

    settings->set_boolean("post-messages", true);
}

CompressorUi::~CompressorUi() {
    settings->set_boolean("post-messages", false);

    for (auto c : connections) {
        c.disconnect();
    }
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
