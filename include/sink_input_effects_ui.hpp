#ifndef EFFECTS_BASE_UI_HPP
#define EFFECTS_BASE_UI_HPP

#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <memory>
#include "sink_input_effects.hpp"

class SinkInputEffectsUi : public Gtk::Box {
   public:
    SinkInputEffectsUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& refBuilder,
                       std::shared_ptr<SinkInputEffects> sie);

    static SinkInputEffectsUi* create(std::shared_ptr<SinkInputEffects> sie);
};

#endif
