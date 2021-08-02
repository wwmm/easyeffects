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

  static constexpr uint n_bands = 8U;

  static auto add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> MultibandCompressorUi*;

  void on_new_frequency_range(std::array<double, n_bands>);

  void on_new_envelope(std::array<double, n_bands>);

  void on_new_curve(std::array<double, n_bands>);

  void on_new_reduction(std::array<double, n_bands>);

  void reset() override;

 private:
  Gtk::ComboBoxText *compressor_mode = nullptr, *envelope_boost = nullptr;

  Gtk::Stack* stack = nullptr;

  Gtk::ListBox* listbox = nullptr;

  Gtk::Scale *input_gain = nullptr, *output_gain = nullptr;

  std::array<Gtk::Label*, n_bands> bands_end, bands_gain_label, bands_envelope_label, bands_curve_label;

  std::array<Gtk::LevelBar*, n_bands> bands_gain, bands_envelope, bands_curve;

  void prepare_bands();
};

#endif
