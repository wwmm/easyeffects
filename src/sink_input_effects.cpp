#include "sink_input_effects.hpp"
#include "util.hpp"

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
    } else if (src_name == std::string("equalizer_input_level")) {
        sie->equalizer_input_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("equalizer_output_level")) {
        sie->equalizer_output_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("bass_enhancer_input_level")) {
        sie->bass_enhancer_input_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("bass_enhancer_output_level")) {
        sie->bass_enhancer_output_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("exciter_input_level")) {
        sie->exciter_input_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("exciter_output_level")) {
        sie->exciter_output_level.emit(sie->get_peak(message));
    }
}

void on_plugins_order_changed(GSettings* settings,
                              gchar* key,
                              SinkInputEffects* l) {
    bool update_order = false;
    uint count = 0;
    gchar* name;
    GVariantIter* iter;
    auto old_order = l->plugins_order;

    g_settings_get(settings, "plugins", "as", &iter);

    while (g_variant_iter_next(iter, "s", &name)) {
        l->plugins_order[count] = name;

        if (old_order[count] != name) {
            update_order = true;
        }

        count++;
    }

    g_variant_iter_free(iter);

    if (update_order) {
        int idx = old_order.size() - 1;

        gst_element_set_state(l->pipeline, GST_STATE_READY);

        do {
            auto plugin = gst_bin_get_by_name(
                GST_BIN(l->effects_bin), (old_order[idx] + "_plugin").c_str());

            if (plugin) {
                gst_insert_bin_remove(GST_INSERT_BIN(l->effects_bin), plugin,
                                      nullptr, nullptr);
            }

            idx--;
        } while (idx >= 0);

        for (long unsigned int n = 0; n < l->plugins_order.size(); n++) {
            gst_insert_bin_append(GST_INSERT_BIN(l->effects_bin),
                                  l->plugins[l->plugins_order[n]], nullptr,
                                  nullptr);
        }

        l->update_pipeline_state();
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

    limiter = std::make_unique<Limiter>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.limiter");
    compressor = std::make_unique<Compressor>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.compressor");
    filter = std::make_unique<Filter>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.filter");
    equalizer = std::make_unique<Equalizer>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.equalizer");
    reverb = std::make_unique<Reverb>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.reverb");
    bass_enhancer = std::make_unique<BassEnhancer>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.bassenhancer");
    exciter = std::make_unique<Exciter>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.exciter");
    stereo_enhancer = std::make_unique<StereoEnhancer>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.stereoenhancer");

    plugins.insert(std::make_pair(limiter->name, limiter->plugin));
    plugins.insert(std::make_pair(compressor->name, compressor->plugin));
    plugins.insert(std::make_pair(filter->name, filter->plugin));
    plugins.insert(std::make_pair(equalizer->name, equalizer->plugin));
    plugins.insert(std::make_pair(reverb->name, reverb->plugin));
    plugins.insert(std::make_pair(bass_enhancer->name, bass_enhancer->plugin));
    plugins.insert(std::make_pair(exciter->name, exciter->plugin));
    plugins.insert(
        std::make_pair(stereo_enhancer->name, stereo_enhancer->plugin));

    add_plugins_to_pipeline();

    g_signal_connect(sie_settings, "changed::plugins",
                     G_CALLBACK(on_plugins_order_changed), this);
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
    gchar* name;
    GVariantIter* iter;

    g_settings_get(sie_settings, "plugins", "as", &iter);

    while (g_variant_iter_next(iter, "s", &name)) {
        gst_insert_bin_append(effects_bin, plugins[name], nullptr, nullptr);

        plugins_order.push_back(name);
    }

    g_variant_iter_free(iter);
}
