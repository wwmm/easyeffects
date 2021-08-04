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

#ifndef GATE_UI_HPP
#define GATE_UI_HPP

#include "plugin_ui_base.hpp"

class GateUi : public Gtk::Box, public PluginUiBase {
 public:
  GateUi(BaseObjectType* cobject,
         const Glib::RefPtr<Gtk::Builder>& builder,
         const std::string& schema,
         const std::string& schema_path);
  GateUi(const GateUi&) = delete;
  auto operator=(const GateUi&) -> GateUi& = delete;
  GateUi(const GateUi&&) = delete;
  auto operator=(const GateUi&&) -> GateUi& = delete;
  ~GateUi() override;

  static auto add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> GateUi*;

  void on_new_gating(double value);

  void reset() override;

 private:
  Gtk::SpinButton *attack = nullptr, *release = nullptr, *threshold = nullptr, *knee = nullptr, *ratio = nullptr,
                  *range = nullptr, *makeup = nullptr;

  Gtk::LevelBar* gating = nullptr;

  Gtk::Label* gating_label = nullptr;

  Gtk::ComboBoxText *detection = nullptr, *stereo_link = nullptr;

  Gtk::Scale *input_gain = nullptr, *output_gain = nullptr;
};

#endif
