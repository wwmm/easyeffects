#ifndef MULTIBAND_COMPRESSOR_UI_HPP
#define MULTIBAND_COMPRESSOR_UI_HPP

#include <gtkmm/button.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include <gtkmm/togglebutton.h>
#include "plugin_ui_base.hpp"

class MultibandCompressorUi : public Gtk::Grid, public PluginUiBase {
 public:
  MultibandCompressorUi(BaseObjectType* cobject,
                        const Glib::RefPtr<Gtk::Builder>& builder,
                        const std::string& settings_name);
  virtual ~MultibandCompressorUi();

  void on_new_output0(double value);
  void on_new_output1(double value);
  void on_new_output2(double value);
  void on_new_output3(double value);

  void on_new_compression0(double value);
  void on_new_compression1(double value);
  void on_new_compression2(double value);
  void on_new_compression3(double value);

 private:
  Glib::RefPtr<Gtk::Adjustment> freq0, freq1, freq2, input_gain, output_gain;
  Glib::RefPtr<Gtk::Adjustment> attack0, release0, threshold0, knee0, ratio0,
      makeup0;
  Glib::RefPtr<Gtk::Adjustment> attack1, release1, threshold1, knee1, ratio1,
      makeup1;
  Glib::RefPtr<Gtk::Adjustment> attack2, release2, threshold2, knee2, ratio2,
      makeup2;
  Glib::RefPtr<Gtk::Adjustment> attack3, release3, threshold3, knee3, ratio3,
      makeup3;
  Gtk::LevelBar *output0, *output1, *output2, *output3;
  Gtk::Label *output0_label, *output1_label, *output2_label, *output3_label;
  Gtk::LevelBar *compression0, *compression1, *compression2, *compression3;
  Gtk::Label *compression0_label, *compression1_label, *compression2_label,
      *compression3_label;
  Gtk::ComboBoxText *mode, *detection0, *detection1, *detection2, *detection3;
  Gtk::ToggleButton *bypass0, *bypass1, *bypass2, *bypass3, *solo0, *solo1,
      *solo2, *solo3;
};

#endif
