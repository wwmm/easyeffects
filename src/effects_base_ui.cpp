#include "effects_base_ui.hpp"

EffectsBaseUi::EffectsBaseUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& refBuilder)
    : Gtk::Box(cobject) {}
