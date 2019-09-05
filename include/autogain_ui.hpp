#ifndef AUTOGAIN_UI_HPP
#define AUTOGAIN_UI_HPP

#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include <gtkmm/togglebutton.h>
#include "plugin_ui_base.hpp"

class AutoGainUi : public Gtk::Grid, public PluginUiBase {
 public:
  AutoGainUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name);
  virtual ~AutoGainUi();

  void on_new_momentary(const float& value);
  void on_new_shortterm(const float& value);
  void on_new_integrated(const float& value);
  void on_new_relative(const float& value);
  void on_new_loudness(const float& value);
  void on_new_range(const float& value);
  void on_new_gain(const float& value);

 private:
  Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, target, weight_m, weight_s, weight_i;
  Gtk::LevelBar *m_level, *s_level, *i_level, *r_level, *g_level, *l_level, *lra_level;
  Gtk::Label *m_label, *s_label, *i_label, *r_label, *g_label, *l_label, *lra_label;
  Gtk::Button* reset;
  Gtk::ToggleButton* detect_silence;
};

#endif
