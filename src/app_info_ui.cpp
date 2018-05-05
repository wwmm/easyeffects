#include "app_info_ui.hpp"

AppInfoUi::AppInfoUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& refBuilder)
    : Gtk::Grid(cobject), builder(refBuilder) {
    // loading glade widgets

    builder->get_widget("enable", enable);
    builder->get_widget("app_icon", app_icon);
    builder->get_widget("app_name", app_name);
    builder->get_widget("volume", volume);
    builder->get_widget("format", format);
    builder->get_widget("rate", rate);
    builder->get_widget("channels", channels);
    builder->get_widget("resampler", resampler);
    builder->get_widget("buffer", buffer);
    builder->get_widget("latency", latency);
    builder->get_widget("state", state);
    builder->get_widget("level", level);
}

AppInfoUi::~AppInfoUi() {}

std::unique_ptr<AppInfoUi> AppInfoUi::create() {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/app_info.glade");

    AppInfoUi* app_info_ui = nullptr;

    builder->get_widget_derived("widgets_grid", app_info_ui);

    return std::unique_ptr<AppInfoUi>(app_info_ui);
}
