#ifndef SOURCE_OUTPUT_EFFECTS_UI_HPP
#define SOURCE_OUTPUT_EFFECTS_UI_HPP

#include "effects_base_ui.hpp"
#include "source_output_effects.hpp"

class SourceOutputEffectsUi : public EffectsBaseUi {
   public:
    SourceOutputEffectsUi(BaseObjectType* cobject,
                          const Glib::RefPtr<Gtk::Builder>& refBuilder,
                          std::shared_ptr<SourceOutputEffects> soe_ptr);

    ~SourceOutputEffectsUi();

    static std::unique_ptr<SourceOutputEffectsUi> create(
        std::shared_ptr<SourceOutputEffects> soe);

   protected:
    std::string log_tag = "source_output_effects_ui: ";

   private:
    std::shared_ptr<SourceOutputEffects> soe;
};

#endif
