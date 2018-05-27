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
    } else if (src_name == std::string("panorama_input_level")) {
        sie->panorama_input_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("panorama_output_level")) {
        sie->panorama_output_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("crossfeed_input_level")) {
        sie->crossfeed_input_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("crossfeed_output_level")) {
        sie->crossfeed_output_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("maximizer_input_level")) {
        sie->maximizer_input_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("maximizer_output_level")) {
        sie->maximizer_output_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("delay_input_level")) {
        sie->delay_input_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("delay_output_level")) {
        sie->delay_output_level.emit(sie->get_peak(message));
    }
}

void append_element(GstInsertBin* container, GstElement* element) {
    if (element) {
        bool wait_append = true;

        gst_insert_bin_append(container, element,
                              [](auto bin, auto elem, auto success, auto d) {
                                  bool* wait = static_cast<bool*>(d);
                                  *wait = false;
                              },
                              &wait_append);

        while (wait_append) {
        }
    }
}

void remove_element(GstInsertBin* container, GstElement* element) {
    if (element) {
        bool wait_remove = true;

        gst_insert_bin_remove(container, element,
                              [](auto bin, auto elem, auto success, auto d) {
                                  bool* wait = static_cast<bool*>(d);
                                  *wait = false;
                              },
                              &wait_remove);

        while (wait_remove) {
        }
    }
}

void on_plugins_order_changed(GSettings* settings,
                              gchar* key,
                              SinkInputEffects* l) {
    bool update = false;
    gchar* name;
    GVariantIter* iter;
    std::vector<std::string> plugins_order;

    g_settings_get(settings, "plugins", "as", &iter);

    while (g_variant_iter_next(iter, "s", &name)) {
        plugins_order.push_back(name);
    }

    g_variant_iter_free(iter);

    if (plugins_order.size() != l->plugins_order.size()) {
        l->plugins_order = plugins_order;

        update = true;
    } else if (!std::equal(plugins_order.begin(), plugins_order.end(),
                           l->plugins_order.begin())) {
        l->plugins_order = plugins_order;

        update = true;
    }

    if (update) {
        int idx = plugins_order.size() - 1;

        gst_element_set_state(l->pipeline, GST_STATE_NULL);

        do {
            auto plugin =
                gst_bin_get_by_name(GST_BIN(l->effects_bin),
                                    (plugins_order[idx] + "_plugin").c_str());

            remove_element(l->effects_bin, plugin);

            idx--;
        } while (idx >= 0);

        for (long unsigned int n = 0; n < plugins_order.size(); n++) {
            append_element(l->effects_bin, l->plugins[plugins_order[n]]);
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
    panorama = std::make_unique<Panorama>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.panorama");
    crossfeed = std::make_unique<Crossfeed>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.crossfeed");
    maximizer = std::make_unique<Maximizer>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.maximizer");
    delay = std::make_unique<Delay>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.delay");

    plugins.insert(std::make_pair(limiter->name, limiter->plugin));
    plugins.insert(std::make_pair(compressor->name, compressor->plugin));
    plugins.insert(std::make_pair(filter->name, filter->plugin));
    plugins.insert(std::make_pair(equalizer->name, equalizer->plugin));
    plugins.insert(std::make_pair(reverb->name, reverb->plugin));
    plugins.insert(std::make_pair(bass_enhancer->name, bass_enhancer->plugin));
    plugins.insert(std::make_pair(exciter->name, exciter->plugin));
    plugins.insert(
        std::make_pair(stereo_enhancer->name, stereo_enhancer->plugin));
    plugins.insert(std::make_pair(panorama->name, panorama->plugin));
    plugins.insert(std::make_pair(crossfeed->name, crossfeed->plugin));
    plugins.insert(std::make_pair(maximizer->name, maximizer->plugin));
    plugins.insert(std::make_pair(delay->name, delay->plugin));

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
