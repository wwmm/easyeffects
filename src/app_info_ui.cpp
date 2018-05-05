#include "app_info_ui.hpp"

AppInfoUi::AppInfoUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& refBuilder,
                     std::shared_ptr<AppInfo> info,
                     std::shared_ptr<PulseManager> pulse_manager)
    : Gtk::Grid(cobject),
      builder(refBuilder),
      app_info(info),
      pm(pulse_manager) {
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

    enable->signal_state_set().connect(
        sigc::mem_fun(*this, &AppInfoUi::on_enable_app), false);

    volume->signal_value_changed().connect(
        sigc::mem_fun(*this, &AppInfoUi::on_volume_changed));

    mute->signal_toggled().connect(sigc::mem_fun(*this, &AppInfoUi::on_mute));
}

AppInfoUi::~AppInfoUi() {}

std::unique_ptr<AppInfoUi> AppInfoUi::create(std::shared_ptr<AppInfo> app_info,
                                             std::shared_ptr<PulseManager> pm) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/app_info.glade");

    AppInfoUi* app_info_ui = nullptr;

    builder->get_widget_derived("widgets_grid", app_info_ui, app_info, pm);

    return std::unique_ptr<AppInfoUi>(app_info_ui);
}

bool AppInfoUi::on_enable_app(bool state) {
    if (state) {
        if (app_info->app_type == "sink_input") {
            pm->move_sink_input_to_pulseeffects(app_info->index);
        } else {
            pm->move_source_output_to_pulseeffects(app_info->index);
        }
    } else {
        if (app_info->app_type == "sink_input") {
            pm->remove_sink_input_from_pulseeffects(app_info->index);
        } else {
            pm->remove_source_output_from_pulseeffects(app_info->index);
        }
    }

    return false;
}

void AppInfoUi::on_volume_changed() {
    auto value = volume->get_value();

    if (app_info->app_type == "sink_input") {
        pm->set_sink_input_volume(app_info->index, app_info->channels, value);
    } else {
        pm->set_source_output_volume(app_info->index, app_info->channels,
                                     value);
    }
}

void AppInfoUi::on_mute() {
    bool state = mute->get_active();

    if (state) {
        app_icon->set_from_icon_name("audio-volume-muted-symbolic",
                                     Gtk::ICON_SIZE_BUTTON);
    } else {
        app_icon->set_from_icon_name("audio-volume-high-symbolic",
                                     Gtk::ICON_SIZE_BUTTON);
    }

    if (app_info->app_type == "sink_input") {
        pm->set_sink_input_mute(app_info->index, state);
    } else {
        pm->set_source_output_mute(app_info->index, state);
    }
}
