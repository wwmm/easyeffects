#ifndef CRYSTALIZER_UI_HPP
#define CRYSTALIZER_UI_HPP

#include <gtkmm/grid.h>
#include <gtkmm/image.h>
#include <gtkmm/scale.h>
#include <gtkmm/togglebutton.h>
#include "plugin_ui_base.hpp"

class CrystalizerUi : public Gtk::Grid, public PluginUiBase {
 public:
  CrystalizerUi(BaseObjectType* cobject,
                const Glib::RefPtr<Gtk::Builder>& builder,
                const std::string& settings_name);
  virtual ~CrystalizerUi();

 private:
  Gtk::ToggleButton *mute_low, *mute_mid, *mute_high;
  Gtk::Scale *scale_low, *scale_mid, *scale_high;
  Gtk::Image *mute_icon_low, *mute_icon_mid, *mute_icon_high;

  Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, intensity_low,
      intensity_mid, intensity_high, volume_low, volume_mid, volume_high;
};

#endif
