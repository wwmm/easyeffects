#include "source_output_effects.hpp"

SourceOutputEffects::SourceOutputEffects(
    std::shared_ptr<PulseManager> pulse_manager)
    : PipelineBase(pulse_manager->mic_sink_info->rate), pm(pulse_manager) {
    log_tag = "soe: ";

    set_pulseaudio_props(
        "application.id=com.github.wwmm.pulseeffects.sourceoutputs");

    set_output_sink_name("PulseEffects_mic");

    auto PULSE_SOURCE = std::getenv("PULSE_SOURCE");

    if (PULSE_SOURCE) {
        set_source_monitor_name(PULSE_SOURCE);
    } else {
        set_source_monitor_name(pm->server_info.default_source_name);
    }
}

SourceOutputEffects::~SourceOutputEffects() {}
