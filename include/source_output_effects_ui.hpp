#ifndef SOURCE_OUTPUT_EFFECTS_UI_HPP
#define SOURCE_OUTPUT_EFFECTS_UI_HPP

#include "compressor_ui.hpp"
#include "deesser_ui.hpp"
#include "effects_base_ui.hpp"
#include "equalizer_ui.hpp"
#include "filter_ui.hpp"
#include "gate_ui.hpp"
#include "limiter_ui.hpp"
#include "multiband_compressor_ui.hpp"
#include "multiband_gate_ui.hpp"
#include "pitch_ui.hpp"
#include "reverb_ui.hpp"
#include "source_output_effects.hpp"
#include "webrtc_ui.hpp"

class SourceOutputEffectsUi : public Gtk::Box, public EffectsBaseUi {
 public:
  SourceOutputEffectsUi(BaseObjectType* cobject,
                        const Glib::RefPtr<Gtk::Builder>& refBuilder,
                        const Glib::RefPtr<Gio::Settings>& refSettings,
                        SourceOutputEffects* soe_ptr);
  SourceOutputEffectsUi(const SourceOutputEffectsUi&) = delete;
  auto operator=(const SourceOutputEffectsUi&) -> SourceOutputEffectsUi& = delete;
  SourceOutputEffectsUi(const SourceOutputEffectsUi&&) = delete;
  auto operator=(const SourceOutputEffectsUi &&) -> SourceOutputEffectsUi& = delete;
  ~SourceOutputEffectsUi() override;

  static auto add_to_stack(Gtk::Stack* stack, SourceOutputEffects* soe_ptr) -> SourceOutputEffectsUi*;

 protected:
  std::string log_tag = "soe_ui: ";

 private:
  SourceOutputEffects* soe = nullptr;

  LimiterUi* limiter_ui = nullptr;
  CompressorUi* compressor_ui = nullptr;
  FilterUi* filter_ui = nullptr;
  EqualizerUi* equalizer_ui = nullptr;
  ReverbUi* reverb_ui = nullptr;
  GateUi* gate_ui = nullptr;
  DeesserUi* deesser_ui = nullptr;
  PitchUi* pitch_ui = nullptr;
  WebrtcUi* webrtc_ui = nullptr;
  MultibandCompressorUi* multiband_compressor_ui = nullptr;
  MultibandGateUi* multiband_gate_ui = nullptr;

  void level_meters_connections();
  void up_down_connections();
};

#endif
