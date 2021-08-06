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

#ifndef LIMITER_UI_HPP
#define LIMITER_UI_HPP

#include <cstring>
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
  Gtk::ComboBoxText *mode = nullptr, *oversampling = nullptr, *dither = nullptr;

  Gtk::SpinButton *sc_preamp = nullptr, *lookahead = nullptr, *attack = nullptr, *release = nullptr,
      *threshold = nullptr, *stereo_link = nullptr, *alr_attack = nullptr, *alr_release = nullptr,
      *alr_knee = nullptr;

  Gtk::CheckButton* boost = nullptr;

  Gtk::ToggleButton* alr = nullptr;

  Gtk::Label *gain_left = nullptr, *gain_right = nullptr, *sidechain_left = nullptr, *sidechain_right = nullptr;

  Gtk::Scale *input_gain = nullptr, *output_gain = nullptr;
};

#endif
