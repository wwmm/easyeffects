#ifndef SINK_INPUT_EFFECTS_HPP
#define SINK_INPUT_EFFECTS_HPP

#include "pulse_manager.hpp"

class SinkInputEffects {
   public:
    SinkInputEffects(std::shared_ptr<PulseManager> pulse_manager);
    ~SinkInputEffects();

   private:
    std::shared_ptr<PulseManager> pm;
};

#endif
