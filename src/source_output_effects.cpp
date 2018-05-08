#include "source_output_effects.hpp"

SourceOutputEffects::SourceOutputEffects(
    std::shared_ptr<PulseManager> pulse_manager)
    : PipelineBase(pulse_manager->mic_sink_info->rate), pm(pulse_manager) {
    log_tag = "soe: ";
}

SourceOutputEffects::~SourceOutputEffects() {}
