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

#ifndef EXCITER_UI_HPP
#define EXCITER_UI_HPP

#include "plugin_ui_base.hpp"

class ExciterUi : public Gtk::Box, public PluginUiBase {
 public:
  ExciterUi(BaseObjectType* cobject,
            const Glib::RefPtr<Gtk::Builder>& builder,
            const std::string& schema,
            const std::string& schema_path);
  ExciterUi(const ExciterUi&) = delete;
  auto operator=(const ExciterUi&) -> ExciterUi& = delete;
  ExciterUi(const ExciterUi&&) = delete;
  auto operator=(const ExciterUi&&) -> ExciterUi& = delete;
  ~ExciterUi() override;

  static auto add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> ExciterUi*;

  void on_new_harmonics_level(double value);

  void reset() override;

 private:
  Gtk::LevelBar* harmonics_levelbar = nullptr;

  Gtk::Label* harmonics_levelbar_label = nullptr;

  Gtk::SpinButton *ceil = nullptr, *amount = nullptr, *harmonics = nullptr, *scope = nullptr;

  Gtk::Scale *input_gain = nullptr, *output_gain = nullptr, *blend = nullptr;

  Gtk::ToggleButton *ceil_active = nullptr, *listen = nullptr;
};

#endif
