#ifndef SINK_INPUT_EFFECTS_HPP
#define SINK_INPUT_EFFECTS_HPP

#include "autogain.hpp"
#include "bass_enhancer.hpp"
#include "convolver.hpp"
#include "crossfeed.hpp"
#include "crystalizer.hpp"
#include "delay.hpp"
#include "exciter.hpp"
#include "loudness.hpp"
#include "maximizer.hpp"
#include "multiband_compressor.hpp"
#include "multiband_gate.hpp"
#include "pipeline_base.hpp"
#include "pulse_manager.hpp"
#include "stereo_tools.hpp"

class SinkInputEffects : public PipelineBase {
 public:
  SinkInputEffects(PulseManager* pulse_manager);
  virtual ~SinkInputEffects();

  std::unique_ptr<BassEnhancer> bass_enhancer;
  std::unique_ptr<Exciter> exciter;
  std::unique_ptr<Crossfeed> crossfeed;
  std::unique_ptr<Maximizer> maximizer;
  std::unique_ptr<MultibandCompressor> multiband_compressor;
  std::unique_ptr<Loudness> loudness;
  std::unique_ptr<MultibandGate> multiband_gate;
  std::unique_ptr<StereoTools> stereo_tools;
  std::unique_ptr<Convolver> convolver;
  std::unique_ptr<Crystalizer> crystalizer;
  std::unique_ptr<AutoGain> autogain;
  std::unique_ptr<Delay> delay;

  sigc::signal<void, std::array<double, 2>> bass_enhancer_input_level;
  sigc::signal<void, std::array<double, 2>> bass_enhancer_output_level;
  sigc::signal<void, std::array<double, 2>> exciter_input_level;
  sigc::signal<void, std::array<double, 2>> exciter_output_level;
  sigc::signal<void, std::array<double, 2>> crossfeed_input_level;
  sigc::signal<void, std::array<double, 2>> crossfeed_output_level;
  sigc::signal<void, std::array<double, 2>> maximizer_input_level;
  sigc::signal<void, std::array<double, 2>> maximizer_output_level;
  sigc::signal<void, std::array<double, 2>> loudness_input_level;
  sigc::signal<void, std::array<double, 2>> loudness_output_level;
  sigc::signal<void, std::array<double, 2>> convolver_input_level;
  sigc::signal<void, std::array<double, 2>> convolver_output_level;
  sigc::signal<void, std::array<double, 2>> crystalizer_input_level;
  sigc::signal<void, std::array<double, 2>> crystalizer_output_level;
  sigc::signal<void, std::array<double, 2>> autogain_input_level;
  sigc::signal<void, std::array<double, 2>> autogain_output_level;
  sigc::signal<void, std::array<double, 2>> delay_input_level;
  sigc::signal<void, std::array<double, 2>> delay_output_level;

 private:
  void add_plugins_to_pipeline();

  void on_app_added(const std::shared_ptr<AppInfo>& app_info);
};

#endif
