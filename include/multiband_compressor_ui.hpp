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

#ifndef MULTIBAND_COMPRESSOR_UI_HPP
#define MULTIBAND_COMPRESSOR_UI_HPP

#include <cstring>
#include "plugin_ui_base.hpp"

class MultibandCompressorUi : public Gtk::Box, public PluginUiBase {
 public:
  MultibandCompressorUi(BaseObjectType* cobject,
                        const Glib::RefPtr<Gtk::Builder>& builder,
                        const std::string& schema,
                        const std::string& schema_path);
  MultibandCompressorUi(const MultibandCompressorUi&) = delete;
  auto operator=(const MultibandCompressorUi&) -> MultibandCompressorUi& = delete;
  MultibandCompressorUi(const MultibandCompressorUi&&) = delete;
  auto operator=(const MultibandCompressorUi&&) -> MultibandCompressorUi& = delete;
  ~MultibandCompressorUi() override;

  static auto add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> MultibandCompressorUi*;

  void on_new_output0(double value);
  void on_new_output1(double value);
  void on_new_output2(double value);
  void on_new_output3(double value);

  void on_new_compression0(double value);
  void on_new_compression1(double value);
  void on_new_compression2(double value);
  void on_new_compression3(double value);

  void reset() override;

 private:
  Gtk::SpinButton *freq0 = nullptr, *freq1 = nullptr, *freq2 = nullptr;

  Gtk::SpinButton *attack0 = nullptr, *release0 = nullptr, *threshold0 = nullptr, *knee0 = nullptr, *ratio0 = nullptr,
                  *makeup0 = nullptr;

  Gtk::SpinButton *attack1 = nullptr, *release1 = nullptr, *threshold1 = nullptr, *knee1 = nullptr, *ratio1 = nullptr,
                  *makeup1 = nullptr;

  Gtk::SpinButton *attack2 = nullptr, *release2 = nullptr, *threshold2 = nullptr, *knee2 = nullptr, *ratio2 = nullptr,
                  *makeup2 = nullptr;

  Gtk::SpinButton *attack3 = nullptr, *release3 = nullptr, *threshold3 = nullptr, *knee3 = nullptr, *ratio3 = nullptr,
                  *makeup3 = nullptr;

  Gtk::Scale *input_gain = nullptr, *output_gain = nullptr;

  Gtk::LevelBar *output0 = nullptr, *output1 = nullptr, *output2 = nullptr, *output3 = nullptr;

  Gtk::Label *output0_label = nullptr, *output1_label = nullptr, *output2_label = nullptr, *output3_label = nullptr;

  Gtk::LevelBar *compression0 = nullptr, *compression1 = nullptr, *compression2 = nullptr, *compression3 = nullptr;

  Gtk::Label *compression0_label = nullptr, *compression1_label = nullptr, *compression2_label = nullptr,
             *compression3_label = nullptr;

  Gtk::ComboBoxText *mode = nullptr, *detection0 = nullptr, *detection1 = nullptr, *detection2 = nullptr,
                    *detection3 = nullptr;

  Gtk::ToggleButton *bypass0 = nullptr, *bypass1 = nullptr, *bypass2 = nullptr, *bypass3 = nullptr, *solo0 = nullptr,
                    *solo1 = nullptr, *solo2 = nullptr, *solo3 = nullptr;
};

#endif
