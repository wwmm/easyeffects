#ifndef REVERB_UI_HPP
#define REVERB_UI_HPP

#include <gtkmm/button.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class ReverbUi : public Gtk::Grid, public PluginUiBase {
 public:
  ReverbUi(BaseObjectType* cobject,
           const Glib::RefPtr<Gtk::Builder>& builder,
           const std::string& schema,
           const std::string& schema_path);
  ReverbUi(const ReverbUi&) = delete;
  auto operator=(const ReverbUi&) -> ReverbUi& = delete;
  ReverbUi(const ReverbUi&&) = delete;
  auto operator=(const ReverbUi &&) -> ReverbUi& = delete;
  ~ReverbUi() override;

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, predelay, decay_time, diffusion, amount, dry, hf_damp,
      bass_cut, treble_cut;

  Gtk::ComboBoxText* room_size = nullptr;

  Gtk::Button *preset_room = nullptr, *preset_empty_walls = nullptr, *preset_ambience = nullptr,
              *preset_large_empty_hall = nullptr, *preset_disco = nullptr, *preset_large_occupied_hall = nullptr,
              *preset_default = nullptr;

  void init_presets_buttons();
};

#endif
