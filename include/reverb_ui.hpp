#ifndef REVERB_UI_HPP
#define REVERB_UI_HPP

#include <gtkmm/button.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class ReverbUi : public Gtk::Grid, public PluginUiBase {
 public:
  ReverbUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name);
  virtual ~ReverbUi();

 private:
  Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, predelay, decay_time, diffusion, amount, dry, hf_damp,
      bass_cut, treble_cut;
  Gtk::ComboBoxText* room_size;
  Gtk::Button *preset_room, *preset_empty_walls, *preset_ambience, *preset_large_empty_hall, *preset_disco,
      *preset_large_occupied_hall, *preset_default;

  void init_presets_buttons();
};

#endif
