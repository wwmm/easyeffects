#ifndef SOURCE_OUTPUT_EFFECTS_HPP
#define SOURCE_OUTPUT_EFFECTS_HPP

#include "limiter.hpp"
#include "pipeline_base.hpp"
#include "pulse_manager.hpp"

class SourceOutputEffects : public PipelineBase {
   public:
    SourceOutputEffects(const std::shared_ptr<PulseManager>& pulse_manager);
    virtual ~SourceOutputEffects();

    std::shared_ptr<PulseManager> pm;

    std::unique_ptr<Limiter> limiter;

   private:
    std::string log_tag = "soe: ";

    std::array<GstInsertBin*, 1> wrappers;
    std::map<std::string, GstElement*> plugins;

    GSettings* soe_settings;

    void add_plugins_to_pipeline();

    void on_app_added(const std::shared_ptr<AppInfo>& app_info);
};

#endif
