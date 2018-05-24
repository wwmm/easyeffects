#ifndef SOURCE_OUTPUT_EFFECTS_HPP
#define SOURCE_OUTPUT_EFFECTS_HPP

#include "compressor.hpp"
#include "equalizer.hpp"
#include "filter.hpp"
#include "limiter.hpp"
#include "pipeline_base.hpp"
#include "pulse_manager.hpp"
#include "reverb.hpp"

class SourceOutputEffects : public PipelineBase {
   public:
    SourceOutputEffects(const std::shared_ptr<PulseManager>& pulse_manager);
    virtual ~SourceOutputEffects();

    std::shared_ptr<PulseManager> pm;

    std::array<std::string, 5> plugins_order;
    std::map<std::string, GstElement*> plugins;

    std::unique_ptr<Limiter> limiter;
    std::unique_ptr<Compressor> compressor;
    std::unique_ptr<Filter> filter;
    std::unique_ptr<Equalizer> equalizer;
    std::unique_ptr<Reverb> reverb;

    sigc::signal<void, std::array<double, 2>> compressor_input_level;
    sigc::signal<void, std::array<double, 2>> compressor_output_level;
    sigc::signal<void, std::array<double, 2>> equalizer_input_level;
    sigc::signal<void, std::array<double, 2>> equalizer_output_level;

   private:
    std::string log_tag = "soe: ";

    GSettings* soe_settings;

    void add_plugins_to_pipeline();

    void on_app_added(const std::shared_ptr<AppInfo>& app_info);
};

#endif
