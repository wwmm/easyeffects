#ifndef SOURCE_OUTPUT_EFFECTS_HPP
#define SOURCE_OUTPUT_EFFECTS_HPP

#include "pulse_manager.hpp"

class SourceOutputEffects {
   public:
    SourceOutputEffects(std::shared_ptr<PulseManager> pulse_manager);
    virtual ~SourceOutputEffects();

    std::shared_ptr<PulseManager> pm;
};

#endif
