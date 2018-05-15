#ifndef SINK_INPUT_EFFECTS_HPP
#define SINK_INPUT_EFFECTS_HPP

#include "limiter.hpp"
#include "pipeline_base.hpp"
#include "pulse_manager.hpp"

class SinkInputEffects : public PipelineBase {
   public:
    SinkInputEffects(const std::shared_ptr<PulseManager>& pulse_manager);
    virtual ~SinkInputEffects();

    std::shared_ptr<PulseManager> pm;

    sigc::signal<void, std::array<double, 2>> limiter_input_level;
    sigc::signal<void, std::array<double, 2>> limiter_output_level;

   private:
    std::string log_tag = "sie: ";

    std::array<GstInsertBin*, 1> wrappers;
    std::map<std::string, GstElement*> plugins;
    std::unique_ptr<Limiter> limiter;

    void add_plugins_to_pipeline();

    void on_app_added(const std::shared_ptr<AppInfo>& app_info);
};

#endif
