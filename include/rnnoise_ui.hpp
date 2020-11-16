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

#ifndef RNNOISE_UI_HPP
#define RNNOISE_UI_HPP

#include "plugin_ui_base.hpp"

class RNNoiseUi : public Gtk::Grid, public PluginUiBase {
 public:
  RNNoiseUi(BaseObjectType* cobject,
            const Glib::RefPtr<Gtk::Builder>& builder,
            const std::string& schema,
            const std::string& schema_path);
  RNNoiseUi(const RNNoiseUi&) = delete;
  auto operator=(const RNNoiseUi&) -> RNNoiseUi& = delete;
  RNNoiseUi(const RNNoiseUi&&) = delete;
  auto operator=(const RNNoiseUi&&) -> RNNoiseUi& = delete;
  ~RNNoiseUi() override;

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain;
};

#endif
