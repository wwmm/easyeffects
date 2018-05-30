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

class SourceOutputEffectsUi : public EffectsBaseUi {
   public:
    SourceOutputEffectsUi(BaseObjectType* cobject,
                          const Glib::RefPtr<Gtk::Builder>& refBuilder,
                          const Glib::RefPtr<Gio::Settings>& refSettings,
                          const std::shared_ptr<SourceOutputEffects>& soe_ptr);

    ~SourceOutputEffectsUi();

    static std::shared_ptr<SourceOutputEffectsUi> create(
        std::shared_ptr<SourceOutputEffects> soe);

    void reset();

   protected:
    std::string log_tag = "soe_ui: ";

   private:
    std::shared_ptr<SourceOutputEffects> soe;

    std::shared_ptr<LimiterUi> limiter_ui;
    std::shared_ptr<CompressorUi> compressor_ui;
    std::shared_ptr<FilterUi> filter_ui;
    std::shared_ptr<EqualizerUi> equalizer_ui;
    std::shared_ptr<ReverbUi> reverb_ui;
    std::shared_ptr<GateUi> gate_ui;
    std::shared_ptr<DeesserUi> deesser_ui;
    std::shared_ptr<PitchUi> pitch_ui;
    std::shared_ptr<WebrtcUi> webrtc_ui;

    void level_meters_connections();
    void populate_listbox();
    void populate_stack();
    void up_down_connections();
};

#endif
