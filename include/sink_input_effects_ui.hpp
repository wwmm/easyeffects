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
#include "reverb_ui.hpp"
#include "sink_input_effects.hpp"
#include "stereo_tools_ui.hpp"

class SinkInputEffectsUi : public Gtk::Box, public EffectsBaseUi {
 public:
  SinkInputEffectsUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& refBuilder,
                     const Glib::RefPtr<Gio::Settings>& refSettings,
                     SinkInputEffects* sie_ptr);

  virtual ~SinkInputEffectsUi();

  static SinkInputEffectsUi* add_to_stack(Gtk::Stack* stack, SinkInputEffects* sie_ptr);

 protected:
  std::string log_tag = "sie_ui: ";

 private:
  SinkInputEffects* sie;

  LimiterUi* limiter_ui;
  CompressorUi* compressor_ui;
  FilterUi* filter_ui;
  EqualizerUi* equalizer_ui;
  PitchUi* pitch_ui;
  ReverbUi* reverb_ui;
  BassEnhancerUi* bass_enhancer_ui;
  ExciterUi* exciter_ui;
  CrossfeedUi* crossfeed_ui;
  MaximizerUi* maximizer_ui;
  MultibandCompressorUi* multiband_compressor_ui;
  LoudnessUi* loudness_ui;
  GateUi* gate_ui;
  MultibandGateUi* multiband_gate_ui;
  DeesserUi* deesser_ui;
  StereoToolsUi* stereo_tools_ui;
  ConvolverUi* convolver_ui;
  CrystalizerUi* crystalizer_ui;
  AutoGainUi* autogain_ui;
  DelayUi* delay_ui;

  void level_meters_connections();
  void up_down_connections();
};

#endif
