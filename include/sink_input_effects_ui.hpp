#ifndef SINK_INPUT_EFFECTS_UI_HPP
#define SINK_INPUT_EFFECTS_UI_HPP

#include "compressor_ui.hpp"
#include "effects_base_ui.hpp"
#include "equalizer_ui.hpp"
#include "filter_ui.hpp"
#include "limiter_ui.hpp"
#include "reverb_ui.hpp"
#include "sink_input_effects.hpp"

class SinkInputEffectsUi : public EffectsBaseUi {
   public:
    SinkInputEffectsUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& refBuilder,
                       const Glib::RefPtr<Gio::Settings>& refSettings,
                       std::shared_ptr<SinkInputEffects> sie_ptr);

    ~SinkInputEffectsUi();

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

    void level_meters_connections();
    void populate_listbox();
    void populate_stack();
    void up_down_connections();
};

#endif
