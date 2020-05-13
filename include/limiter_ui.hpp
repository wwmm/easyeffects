#ifndef LIMITER_UI_HPP
#define LIMITER_UI_HPP

#include <gtkmm/grid.h>
#include <gtkmm/togglebutton.h>
#include "plugin_ui_base.hpp"

class LimiterUi : public Gtk::Grid, public PluginUiBase {
 public:
  LimiterUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name);
  LimiterUi(const LimiterUi&) = delete;
  auto operator=(const LimiterUi&) -> LimiterUi& = delete;
  LimiterUi(const LimiterUi&&) = delete;
  auto operator=(const LimiterUi &&) -> LimiterUi& = delete;
  ~LimiterUi() override;

  void on_new_attenuation(double value);

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> input_gain, limit, lookahead, release, oversampling;
  Gtk::ToggleButton* asc = nullptr;
  Glib::RefPtr<Gtk::Adjustment> asc_level;

  Gtk::LevelBar* attenuation = nullptr;
  Gtk::Label* attenuation_label = nullptr;
};

#endif
