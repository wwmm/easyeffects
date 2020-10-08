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

#ifndef COMPRESSOR_UI_HPP
#define COMPRESSOR_UI_HPP

#include "plugin_ui_base.hpp"

class CompressorUi : public Gtk::Grid, public PluginUiBase {
 public:
  CompressorUi(BaseObjectType* cobject,
               const Glib::RefPtr<Gtk::Builder>& builder,
               const std::string& schema,
               const std::string& schema_path);
  CompressorUi(const CompressorUi&) = delete;
  auto operator=(const CompressorUi&) -> CompressorUi& = delete;
  CompressorUi(const CompressorUi&&) = delete;
  auto operator=(const CompressorUi &&) -> CompressorUi& = delete;
  ~CompressorUi() override;

  void on_new_reduction(double value);

  void on_new_sidechain(double value);

  void on_new_curve(double value);

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> attack, release, release_threshold, threshold, knee, ratio, makeup, boost_threshold,
      preamp, reactivity, lookahead, input_gain, output_gain, hpf_freq, lpf_freq;

  Gtk::LevelBar *reduction = nullptr, *sidechain = nullptr, *curve = nullptr;

  Gtk::Label *reduction_label = nullptr, *sidechain_label = nullptr, *curve_label = nullptr;

  Gtk::ComboBoxText *compression_mode = nullptr, *sidechain_type = nullptr, *sidechain_mode = nullptr,
                    *sidechain_source = nullptr, *lpf_mode = nullptr, *hpf_mode = nullptr;

  Gtk::ToggleButton* listen = nullptr;
};

#endif
