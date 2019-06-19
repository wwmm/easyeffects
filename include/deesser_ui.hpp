#ifndef DEESSER_UI_HPP
#define DEESSER_UI_HPP

#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include <gtkmm/togglebutton.h>
#include "plugin_ui_base.hpp"

class DeesserUi : public Gtk::Grid, public PluginUiBase {
 public:
  DeesserUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name);
  virtual ~DeesserUi();

  void on_new_compression(double value);
  void on_new_detected(double value);

 private:
  Glib::RefPtr<Gtk::Adjustment> threshold, ratio, makeup, f1_freq, f2_freq, f1_level, f2_level, f2_q;

  Gtk::LevelBar *compression, *detected;
  Gtk::Label *compression_label, *detected_label;
  Gtk::ComboBoxText *detection, *mode;
  Gtk::ToggleButton* sc_listen;
};

#endif
