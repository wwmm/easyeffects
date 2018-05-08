#include "sink_input_effects.hpp"

SinkInputEffects::SinkInputEffects(std::shared_ptr<PulseManager> pulse_manager)
    : PipelineBase(pulse_manager->apps_sink_info->rate), pm(pulse_manager) {
    log_tag = "sie: ";
}

SinkInputEffects::~SinkInputEffects() {}
