#include "source_output_effects.hpp"

SourceOutputEffects::SourceOutputEffects(
    std::shared_ptr<PulseManager> pulse_manager)
    : pm(pulse_manager) {
    log_tag = "source_output_effects: ";
}

SourceOutputEffects::~SourceOutputEffects() {}
