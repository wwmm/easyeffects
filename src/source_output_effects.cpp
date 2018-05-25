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
    }
}

void on_plugins_order_changed(GSettings* settings,
                              gchar* key,
                              SourceOutputEffects* l) {
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

    plugins.insert(std::make_pair(limiter->name, limiter->plugin));
    plugins.insert(std::make_pair(compressor->name, compressor->plugin));
    plugins.insert(std::make_pair(filter->name, filter->plugin));
    plugins.insert(std::make_pair(equalizer->name, equalizer->plugin));
    plugins.insert(std::make_pair(reverb->name, reverb->plugin));

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

    g_settings_get(soe_settings, "plugins", "as", &iter);

    while (g_variant_iter_next(iter, "s", &name)) {
        gst_insert_bin_append(effects_bin, plugins[name], nullptr, nullptr);

        plugins_order.push_back(name);
    }

    g_variant_iter_free(iter);
}
