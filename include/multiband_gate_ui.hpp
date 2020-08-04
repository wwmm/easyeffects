#ifndef MULTIBAND_GATE_UI_HPP
#define MULTIBAND_GATE_UI_HPP

#include "plugin_ui_base.hpp"

class MultibandGateUi : public Gtk::Grid, public PluginUiBase {
 public:
  MultibandGateUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name);
  MultibandGateUi(const MultibandGateUi&) = delete;
  auto operator=(const MultibandGateUi&) -> MultibandGateUi& = delete;
  MultibandGateUi(const MultibandGateUi&&) = delete;
  auto operator=(const MultibandGateUi &&) -> MultibandGateUi& = delete;
  ~MultibandGateUi() override;

  void on_new_output0(double value);
  void on_new_output1(double value);
  void on_new_output2(double value);
  void on_new_output3(double value);

  void on_new_gating0(double value);
  void on_new_gating1(double value);
  void on_new_gating2(double value);
  void on_new_gating3(double value);

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> freq0, freq1, freq2, input_gain, output_gain;
  Glib::RefPtr<Gtk::Adjustment> range0, attack0, release0, threshold0, knee0, ratio0, makeup0;
  Glib::RefPtr<Gtk::Adjustment> range1, attack1, release1, threshold1, knee1, ratio1, makeup1;
  Glib::RefPtr<Gtk::Adjustment> range2, attack2, release2, threshold2, knee2, ratio2, makeup2;
  Glib::RefPtr<Gtk::Adjustment> range3, attack3, release3, threshold3, knee3, ratio3, makeup3;

  Gtk::LevelBar *output0 = nullptr, *output1 = nullptr, *output2 = nullptr, *output3 = nullptr;

  Gtk::Label *output0_label = nullptr, *output1_label = nullptr, *output2_label = nullptr, *output3_label = nullptr;

  Gtk::LevelBar *gating0 = nullptr, *gating1 = nullptr, *gating2 = nullptr, *gating3 = nullptr;

  Gtk::Label *gating0_label = nullptr, *gating1_label = nullptr, *gating2_label = nullptr, *gating3_label = nullptr;

  Gtk::ComboBoxText *mode = nullptr, *detection0 = nullptr, *detection1 = nullptr, *detection2 = nullptr,
                    *detection3 = nullptr;

  Gtk::ToggleButton *bypass0 = nullptr, *bypass1 = nullptr, *bypass2 = nullptr, *bypass3 = nullptr, *solo0 = nullptr,
                    *solo1 = nullptr, *solo2 = nullptr, *solo3 = nullptr;
};

#endif
