#ifndef AUTOGAIN_UI_HPP
#define AUTOGAIN_UI_HPP

#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include <gtkmm/togglebutton.h>
#include "plugin_ui_base.hpp"

class AutoGainUi : public Gtk::Grid, public PluginUiBase {
 public:
  AutoGainUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name);
  AutoGainUi(const AutoGainUi&) = delete;
  auto operator=(const AutoGainUi&) -> AutoGainUi& = delete;
  AutoGainUi(const AutoGainUi&&) = delete;
  auto operator=(const AutoGainUi &&) -> AutoGainUi& = delete;
  ~AutoGainUi() override;

  void on_new_momentary(const float& value);
  void on_new_shortterm(const float& value);
  void on_new_integrated(const float& value);
  void on_new_relative(const float& value);
  void on_new_loudness(const float& value);
  void on_new_range(const float& value);
  void on_new_gain(const float& value);

 private:
  Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, target, weight_m, weight_s, weight_i;

  Gtk::LevelBar *m_level = nullptr, *s_level = nullptr, *i_level = nullptr, *r_level = nullptr, *g_level = nullptr,
                *l_level = nullptr, *lra_level = nullptr;

  Gtk::Label *m_label = nullptr, *s_label = nullptr, *i_label = nullptr, *r_label = nullptr, *g_label = nullptr,
             *l_label = nullptr, *lra_label = nullptr;

  Gtk::Button* reset = nullptr;

  Gtk::ToggleButton *detect_silence = nullptr, *use_geometric_mean = nullptr;

  Gtk::Grid* weights_grid = nullptr;
};

#endif
