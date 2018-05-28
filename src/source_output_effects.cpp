#include "source_output_effects.hpp"

namespace {

void on_message_element(const GstBus* gst_bus,
                        GstMessage* message,
                        SourceOutputEffects* soe) {
    auto src_name = GST_OBJECT_NAME(message->src);

    if (src_name == std::string("autovolume")) {
        soe->limiter->on_new_autovolume_level(soe->get_peak(message));
    } else if (src_name == std::string("compressor_input_level")) {
        soe->compressor_input_level.emit(soe->get_peak(message));
    } else if (src_name == std::string("compressor_output_level")) {
        soe->compressor_output_level.emit(soe->get_peak(message));
    } else if (src_name == std::string("equalizer_input_level")) {
        soe->equalizer_input_level.emit(soe->get_peak(message));
    } else if (src_name == std::string("equalizer_output_level")) {
        soe->equalizer_output_level.emit(soe->get_peak(message));
    } else if (src_name == std::string("gate_input_level")) {
        soe->gate_input_level.emit(soe->get_peak(message));
    } else if (src_name == std::string("gate_output_level")) {
        soe->gate_output_level.emit(soe->get_peak(message));
    } else if (src_name == std::string("deesser_input_level")) {
        soe->deesser_input_level.emit(soe->get_peak(message));
    } else if (src_name == std::string("deesser_output_level")) {
        soe->deesser_output_level.emit(soe->get_peak(message));
    } else if (src_name == std::string("pitch_input_level")) {
        soe->pitch_input_level.emit(soe->get_peak(message));
    } else if (src_name == std::string("pitch_output_level")) {
        soe->pitch_output_level.emit(soe->get_peak(message));
    } else if (src_name == std::string("webrtc_input_level")) {
        soe->webrtc_input_level.emit(soe->get_peak(message));
    } else if (src_name == std::string("webrtc_output_level")) {
        soe->webrtc_output_level.emit(soe->get_peak(message));
    }
}

GstPadProbeReturn on_pad_idle(GstPad* pad,
                              GstPadProbeInfo* info,
                              gpointer user_data) {
    auto l = static_cast<SourceOutputEffects*>(user_data);

    // unlinking elements using old plugins order

    gst_element_unlink(l->identity_in, l->plugins[l->plugins_order_old[0]]);

    for (long unsigned int n = 1; n < l->plugins_order_old.size(); n++) {
        gst_element_unlink(l->plugins[l->plugins_order_old[n - 1]],
                           l->plugins[l->plugins_order_old[n]]);

        std::cout << l->plugins_order[n - 1] << "->" << l->plugins_order[n]
                  << std::endl;
    }

    gst_element_unlink(
        l->plugins[l->plugins_order[l->plugins_order_old.size() - 1]],
        l->identity_out);

    // syncing elements state with effects_bin

    gst_bin_sync_children_states(GST_BIN(l->effects_bin));

    // linking elements using the new plugins order

    gst_element_link(l->identity_in, l->plugins[l->plugins_order[0]]);

    for (long unsigned int n = 1; n < l->plugins_order.size(); n++) {
        gst_element_link(l->plugins[l->plugins_order[n - 1]],
                         l->plugins[l->plugins_order[n]]);

        // std::cout << l->plugins_order[n - 1] << "->" << l->plugins_order[n]
        //           << std::endl;
    }

    gst_element_link(l->plugins[l->plugins_order[l->plugins_order.size() - 1]],
                     l->identity_out);

    return GST_PAD_PROBE_REMOVE;
}

void on_plugins_order_changed(GSettings* settings,
                              gchar* key,
                              SourceOutputEffects* l) {
    bool update = false;
    gchar* name;
    GVariantIter* iter;

    g_settings_get(settings, "plugins", "as", &iter);

    l->plugins_order_old = l->plugins_order;
    l->plugins_order.clear();

    while (g_variant_iter_next(iter, "s", &name)) {
        l->plugins_order.push_back(name);
    }

    g_variant_iter_free(iter);

    if (l->plugins_order.size() != l->plugins_order_old.size()) {
        update = true;
    } else if (!std::equal(l->plugins_order.begin(), l->plugins_order.end(),
                           l->plugins_order_old.begin())) {
        update = true;
    }

    if (update) {
        gst_pad_add_probe(gst_element_get_static_pad(l->identity_in, "src"),
                          GST_PAD_PROBE_TYPE_IDLE, on_pad_idle, l, nullptr);
    }
}

}  // namespace

SourceOutputEffects::SourceOutputEffects(
    const std::shared_ptr<PulseManager>& pulse_manager)
    : PipelineBase("soe: ", pulse_manager->mic_sink_info->rate),
      pm(pulse_manager),
      soe_settings(
          g_settings_new("com.github.wwmm.pulseeffects.sourceoutputs")) {
    set_pulseaudio_props(
        "application.id=com.github.wwmm.pulseeffects.sourceoutputs");

    set_output_sink_name("PulseEffects_mic");

    auto PULSE_SOURCE = std::getenv("PULSE_SOURCE");

    if (PULSE_SOURCE) {
        set_source_monitor_name(PULSE_SOURCE);
    } else {
        set_source_monitor_name(pm->server_info.default_source_name);
    }

    pm->source_output_added.connect(
        sigc::mem_fun(*this, &SourceOutputEffects::on_app_added));
    pm->source_output_changed.connect(
        sigc::mem_fun(*this, &SourceOutputEffects::on_app_changed));
    pm->source_output_removed.connect(
        sigc::mem_fun(*this, &SourceOutputEffects::on_app_removed));

    g_settings_bind(settings, "buffer-in", source, "buffer-time",
                    G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "latency-in", source, "latency-time",
                    G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "buffer-in", sink, "buffer-time",
                    G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "latency-in", sink, "latency-time",
                    G_SETTINGS_BIND_DEFAULT);

    // element message callback

    g_signal_connect(bus, "message::element", G_CALLBACK(on_message_element),
                     this);

    limiter = std::make_unique<Limiter>(
        log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.limiter");
    compressor = std::make_unique<Compressor>(
        log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.compressor");
    filter = std::make_unique<Filter>(
        log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.filter");
    equalizer = std::make_unique<Equalizer>(
        log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.equalizer");
    reverb = std::make_unique<Reverb>(
        log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.reverb");
    gate = std::make_unique<Gate>(
        log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.gate");
    deesser = std::make_unique<Deesser>(
        log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.deesser");
    pitch = std::make_unique<Pitch>(
        log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.pitch");
    webrtc = std::make_unique<Webrtc>(
        log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.webrtc");

    plugins.insert(std::make_pair(limiter->name, limiter->plugin));
    plugins.insert(std::make_pair(compressor->name, compressor->plugin));
    plugins.insert(std::make_pair(filter->name, filter->plugin));
    plugins.insert(std::make_pair(equalizer->name, equalizer->plugin));
    plugins.insert(std::make_pair(reverb->name, reverb->plugin));
    plugins.insert(std::make_pair(gate->name, gate->plugin));
    plugins.insert(std::make_pair(deesser->name, deesser->plugin));
    plugins.insert(std::make_pair(pitch->name, pitch->plugin));
    plugins.insert(std::make_pair(webrtc->name, webrtc->plugin));

    add_plugins_to_pipeline();

    g_signal_connect(soe_settings, "changed::plugins",
                     G_CALLBACK(on_plugins_order_changed), this);
}

SourceOutputEffects::~SourceOutputEffects() {}

void SourceOutputEffects::on_app_added(
    const std::shared_ptr<AppInfo>& app_info) {
    PipelineBase::on_app_added(app_info);

    auto enable_all_apps = g_settings_get_boolean(settings, "enable-all-apps");

    if (enable_all_apps && !app_info->connected) {
        pm->move_source_output_to_pulseeffects(app_info->index);
    }
}

void SourceOutputEffects::add_plugins_to_pipeline() {
    gchar* name;
    GVariantIter* iter;
    std::vector<std::string> default_order;

    g_settings_get(soe_settings, "plugins", "as", &iter);

    while (g_variant_iter_next(iter, "s", &name)) {
        plugins_order.push_back(name);
    }

    g_variant_get(g_settings_get_default_value(soe_settings, "plugins"), "as",
                  &iter);

    while (g_variant_iter_next(iter, "s", &name)) {
        default_order.push_back(name);
    }

    g_variant_iter_free(iter);

    // updating user list if there is any new plugin

    if (plugins_order.size() != default_order.size()) {
        plugins_order = default_order;

        g_settings_reset(soe_settings, "plugins");
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
