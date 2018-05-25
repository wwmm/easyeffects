#ifndef CROSSFEED_UI_HPP
#define CROSSFEED_UI_HPP

#include <gtkmm/adjustment.h>
#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class CrossfeedUi : public Gtk::Grid, public PluginUiBase {
   public:
    CrossfeedUi(BaseObjectType* cobject,
                const Glib::RefPtr<Gtk::Builder>& refBuilder,
                std::string settings_name);
    ~CrossfeedUi();

    static std::shared_ptr<CrossfeedUi> create(std::string settings_name);

    void reset();

   private:
    Gtk::Adjustment *fcut, *feed;
    Gtk::Button *preset_cmoy, *preset_default, *preset_jmeier;

    void init_presets_buttons();
};

#endif
