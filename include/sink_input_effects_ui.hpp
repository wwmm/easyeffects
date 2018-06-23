#ifndef SINK_INPUT_EFFECTS_UI_HPP
#define SINK_INPUT_EFFECTS_UI_HPP

#include "bass_enhancer_ui.hpp"
#include "compressor_ui.hpp"
#include "crossfeed_ui.hpp"
#include "delay_ui.hpp"
#include "effects_base_ui.hpp"
#include "equalizer_ui.hpp"
#include "exciter_ui.hpp"
#include "expander_ui.hpp"
#include "filter_ui.hpp"
#include "limiter_ui.hpp"
#include "maximizer_ui.hpp"
#include "multiband_compressor_ui.hpp"
#include "panorama_ui.hpp"
#include "reverb_ui.hpp"
#include "sink_input_effects.hpp"
#include "stereo_enhancer_ui.hpp"

class SinkInputEffectsUi : public Gtk::Box, public EffectsBaseUi {
   public:
    SinkInputEffectsUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& refBuilder,
                       const Glib::RefPtr<Gio::Settings>& refSettings,
                       SinkInputEffects* sie_ptr);

    virtual ~SinkInputEffectsUi();

    void reset();

   protected:
    std::string log_tag = "sie_ui: ";

   private:
    SinkInputEffects* sie;

    LimiterUi* limiter_ui;
    CompressorUi* compressor_ui;
    FilterUi* filter_ui;
    EqualizerUi* equalizer_ui;
    ReverbUi* reverb_ui;
    BassEnhancerUi* bass_enhancer_ui;
    ExciterUi* exciter_ui;
    StereoEnhancerUi* stereo_enhancer_ui;
    PanoramaUi* panorama_ui;
    CrossfeedUi* crossfeed_ui;
    MaximizerUi* maximizer_ui;
    DelayUi* delay_ui;
    ExpanderUi* expander_ui;
    MultibandCompressorUi* multiband_compressor_ui;

    void level_meters_connections();
    void up_down_connections();
};

#endif
