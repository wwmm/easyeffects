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

#ifndef BASS_LOUDNESS_UI_HPP
#define BASS_LOUDNESS_UI_HPP

#include "plugin_ui_base.hpp"

class BassLoudnessUi : public Gtk::Box, public PluginUiBase {
 public:
  BassLoudnessUi(BaseObjectType* cobject,
                 const Glib::RefPtr<Gtk::Builder>& builder,
                 const std::string& schema,
                 const std::string& schema_path);
  BassLoudnessUi(const BassLoudnessUi&) = delete;
  auto operator=(const BassLoudnessUi&) -> BassLoudnessUi& = delete;
  BassLoudnessUi(const BassLoudnessUi&&) = delete;
  auto operator=(const BassLoudnessUi&&) -> BassLoudnessUi& = delete;
  ~BassLoudnessUi() override;

  static auto add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> BassLoudnessUi*;

  void reset() override;

 private:
  Gtk::SpinButton *loudness = nullptr, *output = nullptr, *link = nullptr;
};

#endif
