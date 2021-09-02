/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef BASS_ENHANCER_UI_HPP
#define BASS_ENHANCER_UI_HPP

#include "plugin_ui_base.hpp"

class BassEnhancerUi : public Gtk::Box, public PluginUiBase {
 public:
  BassEnhancerUi(BaseObjectType* cobject,
                 const Glib::RefPtr<Gtk::Builder>& builder,
                 const std::string& schema,
                 const std::string& schema_path);
  BassEnhancerUi(const BassEnhancerUi&) = delete;
  auto operator=(const BassEnhancerUi&) -> BassEnhancerUi& = delete;
  BassEnhancerUi(const BassEnhancerUi&&) = delete;
  auto operator=(const BassEnhancerUi&&) -> BassEnhancerUi& = delete;
  ~BassEnhancerUi() override;

  static auto add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> BassEnhancerUi*;

  void on_new_harmonics_level(const double& value);

  void reset() override;

 private:
  Gtk::LevelBar* harmonics_levelbar = nullptr;

  Gtk::Label* harmonics_levelbar_label = nullptr;

  Gtk::SpinButton *floor = nullptr, *amount = nullptr, *harmonics = nullptr, *scope = nullptr;

  Gtk::Scale *input_gain = nullptr, *output_gain = nullptr, *blend = nullptr;

  Gtk::ToggleButton *floor_active = nullptr, *listen = nullptr;
};

#endif
