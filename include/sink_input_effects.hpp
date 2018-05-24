#ifndef SINK_INPUT_EFFECTS_HPP
#define SINK_INPUT_EFFECTS_HPP

#include "bass_enhancer.hpp"
#include "compressor.hpp"
#include "equalizer.hpp"
#include "exciter.hpp"
#include "filter.hpp"
#include "limiter.hpp"
#include "pipeline_base.hpp"
#include "pulse_manager.hpp"
#include "reverb.hpp"

class SinkInputEffects : public PipelineBase {
   public:
    SinkInputEffects(const std::shared_ptr<PulseManager>& pulse_manager);
    virtual ~SinkInputEffects();

    std::string log_tag = "sie: ";

    std::shared_ptr<PulseManager> pm;

    std::array<std::string, 6> plugins_order;
    std::map<std::string, GstElement*> plugins;

    std::unique_ptr<Limiter> limiter;
    std::unique_ptr<Compressor> compressor;
    std::unique_ptr<Filter> filter;
    std::unique_ptr<Equalizer> equalizer;
    std::unique_ptr<Reverb> reverb;
    std::unique_ptr<BassEnhancer> bass_enhancer;
    std::unique_ptr<Exciter> exciter;

    sigc::signal<void, std::array<double, 2>> compressor_input_level;
    sigc::signal<void, std::array<double, 2>> compressor_output_level;
    sigc::signal<void, std::array<double, 2>> equalizer_input_level;
    sigc::signal<void, std::array<double, 2>> equalizer_output_level;
    sigc::signal<void, std::array<double, 2>> bass_enhancer_input_level;
    sigc::signal<void, std::array<double, 2>> bass_enhancer_output_level;
    sigc::signal<void, std::array<double, 2>> exciter_input_level;
    sigc::signal<void, std::array<double, 2>> exciter_output_level;

   private:
    GSettings* sie_settings;

    void add_plugins_to_pipeline();

    void on_app_added(const std::shared_ptr<AppInfo>& app_info);
};

#endif
