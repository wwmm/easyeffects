#ifndef DELAY_UI_HPP
#define DELAY_UI_HPP

#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class DelayUi : public Gtk::Grid, public PluginUiBase {
 public:
  DelayUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name);
  virtual ~DelayUi();

 private:
  Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, time_l, time_r;
};

#endif
