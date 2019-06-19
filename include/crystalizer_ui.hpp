#ifndef CRYSTALIZER_UI_HPP
#define CRYSTALIZER_UI_HPP

#include <gtkmm/grid.h>
#include <gtkmm/togglebutton.h>
#include "plugin_ui_base.hpp"

class CrystalizerUi : public Gtk::Grid, public PluginUiBase {
 public:
  CrystalizerUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name);
  virtual ~CrystalizerUi();

  void on_new_range_before(double value);

  void on_new_range_after(double value);

 private:
  Gtk::Grid* bands_grid;

  Gtk::LevelBar *range_before, *range_after;
  Gtk::Label *range_before_label, *range_after_label;
  Gtk::ToggleButton* aggressive;

  Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain;

  void build_bands(const int& nbands);
};

#endif
