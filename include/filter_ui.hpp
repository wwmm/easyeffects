/*
 *  Copyright © 2017-2020 Wellington Wallace
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

#ifndef FILTER_UI_HPP
#define FILTER_UI_HPP

#include "plugin_ui_base.hpp"

class FilterUi : public Gtk::Box, public PluginUiBase {
 public:
  FilterUi(BaseObjectType* cobject,
           const Glib::RefPtr<Gtk::Builder>& builder,
           const std::string& schema,
           const std::string& schema_path);
  FilterUi(const FilterUi&) = delete;
  auto operator=(const FilterUi&) -> FilterUi& = delete;
  FilterUi(const FilterUi&&) = delete;
  auto operator=(const FilterUi&&) -> FilterUi& = delete;
  ~FilterUi() override;

  static auto add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> FilterUi*;

  void reset() override;

 private:
  Gtk::ComboBoxText* mode = nullptr;

  Gtk::SpinButton *frequency = nullptr, *resonance = nullptr, *inertia = nullptr;

  Gtk::Scale *input_gain = nullptr, *output_gain = nullptr;
};

#endif
