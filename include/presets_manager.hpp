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

#ifndef PRESETS_MANAGER_HPP
#define PRESETS_MANAGER_HPP

#include <giomm.h>
#include <glibmm.h>
#include <sigc++/sigc++.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>
#include "autogain_preset.hpp"
#include "bass_enhancer_preset.hpp"
#include "bass_loudness_preset.hpp"
#include "compressor_preset.hpp"
#include "convolver_preset.hpp"
#include "crossfeed_preset.hpp"
#include "crystalizer_preset.hpp"
#include "deesser_preset.hpp"
#include "delay_preset.hpp"
#include "echo_canceller_preset.hpp"
#include "equalizer_preset.hpp"
#include "exciter_preset.hpp"
#include "filter_preset.hpp"
#include "gate_preset.hpp"
#include "giomm/file.h"
#include "giomm/filemonitor.h"
#include "limiter_preset.hpp"
#include "loudness_preset.hpp"
#include "maximizer_preset.hpp"
#include "multiband_compressor_preset.hpp"
#include "multiband_gate_preset.hpp"
#include "pitch_preset.hpp"
#include "plugin_name.hpp"
#include "preset_type.hpp"
#include "reverb_preset.hpp"
#include "rnnoise_preset.hpp"
#include "stereo_tools_preset.hpp"
#include "util.hpp"

class PresetsManager {
 public:
  PresetsManager();
  PresetsManager(const PresetsManager&) = delete;
  auto operator=(const PresetsManager&) -> PresetsManager& = delete;
  PresetsManager(const PresetsManager&&) = delete;
  auto operator=(const PresetsManager&&) -> PresetsManager& = delete;
  ~PresetsManager();

  auto get_names(PresetType preset_type) -> std::vector<Glib::ustring>;

  static auto search_names(std::filesystem::directory_iterator& it) -> std::vector<std::string>;

  void add(PresetType preset_type, const Glib::ustring& name);

  void save_preset_file(PresetType preset_type, const std::string& name);

  void remove(PresetType preset_type, const std::string& name);

  void load_preset_file(PresetType preset_type, const std::string& name);

  void import(PresetType preset_type, const std::string& file_path);

  void add_autoload(PresetType preset_type,
                    const std::string& preset_name,
                    const std::string& device_name,
                    const std::string& device_profile);

  void remove_autoload(PresetType preset_type,
                       const std::string& preset_name,
                       const std::string& device_name,
                       const std::string& device_profile);

  auto find_autoload(PresetType preset_type, const std::string& device_name, const std::string& device_profile)
      -> std::string;

  void autoload(PresetType preset_type, const std::string& device_name, const std::string& device_profile);

  auto get_autoload_profiles(PresetType preset_type) -> std::vector<nlohmann::json>;

  auto preset_file_exists(PresetType preset_type, const std::string& name) -> bool;

  sigc::signal<void(const Glib::RefPtr<Gio::File>& file)> user_output_preset_created;
  sigc::signal<void(const Glib::RefPtr<Gio::File>& file)> user_output_preset_removed;
  sigc::signal<void(const Glib::RefPtr<Gio::File>& file)> user_input_preset_created;
  sigc::signal<void(const Glib::RefPtr<Gio::File>& file)> user_input_preset_removed;

  sigc::signal<void(const std::vector<nlohmann::json>& profiles)> autoload_input_profiles_changed;
  sigc::signal<void(const std::vector<nlohmann::json>& profiles)> autoload_output_profiles_changed;

 private:
  std::string log_tag = "presets_manager: ";

  std::filesystem::path user_presets_dir, user_input_dir, user_output_dir, autoload_input_dir, autoload_output_dir;

  std::vector<std::filesystem::path> system_input_dir, system_output_dir;

  Glib::RefPtr<Gio::Settings> settings, soe_settings, sie_settings;

  Glib::RefPtr<Gio::FileMonitor> user_output_monitor, user_input_monitor;

  Glib::RefPtr<Gio::FileMonitor> autoload_output_monitor, autoload_input_monitor;

  std::unique_ptr<AutoGainPreset> autogain;
  std::unique_ptr<BassEnhancerPreset> bass_enhancer;
  std::unique_ptr<BassLoudnessPreset> bass_loudness;
  std::unique_ptr<CompressorPreset> compressor;
  std::unique_ptr<ConvolverPreset> convolver;
  std::unique_ptr<CrossfeedPreset> crossfeed;
  std::unique_ptr<CrystalizerPreset> crystalizer;
  std::unique_ptr<DeesserPreset> deesser;
  std::unique_ptr<DelayPreset> delay;
  std::unique_ptr<EchoCancellerPreset> echo_canceller;
  std::unique_ptr<EqualizerPreset> equalizer;
  std::unique_ptr<ExciterPreset> exciter;
  std::unique_ptr<FilterPreset> filter;
  std::unique_ptr<GatePreset> gate;
  std::unique_ptr<LimiterPreset> limiter;
  std::unique_ptr<LoudnessPreset> loudness;
  std::unique_ptr<MaximizerPreset> maximizer;
  std::unique_ptr<MultibandCompressorPreset> multiband_compressor;
  std::unique_ptr<MultibandGatePreset> multiband_gate;
  std::unique_ptr<PitchPreset> pitch;
  std::unique_ptr<ReverbPreset> reverb;
  std::unique_ptr<RNNoisePreset> rnnoise;
  std::unique_ptr<StereoToolsPreset> stereo_tools;

  void create_user_directory(const std::filesystem::path& path);

  void save_blocklist(PresetType preset_type, nlohmann::json& json);

  void load_blocklist(PresetType preset_type, const nlohmann::json& json);
};

#endif
