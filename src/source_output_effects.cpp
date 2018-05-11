#include "source_output_effects.hpp"

SourceOutputEffects::SourceOutputEffects(
    std::shared_ptr<PulseManager> pulse_manager)
    : PipelineBase("soe: ", pulse_manager->mic_sink_info->rate),
      pm(pulse_manager) {
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
