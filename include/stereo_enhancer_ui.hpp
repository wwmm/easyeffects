#ifndef STEREO_ENHANCER_UI_HPP
#define STEREO_ENHANCER_UI_HPP

#include <gtkmm/button.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include <gtkmm/togglebutton.h>
#include "plugin_ui_base.hpp"

class StereoEnhancerUi : public Gtk::Grid, public PluginUiBase {
   public:
    StereoEnhancerUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& refBuilder,
                     const std::string& settings_name);
    ~StereoEnhancerUi();

    static std::shared_ptr<StereoEnhancerUi> create(std::string settings_name);

    void reset();

    void on_new_side_level(const std::array<double, 2>& peak);

   private:
    Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, side_gain,
        left_balance, left_delay, left_gain, right_balance, right_delay,
        right_gain;
    Gtk::ComboBoxText* middle_source;
    Gtk::ToggleButton *middle_phase, *left_phase, *right_phase;
    Gtk::LevelBar *side_l_levelbar, *side_r_levelbar;
};

#endif
