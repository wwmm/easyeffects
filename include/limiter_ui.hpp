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

#ifndef LIMITER_UI_HPP
#define LIMITER_UI_HPP

#include "plugin_ui_base.hpp"

class LimiterUi : public Gtk::Box, public PluginUiBase {
 public:
  LimiterUi(BaseObjectType* cobject,
            const Glib::RefPtr<Gtk::Builder>& builder,
            const std::string& schema,
            const std::string& schema_path);
  LimiterUi(const LimiterUi&) = delete;
  auto operator=(const LimiterUi&) -> LimiterUi& = delete;
  LimiterUi(const LimiterUi&&) = delete;
  auto operator=(const LimiterUi&&) -> LimiterUi& = delete;
  ~LimiterUi() override;

  static auto add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> LimiterUi*;

  void on_new_attenuation(double value);

  void reset() override;

 private:
  Gtk::SpinButton *lookahead = nullptr, *release = nullptr, *limit = nullptr, *oversampling = nullptr,
                  *asc_level = nullptr;

  Gtk::Scale *input_gain = nullptr, *output_gain = nullptr;

  Gtk::ToggleButton *auto_level = nullptr, *asc = nullptr;

  Gtk::LevelBar* attenuation = nullptr;

  Gtk::Label* attenuation_label = nullptr;
};

#endif
