#ifndef SOURCE_OUTPUT_EFFECTS_HPP
#define SOURCE_OUTPUT_EFFECTS_HPP

#include "pipeline_base.hpp"
#include "pulse_manager.hpp"

class SourceOutputEffects : public PipelineBase {
   public:
    SourceOutputEffects(std::shared_ptr<PulseManager> pulse_manager);
    virtual ~SourceOutputEffects();

    std::shared_ptr<PulseManager> pm;

   protected:
    std::string log_tag = "source_output_effects: ";
};

#endif
