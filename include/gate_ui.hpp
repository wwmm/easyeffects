#ifndef GATE_UI_HPP
#define GATE_UI_HPP

#include "plugin_ui_base.hpp"

class GateUi : public Gtk::Grid, public PluginUiBase {
 public:
  GateUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name);
  GateUi(const GateUi&) = delete;
  auto operator=(const GateUi&) -> GateUi& = delete;
  GateUi(const GateUi&&) = delete;
  auto operator=(const GateUi &&) -> GateUi& = delete;
  ~GateUi() override;

  void on_new_gating(double value);

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> attack, release, threshold, knee, ratio, range, input, makeup;
  Gtk::LevelBar* gating = nullptr;
  Gtk::Label* gating_label = nullptr;
  Gtk::ComboBoxText *detection = nullptr, *stereo_link = nullptr;
};

#endif
