#ifndef CRYSTALIZER_UI_HPP
#define CRYSTALIZER_UI_HPP

#include <gtkmm/grid.h>
#include <gtkmm/togglebutton.h>
#include "plugin_ui_base.hpp"

class CrystalizerUi : public Gtk::Grid, public PluginUiBase {
 public:
  CrystalizerUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name);
  CrystalizerUi(const CrystalizerUi&) = delete;
  auto operator=(const CrystalizerUi&) -> CrystalizerUi& = delete;
  CrystalizerUi(const CrystalizerUi&&) = delete;
  auto operator=(const CrystalizerUi &&) -> CrystalizerUi& = delete;
  ~CrystalizerUi() override;

  void on_new_range_before(double value);

  void on_new_range_after(double value);

  void reset() override;

 private:
  Gtk::Grid* bands_grid = nullptr;

  Gtk::LevelBar *range_before = nullptr, *range_after = nullptr;
  Gtk::Label *range_before_label = nullptr, *range_after_label = nullptr;
  Gtk::ToggleButton* aggressive = nullptr;

  Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain;

  void build_bands(const int& nbands);
};

#endif
