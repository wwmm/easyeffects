#include "panorama_ui.hpp"

PanoramaUi::PanoramaUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
    name = "panorama";

    // loading glade widgets

    get_object(builder, "position", position);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("installed", this, "sensitive", flag);
    settings->bind("position", position.get(), "value", flag);

    settings->set_boolean("post-messages", true);
}

PanoramaUi::~PanoramaUi() {
    settings->set_boolean("post-messages", false);
}

std::shared_ptr<PanoramaUi> PanoramaUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/panorama.glade");

    PanoramaUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    grid->reference();

    return std::shared_ptr<PanoramaUi>(grid);
}

void PanoramaUi::reset() {
    settings->reset("state");
    settings->reset("position");
}
