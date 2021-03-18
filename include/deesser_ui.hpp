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

#ifndef DEESSER_UI_HPP
#define DEESSER_UI_HPP

#include "plugin_ui_base.hpp"

class DeesserUi : public Gtk::Box, public PluginUiBase {
 public:
  DeesserUi(BaseObjectType* cobject,
            const Glib::RefPtr<Gtk::Builder>& builder,
            const std::string& schema,
            const std::string& schema_path);
  DeesserUi(const DeesserUi&) = delete;
  auto operator=(const DeesserUi&) -> DeesserUi& = delete;
  DeesserUi(const DeesserUi&&) = delete;
  auto operator=(const DeesserUi&&) -> DeesserUi& = delete;
  ~DeesserUi() override;

  void on_new_compression(double value);
  void on_new_detected(double value);

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> threshold, ratio, makeup, f1_freq, f2_freq, f1_level, f2_level, f2_q, laxity;

  Gtk::LevelBar *compression = nullptr, *detected = nullptr;
  Gtk::Label *compression_label = nullptr, *detected_label = nullptr;
  Gtk::ComboBoxText *detection = nullptr, *mode = nullptr;
  Gtk::ToggleButton* sc_listen = nullptr;
};

#endif
