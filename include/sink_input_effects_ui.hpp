#ifndef SINK_INPUT_EFFECTS_UI_HPP
#define SINK_INPUT_EFFECTS_UI_HPP

#include "bass_enhancer_ui.hpp"
#include "compressor_ui.hpp"
#include "crossfeed_ui.hpp"
#include "delay_ui.hpp"
#include "effects_base_ui.hpp"
#include "equalizer_ui.hpp"
#include "exciter_ui.hpp"
#include "filter_ui.hpp"
#include "limiter_ui.hpp"
#include "maximizer_ui.hpp"
#include "panorama_ui.hpp"
#include "reverb_ui.hpp"
#include "sink_input_effects.hpp"
#include "stereo_enhancer_ui.hpp"

class SinkInputEffectsUi : public EffectsBaseUi {
   public:
    SinkInputEffectsUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& refBuilder,
                       const Glib::RefPtr<Gio::Settings>& refSettings,
                       const std::shared_ptr<SinkInputEffects>& sie_ptr);

    virtual ~SinkInputEffectsUi();

    static std::unique_ptr<SinkInputEffectsUi> create(
        std::shared_ptr<SinkInputEffects> sie);

    void reset();

   protected:
    std::string log_tag = "sie_ui: ";

   private:
    std::shared_ptr<SinkInputEffects> sie;

    std::shared_ptr<LimiterUi> limiter_ui;
    std::shared_ptr<CompressorUi> compressor_ui;
    std::shared_ptr<FilterUi> filter_ui;
    std::shared_ptr<EqualizerUi> equalizer_ui;
    std::shared_ptr<ReverbUi> reverb_ui;
    std::shared_ptr<BassEnhancerUi> bass_enhancer_ui;
    std::shared_ptr<ExciterUi> exciter_ui;
    std::shared_ptr<StereoEnhancerUi> stereo_enhancer_ui;
    std::shared_ptr<PanoramaUi> panorama_ui;
    std::shared_ptr<CrossfeedUi> crossfeed_ui;
    std::shared_ptr<MaximizerUi> maximizer_ui;
    std::shared_ptr<DelayUi> delay_ui;

    void level_meters_connections();
    void populate_listbox();
    void populate_stack();
    void up_down_connections();
};

#endif
