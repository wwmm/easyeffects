#ifndef SOURCE_OUTPUT_EFFECTS_UI_HPP
#define SOURCE_OUTPUT_EFFECTS_UI_HPP

#include "compressor_ui.hpp"
#include "effects_base_ui.hpp"
#include "limiter_ui.hpp"
#include "source_output_effects.hpp"

class SourceOutputEffectsUi : public EffectsBaseUi {
   public:
    SourceOutputEffectsUi(BaseObjectType* cobject,
                          const Glib::RefPtr<Gtk::Builder>& refBuilder,
                          std::shared_ptr<SourceOutputEffects> soe_ptr);

    ~SourceOutputEffectsUi();

    static std::unique_ptr<SourceOutputEffectsUi> create(
        std::shared_ptr<SourceOutputEffects> soe);

    void reset();

   protected:
    std::string log_tag = "soe_ui: ";

   private:
    std::shared_ptr<SourceOutputEffects> soe;

    Glib::RefPtr<Gio::Settings> settings;

    std::vector<sigc::connection> connections;

    std::shared_ptr<LimiterUi> limiter_ui;
    std::shared_ptr<CompressorUi> compressor_ui;

    void add_plugins();
};

#endif
