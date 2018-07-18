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
    } else if (src_name == std::string("crossfeed_input_level")) {
        sie->crossfeed_input_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("crossfeed_output_level")) {
        sie->crossfeed_output_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("maximizer_input_level")) {
        sie->maximizer_input_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("maximizer_output_level")) {
        sie->maximizer_output_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("loudness_input_level")) {
        sie->loudness_input_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("loudness_output_level")) {
        sie->loudness_output_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("gate_input_level")) {
        sie->gate_input_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("gate_output_level")) {
        sie->gate_output_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("deesser_input_level")) {
        sie->deesser_input_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("deesser_output_level")) {
        sie->deesser_output_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("convolver_input_level")) {
        sie->convolver_input_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("convolver_output_level")) {
        sie->convolver_output_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("crystalizer_input_level")) {
        sie->crystalizer_input_level.emit(sie->get_peak(message));
    } else if (src_name == std::string("crystalizer_output_level")) {
        sie->crystalizer_output_level.emit(sie->get_peak(message));
    }
}

GstPadProbeReturn on_pad_idle(GstPad* pad,
                              GstPadProbeInfo* info,
                              gpointer user_data) {
    auto l = static_cast<SinkInputEffects*>(user_data);

    // unlinking elements using old plugins order

    gst_element_unlink(l->identity_in, l->plugins[l->plugins_order_old[0]]);

    for (long unsigned int n = 1; n < l->plugins_order_old.size(); n++) {
        gst_element_unlink(l->plugins[l->plugins_order_old[n - 1]],
                           l->plugins[l->plugins_order_old[n]]);
    }

    gst_element_unlink(
        l->plugins[l->plugins_order_old[l->plugins_order_old.size() - 1]],
        l->identity_out);

    for (auto& p : l->plugins) {
        gst_element_set_state(p.second, GST_STATE_NULL);
    }

    // linking elements using the new plugins order

    gst_element_link(l->identity_in, l->plugins[l->plugins_order[0]]);

    for (long unsigned int n = 1; n < l->plugins_order.size(); n++) {
        gst_element_link(l->plugins[l->plugins_order[n - 1]],
                         l->plugins[l->plugins_order[n]]);
    }

    gst_element_link(l->plugins[l->plugins_order[l->plugins_order.size() - 1]],
                     l->identity_out);

    // syncing elements state with effects_bin

    gst_bin_sync_children_states(GST_BIN(l->effects_bin));

    std::string list;

    for (auto name : l->plugins_order) {
        list += name + ",";
    }

    util::debug(l->log_tag + "new plugins order: [" + list + "]");

    return GST_PAD_PROBE_REMOVE;
}

void on_plugins_order_changed(GSettings* settings,
                              gchar* key,
                              SinkInputEffects* l) {
    bool update = false;
    gchar* name;
    GVariantIter* iter;

    g_settings_get(settings, "plugins", "as", &iter);

    l->plugins_order_old = l->plugins_order;
    l->plugins_order.clear();

    while (g_variant_iter_next(iter, "s", &name)) {
        l->plugins_order.push_back(name);
        g_free(name);
    }

    g_variant_iter_free(iter);

    if (l->plugins_order.size() != l->plugins_order_old.size()) {
        update = true;
    } else if (!std::equal(l->plugins_order.begin(), l->plugins_order.end(),
                           l->plugins_order_old.begin())) {
        update = true;
    }

    if (update) {
        auto srcpad = gst_element_get_static_pad(l->identity_in, "src");

        gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
                          on_pad_idle, l, nullptr);

        g_object_unref(srcpad);
    }
}

void on_blocksize_changed(GSettings* settings,
                          gchar* key,
                          SinkInputEffects* l) {
    int value = g_settings_get_enum(settings, key);
    int old_value;

    g_object_get(l->source, "blocksize", &old_value, nullptr);

    if (value != old_value) {
        GstState state;

        gst_element_get_state(l->pipeline, &state, nullptr, 0);

        if (state == GST_STATE_PLAYING) {
            gst_element_set_state(l->pipeline, GST_STATE_NULL);

            // 2 channels per buffer
            g_object_set(l->source, "blocksize", value * 2 * sizeof(float),
                         nullptr);

            l->update_pipeline_state();
        } else {
            // 2 channels per buffer
            g_object_set(l->source, "blocksize", value * 2 * sizeof(float),
                         nullptr);
        }
    }
}

}  // namespace

SinkInputEffects::SinkInputEffects(PulseManager* pulse_manager)
    : PipelineBase("sie: ", pulse_manager->apps_sink_info->rate),
      log_tag("sie: "),
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

    g_signal_connect(settings, "changed::blocksize-out",
                     G_CALLBACK(on_blocksize_changed), this);

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
    crossfeed = std::make_unique<Crossfeed>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.crossfeed");
    maximizer = std::make_unique<Maximizer>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.maximizer");
    multiband_compressor = std::make_unique<MultibandCompressor>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.multibandcompressor");
    loudness = std::make_unique<Loudness>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.loudness");
    gate = std::make_unique<Gate>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.gate");
    multiband_gate = std::make_unique<MultibandGate>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.multibandgate");
    deesser = std::make_unique<Deesser>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.deesser");
    stereo_tools = std::make_unique<StereoTools>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.stereotools");
    convolver = std::make_unique<Convolver>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.convolver");
    crystalizer = std::make_unique<Crystalizer>(
        log_tag, "com.github.wwmm.pulseeffects.sinkinputs.crystalizer");

    plugins.insert(std::make_pair(limiter->name, limiter->plugin));
    plugins.insert(std::make_pair(compressor->name, compressor->plugin));
    plugins.insert(std::make_pair(filter->name, filter->plugin));
    plugins.insert(std::make_pair(equalizer->name, equalizer->plugin));
    plugins.insert(std::make_pair(reverb->name, reverb->plugin));
    plugins.insert(std::make_pair(bass_enhancer->name, bass_enhancer->plugin));
    plugins.insert(std::make_pair(exciter->name, exciter->plugin));
    plugins.insert(
        std::make_pair(stereo_enhancer->name, stereo_enhancer->plugin));
    plugins.insert(std::make_pair(crossfeed->name, crossfeed->plugin));
    plugins.insert(std::make_pair(maximizer->name, maximizer->plugin));
    plugins.insert(std::make_pair(multiband_compressor->name,
                                  multiband_compressor->plugin));
    plugins.insert(std::make_pair(loudness->name, loudness->plugin));
    plugins.insert(std::make_pair(gate->name, gate->plugin));
    plugins.insert(
        std::make_pair(multiband_gate->name, multiband_gate->plugin));
    plugins.insert(std::make_pair(deesser->name, deesser->plugin));
    plugins.insert(std::make_pair(stereo_tools->name, stereo_tools->plugin));
    plugins.insert(std::make_pair(convolver->name, convolver->plugin));
    plugins.insert(std::make_pair(crystalizer->name, crystalizer->plugin));

    add_plugins_to_pipeline();

    g_signal_connect(sie_settings, "changed::plugins",
                     G_CALLBACK(on_plugins_order_changed), this);
}

SinkInputEffects::~SinkInputEffects() {
    g_object_unref(sie_settings);

    util::debug(log_tag + "destroyed");
}

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
    std::vector<std::string> default_order;

    g_settings_get(sie_settings, "plugins", "as", &iter);

    while (g_variant_iter_next(iter, "s", &name)) {
        plugins_order.push_back(name);
        g_free(name);
    }

    auto gvariant = g_settings_get_default_value(sie_settings, "plugins");

    g_variant_get(gvariant, "as", &iter);

    g_variant_unref(gvariant);

    while (g_variant_iter_next(iter, "s", &name)) {
        default_order.push_back(name);
        g_free(name);
    }

    g_variant_iter_free(iter);

    // updating user list if there is any new plugin

    if (plugins_order.size() != default_order.size()) {
        plugins_order = default_order;

        g_settings_reset(sie_settings, "plugins");
    }

    // adding plugins to effects_bin

    for (auto& p : plugins) {
        gst_bin_add(GST_BIN(effects_bin), p.second);
    }

    // linking plugins

    gst_element_unlink(identity_in, identity_out);

    gst_element_link(identity_in, plugins[plugins_order[0]]);

    for (long unsigned int n = 1; n < plugins_order.size(); n++) {
        gst_element_link(plugins[plugins_order[n - 1]],
                         plugins[plugins_order[n]]);
    }

    gst_element_link(plugins[plugins_order[plugins_order.size() - 1]],
                     identity_out);
}
