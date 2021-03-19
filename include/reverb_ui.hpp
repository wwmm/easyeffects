/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef REVERB_UI_HPP
#define REVERB_UI_HPP

#include "plugin_ui_base.hpp"

class ReverbUi : public Gtk::Box, public PluginUiBase {
 public:
  ReverbUi(BaseObjectType* cobject,
           const Glib::RefPtr<Gtk::Builder>& builder,
           const std::string& schema,
           const std::string& schema_path);
  ReverbUi(const ReverbUi&) = delete;
  auto operator=(const ReverbUi&) -> ReverbUi& = delete;
  ReverbUi(const ReverbUi&&) = delete;
  auto operator=(const ReverbUi&&) -> ReverbUi& = delete;
  ~ReverbUi() override;

  static auto add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> ReverbUi*;

  void reset() override;

 private:
  Gtk::SpinButton *predelay = nullptr, *decay_time = nullptr, *diffusion = nullptr, *amount = nullptr, *dry = nullptr,
                  *hf_damp = nullptr, *bass_cut = nullptr, *treble_cut = nullptr;

  Gtk::ComboBoxText* room_size = nullptr;

  Gtk::Button *preset_room = nullptr, *preset_empty_walls = nullptr, *preset_ambience = nullptr,
              *preset_large_empty_hall = nullptr, *preset_disco = nullptr, *preset_large_occupied_hall = nullptr;

  Gtk::Scale *input_gain = nullptr, *output_gain = nullptr;

  void init_presets_buttons();
};

#endif
