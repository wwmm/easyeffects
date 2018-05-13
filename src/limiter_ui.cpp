#include "limiter_ui.hpp"

LimiterUi::LimiterUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& refBuilder,
                     std::string settings_name)
    : Gtk::Grid(cobject),
      builder(refBuilder),
      settings(Gio::Settings::create(settings_name)) {
    // loading glade widgets

    builder->get_widget("listbox_control", listbox_control);
}

LimiterUi::~LimiterUi() {}

LimiterUi* LimiterUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/limiter.glade");

    LimiterUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    return grid;
}
