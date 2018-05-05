#ifndef SINK_INPUT_EFFECTS_UI_HPP
#define SINK_INPUT_EFFECTS_UI_HPP

#include "effects_base_ui.hpp"
#include "sink_input_effects.hpp"

class SinkInputEffectsUi : public EffectsBaseUi {
   public:
    SinkInputEffectsUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& refBuilder,
                       std::shared_ptr<SinkInputEffects> sie);

    ~SinkInputEffectsUi();

    static std::unique_ptr<SinkInputEffectsUi> create(
        std::shared_ptr<SinkInputEffects> sie);

   private:
    void on_enable_app(bool state) override;

    void on_volume_changed() override;

    void on_mute() override;
};

#endif
