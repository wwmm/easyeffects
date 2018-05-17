#ifndef SINK_INPUT_EFFECTS_HPP
#define SINK_INPUT_EFFECTS_HPP

#include "compressor.hpp"
#include "limiter.hpp"
#include "pipeline_base.hpp"
#include "pulse_manager.hpp"

class SinkInputEffects : public PipelineBase {
   public:
    SinkInputEffects(const std::shared_ptr<PulseManager>& pulse_manager);
    virtual ~SinkInputEffects();

    std::shared_ptr<PulseManager> pm;

    std::unique_ptr<Limiter> limiter;
    std::unique_ptr<Compressor> compressor;

   private:
    std::string log_tag = "sie: ";

    GSettings* sie_settings;

    std::array<GstInsertBin*, 2> wrappers;
    std::map<std::string, GstElement*> plugins;

    void add_plugins_to_pipeline();

    void on_app_added(const std::shared_ptr<AppInfo>& app_info);
};

#endif
