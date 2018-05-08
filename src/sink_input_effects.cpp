#include "sink_input_effects.hpp"

SinkInputEffects::SinkInputEffects(std::shared_ptr<PulseManager> pulse_manager)
    : PipelineBase(pulse_manager->apps_sink_info->rate), pm(pulse_manager) {
    log_tag = "sie: ";

    set_pulseaudio_props(
        "application.id=com.github.wwmm.pulseeffects.sinkinputs");

    set_source_monitor_name(pm->apps_sink_info->monitor_source_name);

    auto PULSE_SINK = std::getenv("PULSE_SINK");

    if (PULSE_SINK) {
        set_output_sink_name(PULSE_SINK);
    } else {
        set_output_sink_name(pm->server_info.default_sink_name);
    }
}

SinkInputEffects::~SinkInputEffects() {}
