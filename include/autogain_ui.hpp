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

#ifndef AUTOGAIN_UI_HPP
#define AUTOGAIN_UI_HPP

#include "plugin_ui_base.hpp"

class AutoGainUi : public Gtk::Grid, public PluginUiBase {
 public:
  AutoGainUi(BaseObjectType* cobject,
             const Glib::RefPtr<Gtk::Builder>& builder,
             const std::string& schema,
             const std::string& schema_path);
  AutoGainUi(const AutoGainUi&) = delete;
  auto operator=(const AutoGainUi&) -> AutoGainUi& = delete;
  AutoGainUi(const AutoGainUi&&) = delete;
  auto operator=(const AutoGainUi &&) -> AutoGainUi& = delete;
  ~AutoGainUi() override;

  void on_new_momentary(const float& value);
  void on_new_shortterm(const float& value);
  void on_new_integrated(const float& value);
  void on_new_relative(const float& value);
  void on_new_loudness(const float& value);
  void on_new_range(const float& value);
  void on_new_gain(const float& value);

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, target, weight_m, weight_s, weight_i;

  Gtk::LevelBar *m_level = nullptr, *s_level = nullptr, *i_level = nullptr, *r_level = nullptr, *g_level = nullptr,
                *l_level = nullptr, *lra_level = nullptr;

  Gtk::Label *m_label = nullptr, *s_label = nullptr, *i_label = nullptr, *r_label = nullptr, *g_label = nullptr,
             *l_label = nullptr, *lra_label = nullptr;

  Gtk::Button* reset_history = nullptr;

  Gtk::ToggleButton *detect_silence = nullptr, *use_geometric_mean = nullptr;

  Gtk::Grid *weight_m_grid = nullptr, *weight_s_grid = nullptr, *weight_i_grid = nullptr;
};

#endif
