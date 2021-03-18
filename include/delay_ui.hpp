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

#ifndef DELAY_UI_HPP
#define DELAY_UI_HPP

#include "plugin_ui_base.hpp"

class DelayUi : public Gtk::Box, public PluginUiBase {
 public:
  DelayUi(BaseObjectType* cobject,
          const Glib::RefPtr<Gtk::Builder>& builder,
          const std::string& schema,
          const std::string& schema_path);
  DelayUi(const DelayUi&) = delete;
  auto operator=(const DelayUi&) -> DelayUi& = delete;
  DelayUi(const DelayUi&&) = delete;
  auto operator=(const DelayUi&&) -> DelayUi& = delete;
  ~DelayUi() override;

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, time_l, time_r;
};

#endif
