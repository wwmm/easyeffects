#ifndef STEREO_TOOLS_UI_HPP
#define STEREO_TOOLS_UI_HPP

#include <gtkmm/button.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include <gtkmm/togglebutton.h>
#include "plugin_ui_base.hpp"

class StereoToolsUi : public Gtk::Grid, public PluginUiBase {
 public:
  StereoToolsUi(BaseObjectType* cobject,
                const Glib::RefPtr<Gtk::Builder>& builder,
                const std::string& schema,
                const std::string& schema_path);
  StereoToolsUi(const StereoToolsUi&) = delete;
  auto operator=(const StereoToolsUi&) -> StereoToolsUi& = delete;
  StereoToolsUi(const StereoToolsUi&&) = delete;
  auto operator=(const StereoToolsUi &&) -> StereoToolsUi& = delete;
  ~StereoToolsUi() override;

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, balance_in, balance_out, slev, sbal, mlev, mpan, stereo_base,
      delay, sc_level, stereo_phase;

  Gtk::ComboBoxText* mode = nullptr;

  Gtk::ToggleButton *softclip = nullptr, *mutel = nullptr, *muter = nullptr, *phasel = nullptr, *phaser = nullptr;
};

#endif
