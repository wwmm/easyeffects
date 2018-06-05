#include "maximizer_ui.hpp"

MaximizerUi::MaximizerUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& refBuilder,
                         const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(refBuilder, settings_name) {
    name = "maximizer";

    // loading glade widgets

    builder->get_widget("reduction", reduction);
    builder->get_widget("reduction_label", reduction_label);

    get_object("ceiling", ceiling);
    get_object("release", release);
    get_object("threshold", threshold);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("installed", this, "sensitive", flag);
    settings->bind("ceiling", ceiling, "value", flag);
    settings->bind("release", release, "value", flag);
    settings->bind("threshold", threshold, "value", flag);

    settings->set_boolean("post-messages", true);
}

MaximizerUi::~MaximizerUi() {
    settings->set_boolean("post-messages", false);
}

std::shared_ptr<MaximizerUi> MaximizerUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/maximizer.glade");

    MaximizerUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    return std::shared_ptr<MaximizerUi>(grid);
}

void MaximizerUi::on_new_reduction(double value) {
    reduction->set_value(value);

    reduction_label->set_text(level_to_str(value));
}

void MaximizerUi::reset() {
    settings->reset("state");
    settings->reset("release");
    settings->reset("threshold");
    settings->reset("ceiling");
}
