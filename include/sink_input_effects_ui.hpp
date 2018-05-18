#ifndef SINK_INPUT_EFFECTS_UI_HPP
#define SINK_INPUT_EFFECTS_UI_HPP

#include "compressor_ui.hpp"
#include "effects_base_ui.hpp"
#include "limiter_ui.hpp"
#include "sink_input_effects.hpp"

class SinkInputEffectsUi : public EffectsBaseUi {
   public:
    SinkInputEffectsUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& refBuilder,
                       std::shared_ptr<SinkInputEffects> sie_ptr);

    ~SinkInputEffectsUi();

    static std::unique_ptr<SinkInputEffectsUi> create(
        std::shared_ptr<SinkInputEffects> sie);

    void reset();

   protected:
    std::string log_tag = "sie_ui: ";

   private:
    std::shared_ptr<SinkInputEffects> sie;

    Glib::RefPtr<Gio::Settings> settings;

    std::vector<sigc::connection> connections;

    std::shared_ptr<LimiterUi> limiter_ui;
    std::shared_ptr<CompressorUi> compressor_ui;

    void add_plugins();
};

#endif
