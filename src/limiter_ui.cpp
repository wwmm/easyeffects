#include "limiter_ui.hpp"

LimiterUi::LimiterUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& refBuilder,
                     Glib::RefPtr<Gio::Settings> refSettings)
    : Gtk::Grid(cobject), builder(refBuilder), settings(refSettings) {}

LimiterUi::~LimiterUi() {}

std::unique_ptr<LimiterUi> LimiterUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/limiter.glade");

    auto s = Gio::Settings::create(settings_name);

    LimiterUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, s);

    return std::unique_ptr<LimiterUi>(grid);
}
