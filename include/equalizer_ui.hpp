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

#ifndef EQUALIZER_UI_HPP
#define EQUALIZER_UI_HPP

#include <glibmm/i18n.h>
#include <filesystem>
#include <fstream>
#include <regex>
#include <unordered_map>
#include "plugin_ui_base.hpp"

enum FilterType : const unsigned int {
  PEAKING = 1U,
  LOW_PASS = 1U << 1U,
  LOW_PASS_Q = 1U << 2U,
  HIGH_PASS = 1U << 3U,
  HIGH_PASS_Q = 1U << 4U,
  BAND_PASS = 1U << 5U,
  LOW_SHELF = 1U << 6U,
  LOW_SHELF_xdB = 1U << 7U,
  HIGH_SHELF = 1U << 8U,
  HIGH_SHELF_xdB = 1U << 9U,
  NOTCH = 1U << 10U,
  ALL_PASS = 1U << 11U
};

struct ImportedBand {
  unsigned int type;
  float freq;
  float gain;
  float quality_factor;
  float slope_dB;
};

class EqualizerUi : public Gtk::Box, public PluginUiBase {
 public:
  EqualizerUi(BaseObjectType* cobject,
              const Glib::RefPtr<Gtk::Builder>& builder,
              const std::string& schema,
              const std::string& schema_path,
              const std::string& schema_channel,
              const std::string& schema_channel_left_path,
              const std::string& schema_channel_right_path);
  EqualizerUi(const EqualizerUi&) = delete;
  auto operator=(const EqualizerUi&) -> EqualizerUi& = delete;
  EqualizerUi(const EqualizerUi&&) = delete;
  auto operator=(const EqualizerUi&&) -> EqualizerUi& = delete;
  ~EqualizerUi() override;

  static auto add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> EqualizerUi*;

  void reset() override;

 private:
  int max_bands = 32;

  Glib::RefPtr<Gio::Settings> settings_left, settings_right;

  Gtk::SpinButton *nbands = nullptr, *resonance = nullptr, *inertia = nullptr;

  Gtk::Box *bands_box_left = nullptr, *bands_box_right = nullptr;

  Gtk::Button *flat_response = nullptr, *calculate_freqs = nullptr, *import_apo = nullptr;

  Gtk::ToggleButton* split_channels = nullptr;

  Gtk::Stack* stack = nullptr;

  Gtk::StackSwitcher* stack_switcher = nullptr;

  Gtk::ComboBoxText* mode = nullptr;

  std::vector<sigc::connection> connections_bands;

  void on_nbands_changed();

  void build_bands(Gtk::Box* bands_box,
                   const Glib::RefPtr<Gio::Settings>& cfg,
                   const int& nbands,
                   const bool& split_mode);

  void on_flat_response();

  void on_calculate_frequencies();

  void on_import_apo_preset_clicked();

  bool parse_apo_preamp(const std::string& line, double &preamp);

  auto parse_apo_filter(const std::string& line, struct ImportedBand& filter) -> bool;

  void import_apo_preset(const std::string& file_path);

  std::unordered_map<std::string, FilterType> const FilterTypeMap = {
      {"PK", FilterType::PEAKING},         {"LP", FilterType::LOW_PASS},       {"LPQ", FilterType::LOW_PASS_Q},
      {"HP", FilterType::HIGH_PASS},       {"HPQ", FilterType::HIGH_PASS_Q},   {"BP", FilterType::BAND_PASS},
      {"LS", FilterType::LOW_SHELF},       {"LSC", FilterType::LOW_SHELF_xdB}, {"HS", FilterType::HIGH_SHELF},
      {"HSC", FilterType::HIGH_SHELF_xdB}, {"NO", FilterType::NOTCH},          {"AP", FilterType::ALL_PASS}};
};

#endif
