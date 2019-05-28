#ifndef SOURCE_OUTPUT_EFFECTS_HPP
#define SOURCE_OUTPUT_EFFECTS_HPP

#include "multiband_compressor.hpp"
#include "multiband_gate.hpp"
#include "pipeline_base.hpp"
#include "pulse_manager.hpp"
#include "webrtc.hpp"

class SourceOutputEffects : public PipelineBase {
 public:
  SourceOutputEffects(PulseManager* pulse_manager);
  virtual ~SourceOutputEffects();

  std::unique_ptr<Webrtc> webrtc;
  std::unique_ptr<MultibandCompressor> multiband_compressor;
  std::unique_ptr<MultibandGate> multiband_gate;

  sigc::signal<void, std::array<double, 2>> webrtc_input_level;
  sigc::signal<void, std::array<double, 2>> webrtc_output_level;

 private:
  void add_plugins_to_pipeline();

  void on_app_added(const std::shared_ptr<AppInfo>& app_info);
};

#endif
