#ifndef SOURCE_OUTPUT_EFFECTS_UI_HPP
#define SOURCE_OUTPUT_EFFECTS_UI_HPP

#include "compressor_ui.hpp"
#include "deesser_ui.hpp"
#include "effects_base_ui.hpp"
#include "equalizer_ui.hpp"
#include "filter_ui.hpp"
#include "gate_ui.hpp"
#include "limiter_ui.hpp"
#include "pitch_ui.hpp"
#include "reverb_ui.hpp"
#include "source_output_effects.hpp"
#include "webrtc_ui.hpp"

class SourceOutputEffectsUi : public Gtk::Box, public EffectsBaseUi {
   public:
    SourceOutputEffectsUi(BaseObjectType* cobject,
                          const Glib::RefPtr<Gtk::Builder>& refBuilder,
                          const Glib::RefPtr<Gio::Settings>& refSettings,
                          const std::shared_ptr<SourceOutputEffects>& soe_ptr);

    virtual ~SourceOutputEffectsUi();

    static SourceOutputEffectsUi* create(
        std::shared_ptr<SourceOutputEffects> soe);

    void reset();

   protected:
    std::string log_tag = "soe_ui: ";

   private:
    std::shared_ptr<SourceOutputEffects> soe;

    LimiterUi* limiter_ui;
    CompressorUi* compressor_ui;
    FilterUi* filter_ui;
    EqualizerUi* equalizer_ui;
    ReverbUi* reverb_ui;
    GateUi* gate_ui;
    DeesserUi* deesser_ui;
    PitchUi* pitch_ui;
    WebrtcUi* webrtc_ui;

    void level_meters_connections();
    void up_down_connections();
};

#endif
