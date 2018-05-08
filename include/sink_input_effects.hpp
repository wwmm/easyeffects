#ifndef SINK_INPUT_EFFECTS_HPP
#define SINK_INPUT_EFFECTS_HPP

#include "pipeline_base.hpp"
#include "pulse_manager.hpp"

class SinkInputEffects : public PipelineBase {
   public:
    SinkInputEffects(std::shared_ptr<PulseManager> pulse_manager);
    virtual ~SinkInputEffects();

    std::shared_ptr<PulseManager> pm;
};

#endif
