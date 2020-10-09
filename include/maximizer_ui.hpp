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

#ifndef MAXIMIZER_UI_HPP
#define MAXIMIZER_UI_HPP

#include "plugin_ui_base.hpp"

class MaximizerUi : public Gtk::Grid, public PluginUiBase {
 public:
  MaximizerUi(BaseObjectType* cobject,
              const Glib::RefPtr<Gtk::Builder>& builder,
              const std::string& schema,
              const std::string& schema_path);
  MaximizerUi(const MaximizerUi&) = delete;
  auto operator=(const MaximizerUi&) -> MaximizerUi& = delete;
  MaximizerUi(const MaximizerUi&&) = delete;
  auto operator=(const MaximizerUi &&) -> MaximizerUi& = delete;
  ~MaximizerUi() override;

  void on_new_reduction(double value);

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> release, threshold, ceiling;
  Gtk::LevelBar* reduction = nullptr;
  Gtk::Label* reduction_label = nullptr;
};

#endif
