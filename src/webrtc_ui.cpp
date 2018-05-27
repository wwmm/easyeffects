#include "webrtc_ui.hpp"

WebrtcUi::WebrtcUi(BaseObjectType* cobject,
                   const Glib::RefPtr<Gtk::Builder>& refBuilder,
                   std::string settings_name)
    : Gtk::Grid(cobject), PluginUiBase(refBuilder, settings_name) {
    name = "webrtc";

    // loading glade widgets

    builder->get_widget("echo_cancel", echo_cancel);
    builder->get_widget("extended_filter", extended_filter);
    builder->get_widget("high_pass_filter", high_pass_filter);
    builder->get_widget("delay_agnostic", delay_agnostic);
    builder->get_widget("noise_suppression", noise_suppression);
    builder->get_widget("gain_control", gain_control);
    builder->get_widget("limiter", limiter);
    builder->get_widget("voice_detection", voice_detection);
    builder->get_widget("echo_suppression_level", echo_suppression_level);
    builder->get_widget("noise_suppression_level", noise_suppression_level);
    builder->get_widget("gain_control_mode", gain_control_mode);
    builder->get_widget("voice_detection_likelihood",
                        voice_detection_likelihood);

    get_object("compression_gain_db", compression_gain_db);
    get_object("target_level_dbfs", target_level_dbfs);
    get_object("voice_detection_frame_size", voice_detection_frame_size);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("compression-gain-db", compression_gain_db, "value", flag);
    settings->bind("target-level-dbfs", target_level_dbfs, "value", flag);
    settings->bind("voice-detection-frame-size-ms", voice_detection_frame_size,
                   "value", flag);
    settings->bind("echo-cancel", echo_cancel, "value", flag);
    settings->bind("extended-filter", extended_filter, "value", flag);
    settings->bind("high-pass-filter", high_pass_filter, "value", flag);
    settings->bind("delay-agnostic", delay_agnostic, "value", flag);
    settings->bind("noise-suppression", noise_suppression, "value", flag);
    settings->bind("gain-control", gain_control, "value", flag);
    settings->bind("limiter", limiter, "value", flag);
    settings->bind("voice-detection", voice_detection, "value", flag);

    settings->set_boolean("post-messages", true);
}

WebrtcUi::~WebrtcUi() {
    settings->set_boolean("post-messages", false);
}

std::shared_ptr<WebrtcUi> WebrtcUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/webrtc.glade");

    WebrtcUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    return std::shared_ptr<WebrtcUi>(grid);
}

void WebrtcUi::reset() {
    settings->reset("state");
    settings->reset("compression_gain_db");
}
