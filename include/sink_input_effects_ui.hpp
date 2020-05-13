#ifndef SINK_INPUT_EFFECTS_UI_HPP
#define SINK_INPUT_EFFECTS_UI_HPP

#include "autogain_ui.hpp"
#include "bass_enhancer_ui.hpp"
#include "compressor_ui.hpp"
#include "convolver_ui.hpp"
#include "crossfeed_ui.hpp"
#include "crystalizer_ui.hpp"
#include "deesser_ui.hpp"
#include "delay_ui.hpp"
#include "effects_base_ui.hpp"
#include "equalizer_ui.hpp"
#include "exciter_ui.hpp"
#include "filter_ui.hpp"
#include "gate_ui.hpp"
#include "limiter_ui.hpp"
#include "loudness_ui.hpp"
#include "maximizer_ui.hpp"
#include "multiband_compressor_ui.hpp"
#include "multiband_gate_ui.hpp"
#include "pitch_ui.hpp"
#include "preset_type.hpp"
#include "reverb_ui.hpp"
#include "sink_input_effects.hpp"
#include "stereo_tools_ui.hpp"

class SinkInputEffectsUi : public Gtk::Box, public EffectsBaseUi {
 public:
  SinkInputEffectsUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& refBuilder,
                     const Glib::RefPtr<Gio::Settings>& refSettings,
                     SinkInputEffects* sie_ptr);
  SinkInputEffectsUi(const SinkInputEffectsUi&) = delete;
  auto operator=(const SinkInputEffectsUi&) -> SinkInputEffectsUi& = delete;
  SinkInputEffectsUi(const SinkInputEffectsUi&&) = delete;
  auto operator=(const SinkInputEffectsUi &&) -> SinkInputEffectsUi& = delete;
  ~SinkInputEffectsUi() override;

  static auto add_to_stack(Gtk::Stack* stack, SinkInputEffects* sie_ptr) -> SinkInputEffectsUi*;

 protected:
  std::string log_tag = "sie_ui: ";

 private:
  SinkInputEffects* sie = nullptr;

  LimiterUi* limiter_ui = nullptr;
  CompressorUi* compressor_ui = nullptr;
  FilterUi* filter_ui = nullptr;
  EqualizerUi* equalizer_ui = nullptr;
  PitchUi* pitch_ui = nullptr;
  ReverbUi* reverb_ui = nullptr;
  BassEnhancerUi* bass_enhancer_ui = nullptr;
  ExciterUi* exciter_ui = nullptr;
  CrossfeedUi* crossfeed_ui = nullptr;
  MaximizerUi* maximizer_ui = nullptr;
  MultibandCompressorUi* multiband_compressor_ui = nullptr;
  LoudnessUi* loudness_ui = nullptr;
  GateUi* gate_ui = nullptr;
  MultibandGateUi* multiband_gate_ui = nullptr;
  DeesserUi* deesser_ui = nullptr;
  StereoToolsUi* stereo_tools_ui = nullptr;
  ConvolverUi* convolver_ui = nullptr;
  CrystalizerUi* crystalizer_ui = nullptr;
  AutoGainUi* autogain_ui = nullptr;
  DelayUi* delay_ui = nullptr;

  void level_meters_connections();
  void up_down_connections();
};

#endif
