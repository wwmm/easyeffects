#ifndef LIMITER_UI_HPP
#define LIMITER_UI_HPP

#include <gtkmm/adjustment.h>
#include <gtkmm/grid.h>
#include <gtkmm/togglebutton.h>
#include "plugin_ui_base.hpp"

class LimiterUi : public Gtk::Grid, public PluginUiBase {
   public:
    LimiterUi(BaseObjectType* cobject,
              const Glib::RefPtr<Gtk::Builder>& refBuilder,
              std::string settings_name);
    ~LimiterUi();

    static LimiterUi* create(std::string settings_name);

    std::string name = "limiter";

    void on_new_attenuation(double value);

   private:
    Gtk::Adjustment *input_gain, *limit, *lookahead, *release, *oversampling;
    Gtk::ToggleButton *asc, *autovolume_enable;
    Gtk::Adjustment *asc_level, *autovolume_target, *autovolume_window;
    Gtk::Adjustment *autovolume_threshold, *autovolume_tolerance;
    Gtk::Grid *autovolume_controls, *limiter_controls;
    Gtk::LevelBar* attenuation;
    Gtk::Label* attenuation_label;

    void init_autovolume();
};

#endif
