#ifndef SINK_INPUT_EFFECTS_HPP
#define SINK_INPUT_EFFECTS_HPP

#include "pipeline_base.hpp"
#include "pulse_manager.hpp"

class SinkInputEffects : public PipelineBase {
   public:
    SinkInputEffects(std::shared_ptr<PulseManager> pulse_manager);
    virtual ~SinkInputEffects();

    std::shared_ptr<PulseManager> pm;

   protected:
    std::string log_tag = "sink_input_effects.cpp: ";
};

#endif
