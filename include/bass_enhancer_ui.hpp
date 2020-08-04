#ifndef BASS_ENHANCER_UI_HPP
#define BASS_ENHANCER_UI_HPP

#include "plugin_ui_base.hpp"

class BassEnhancerUi : public Gtk::Grid, public PluginUiBase {
 public:
  BassEnhancerUi(BaseObjectType* cobject,
                 const Glib::RefPtr<Gtk::Builder>& builder,
                 const std::string& schema,
                 const std::string& schema_path);
  BassEnhancerUi(const BassEnhancerUi&) = delete;
  auto operator=(const BassEnhancerUi&) -> BassEnhancerUi& = delete;
  BassEnhancerUi(const BassEnhancerUi&&) = delete;
  auto operator=(const BassEnhancerUi &&) -> BassEnhancerUi& = delete;
  ~BassEnhancerUi() override;

  void on_new_harmonics_level(double value);

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> amount, blend, floorv, harmonics, input_gain, output_gain, scope;

  Gtk::LevelBar* harmonics_levelbar = nullptr;
  Gtk::Label* harmonics_levelbar_label = nullptr;
  Gtk::ToggleButton *floor_active = nullptr, *listen = nullptr;
};

#endif
