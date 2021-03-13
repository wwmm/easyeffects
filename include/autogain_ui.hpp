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

class AutoGainUi : public Gtk::Box, public PluginUiBase {
 public:
  AutoGainUi(BaseObjectType* cobject,
             const Glib::RefPtr<Gtk::Builder>& builder,
             const std::string& schema,
             const std::string& schema_path);
  AutoGainUi(const AutoGainUi&) = delete;
  auto operator=(const AutoGainUi&) -> AutoGainUi& = delete;
  AutoGainUi(const AutoGainUi&&) = delete;
  auto operator=(const AutoGainUi&&) -> AutoGainUi& = delete;
  ~AutoGainUi() override;

  static auto add_to_stack(Gtk::Stack* stack) -> AutoGainUi*;

  void on_new_results(const double& loudness,
                      const double& gain,
                      const double& momentary,
                      const double& shortterm,
                      const double& integrated,
                      const double& relative,
                      const double& range);

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, target;

  Gtk::LevelBar *m_level = nullptr, *s_level = nullptr, *i_level = nullptr, *r_level = nullptr, *g_level = nullptr,
                *l_level = nullptr, *lra_level = nullptr;

  Gtk::Label *m_label = nullptr, *s_label = nullptr, *i_label = nullptr, *r_label = nullptr, *g_label = nullptr,
             *l_label = nullptr, *lra_label = nullptr;

  Gtk::Button* reset_history = nullptr;
};

#endif
