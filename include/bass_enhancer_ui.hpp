#ifndef BASS_ENHANCER_UI_HPP
#define BASS_ENHANCER_UI_HPP

#include <gtkmm/adjustment.h>
#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class BassEnhancerUi : public Gtk::Grid, public PluginUiBase {
   public:
    BassEnhancerUi(BaseObjectType* cobject,
                   const Glib::RefPtr<Gtk::Builder>& refBuilder,
                   std::string settings_name);
    ~BassEnhancerUi();

    static std::shared_ptr<BassEnhancerUi> create(std::string settings_name);

    void reset();

    void on_new_harmonics_level(double value);

   private:
    Gtk::Adjustment *amount, *blend, *floor, *harmonics, *input_gain,
        *output_gain, *scope;
    Gtk::LevelBar* harmonics_levelbar;
    Gtk::Label* harmonics_label;
};

#endif
