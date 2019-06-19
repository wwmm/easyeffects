#ifndef LIMITER_UI_HPP
#define LIMITER_UI_HPP

#include <gtkmm/grid.h>
#include <gtkmm/togglebutton.h>
#include "plugin_ui_base.hpp"

class LimiterUi : public Gtk::Grid, public PluginUiBase {
 public:
  LimiterUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name);
  virtual ~LimiterUi();

  void on_new_attenuation(double value);

 private:
  Glib::RefPtr<Gtk::Adjustment> input_gain, limit, lookahead, release, oversampling;
  Gtk::ToggleButton* asc;
  Glib::RefPtr<Gtk::Adjustment> asc_level;

  Gtk::LevelBar* attenuation;
  Gtk::Label* attenuation_label;
};

#endif
