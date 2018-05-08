#include "sink_input_effects.hpp"

SinkInputEffects::SinkInputEffects(std::shared_ptr<PulseManager> pulse_manager)
    : pm(pulse_manager) {
    log_tag = "sink_input_effects: ";
}

SinkInputEffects::~SinkInputEffects() {}
