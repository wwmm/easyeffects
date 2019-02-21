#ifndef CRYSTALIZER_UI_HPP
#define CRYSTALIZER_UI_HPP

#include <gtkmm/grid.h>
#include <gtkmm/togglebutton.h>
#include "plugin_ui_base.hpp"

class CrystalizerUi : public Gtk::Grid, public PluginUiBase {
 public:
  CrystalizerUi(BaseObjectType* cobject,
                const Glib::RefPtr<Gtk::Builder>& builder,
                const std::string& settings_name);
  virtual ~CrystalizerUi();

 private:
  Gtk::ToggleButton *mute_band0, *mute_band1, *mute_band2, *mute_band3,
      *mute_band4;

  Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, intensity_band0,
      intensity_band1, intensity_band2, intensity_band3, intensity_band4, freq1,
      freq2, freq3, freq4;
};

#endif
