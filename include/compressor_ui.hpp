#ifndef COMPRESSOR_UI_HPP
#define COMPRESSOR_UI_HPP

#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include <gtkmm/togglebutton.h>
#include "plugin_ui_base.hpp"

class CompressorUi : public Gtk::Grid, public PluginUiBase {
 public:
  CompressorUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name);
  CompressorUi(const CompressorUi&) = delete;
  auto operator=(const CompressorUi&) -> CompressorUi& = delete;
  CompressorUi(const CompressorUi&&) = delete;
  auto operator=(const CompressorUi &&) -> CompressorUi& = delete;
  ~CompressorUi() override;

  void on_new_reduction(double value);

  void on_new_sidechain(double value);

  void on_new_curve(double value);

 private:
  Glib::RefPtr<Gtk::Adjustment> attack, release, threshold, knee, ratio, makeup, preamp, reactivity, lookahead,
      input_gain, output_gain;

  Gtk::LevelBar *reduction = nullptr, *sidechain = nullptr, *curve = nullptr;

  Gtk::Label *reduction_label = nullptr, *sidechain_label = nullptr, *curve_label = nullptr;

  Gtk::ComboBoxText *compression_mode = nullptr, *sidechain_type = nullptr, *sidechain_mode = nullptr,
                    *sidechain_source = nullptr;

  Gtk::ToggleButton* listen = nullptr;
};

#endif
