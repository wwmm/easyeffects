#include "sink_input_effects.hpp"

namespace {

void on_message_element(const GstBus* gst_bus,
                        GstMessage* message,
                        SinkInputEffects* sie) {
    auto src_name = GST_OBJECT_NAME(message->src);

    if (src_name == std::string("autovolume")) {
        sie->limiter->on_new_autovolume_level(sie->get_peak(message));
    } else if (src_name == std::string("compressor_input_level")) {
        sie->compressor_input_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("compressor_output_level")) {
        sie->compressor_output_level.emit(sie->get_peak(message));
    }
}

}  // namespace

SinkInputEffects::SinkInputEffects(
    const std::shared_ptr<PulseManager>& pulse_manager)
    : PipelineBase("sie: ", pulse_manager->apps_sink_info->rate),
      pm(pulse_manager),
      sie_settings(g_settings_new("com.github.wwmm.pulseeffects.sinkinputs")) {
    set_pulseaudio_props(
        "application.id=com.github.wwmm.pulseeffects.sinkinputs");

    set_source_monitor_name(pm->apps_sink_info->monitor_source_name);

    auto PULSE_SINK = std::getenv("PULSE_SINK");

    if (PULSE_SINK) {
        set_output_sink_name(PULSE_SINK);
    } else {
        set_output_sink_name(pm->server_info.default_sink_name);
    }

    pm->sink_input_added.connect(
        sigc::mem_fun(*this, &SinkInputEffects::on_app_added));
    pm->sink_input_changed.connect(
        sigc::mem_fun(*this, &SinkInputEffects::on_app_changed));
    pm->sink_input_removed.connect(
        sigc::mem_fun(*this, &SinkInputEffects::on_app_removed));

    g_settings_bind(settings, "buffer-out", source, "buffer-time",
                    G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "latency-out", source, "latency-time",
                    G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "buffer-out", sink, "buffer-time",
                    G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "latency-out", sink, "latency-time",
                    G_SETTINGS_BIND_DEFAULT);

    // element message callback

    g_signal_connect(bus, "message::element", G_CALLBACK(on_message_element),
                     this);

    // plugins wrappers

    for (long unsigned int n = 0; n < wrappers.size(); n++) {
        wrappers[n] = GST_INSERT_BIN(gst_insert_bin_new(
            std::string("wrapper" + std::to_string(n)).c_str()));

        gst_insert_bin_append(effects_bin, GST_ELEMENT(wrappers[n]), nullptr,
                              nullptr);
    }

    // plugins

    limiter = std::make_unique<Limiter>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.limiter");
    compressor = std::make_unique<Compressor>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.compressor");

    plugins.insert(std::make_pair(limiter->name, limiter->plugin));
    plugins.insert(std::make_pair(compressor->name, compressor->plugin));

    add_plugins_to_pipeline();
}

SinkInputEffects::~SinkInputEffects() {}

void SinkInputEffects::on_app_added(const std::shared_ptr<AppInfo>& app_info) {
    PipelineBase::on_app_added(app_info);

    auto enable_all_apps = g_settings_get_boolean(settings, "enable-all-apps");

    if (enable_all_apps && !app_info->connected) {
        pm->move_sink_input_to_pulseeffects(app_info->index);
    }
}

void SinkInputEffects::add_plugins_to_pipeline() {
    uint index = 0;
    gchar* name;
    GVariantIter* iter;

    g_settings_get(sie_settings, "plugins", "as", &iter);

    while (g_variant_iter_next(iter, "s", &name)) {
        gst_insert_bin_append(wrappers[index], plugins[name], nullptr, nullptr);

        index++;
    }

    g_variant_iter_free(iter);
}
