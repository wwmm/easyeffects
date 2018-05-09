#ifndef SOURCE_OUTPUT_EFFECTS_HPP
#define SOURCE_OUTPUT_EFFECTS_HPP

#include "pipeline_base.hpp"
#include "pulse_manager.hpp"

class SourceOutputEffects : public PipelineBase {
   public:
    SourceOutputEffects(std::shared_ptr<PulseManager> pulse_manager);
    virtual ~SourceOutputEffects();

    std::shared_ptr<PulseManager> pm;

   private:
    void on_app_added(const std::shared_ptr<AppInfo>& app_info);
};

#endif
