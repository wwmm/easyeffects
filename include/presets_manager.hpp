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

#ifndef PRESETS_MANAGER_HPP
#define PRESETS_MANAGER_HPP

#include <giomm.h>
#include <glibmm.h>
#include <sigc++/sigc++.h>
#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include "autogain_preset.hpp"
#include "bass_enhancer_preset.hpp"
#include "compressor_preset.hpp"
#include "convolver_preset.hpp"
#include "crossfeed_preset.hpp"
#include "crystalizer_preset.hpp"
#include "deesser_preset.hpp"
#include "delay_preset.hpp"
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
#include "preset_type.hpp"
#include "reverb_preset.hpp"
#include "rnnoise_preset.hpp"
#include "spectrum_preset.hpp"
#include "stereo_tools_preset.hpp"
#include "util.hpp"
#include "webrtc_preset.hpp"

class PresetsManager {
 public:
  PresetsManager();
  PresetsManager(const PresetsManager&) = delete;
  auto operator=(const PresetsManager&) -> PresetsManager& = delete;
  PresetsManager(const PresetsManager&&) = delete;
  auto operator=(const PresetsManager&&) -> PresetsManager& = delete;
  ~PresetsManager();

  auto get_names(PresetType preset_type) -> std::vector<Glib::ustring>;
  static auto search_names(boost::filesystem::directory_iterator& it) -> std::vector<std::string>;
  void add(PresetType preset_type, const Glib::ustring& name);
  void save(PresetType preset_type, const std::string& name);
  void remove(PresetType preset_type, const std::string& name);
  void load(PresetType preset_type, const std::string& name);
  void import(PresetType preset_type, const std::string& file_path);
  void add_autoload(const std::string& device, const std::string& name);
  void remove_autoload(const std::string& device, const std::string& name);
  auto find_autoload(const std::string& device) -> std::string;
  void autoload(PresetType preset_type, const std::string& device);
  auto preset_file_exists(PresetType preset_type, const std::string& name) -> bool;

  sigc::signal<void(const Glib::RefPtr<Gio::File>& file)> user_output_preset_created;
  sigc::signal<void(const Glib::RefPtr<Gio::File>& file)> user_output_preset_removed;
  sigc::signal<void(const Glib::RefPtr<Gio::File>& file)> user_input_preset_created;
  sigc::signal<void(const Glib::RefPtr<Gio::File>& file)> user_input_preset_removed;

 private:
  std::string log_tag = "presets_manager: ";

  boost::filesystem::path user_presets_dir, user_input_dir, user_output_dir, autoload_dir;

  std::vector<boost::filesystem::path> system_input_dir, system_output_dir;

  Glib::RefPtr<Gio::Settings> settings, sie_settings, soe_settings;

  Glib::RefPtr<Gio::FileMonitor> user_output_monitor, user_input_monitor;

  std::unique_ptr<LimiterPreset> limiter;
  std::unique_ptr<BassEnhancerPreset> bass_enhancer;
  std::unique_ptr<CompressorPreset> compressor;
  std::unique_ptr<CrossfeedPreset> crossfeed;
  std::unique_ptr<DeesserPreset> deesser;
  std::unique_ptr<EqualizerPreset> equalizer;
  std::unique_ptr<ExciterPreset> exciter;
  std::unique_ptr<FilterPreset> filter;
  std::unique_ptr<GatePreset> gate;
  std::unique_ptr<MaximizerPreset> maximizer;
  std::unique_ptr<PitchPreset> pitch;
  std::unique_ptr<ReverbPreset> reverb;
  std::unique_ptr<WebrtcPreset> webrtc;
  std::unique_ptr<MultibandCompressorPreset> multiband_compressor;
  std::unique_ptr<LoudnessPreset> loudness;
  std::unique_ptr<MultibandGatePreset> multiband_gate;
  std::unique_ptr<StereoToolsPreset> stereo_tools;
  std::unique_ptr<ConvolverPreset> convolver;
  std::unique_ptr<CrystalizerPreset> crystalizer;
  std::unique_ptr<AutoGainPreset> autogain;
  std::unique_ptr<DelayPreset> delay;
  std::unique_ptr<RNNoisePreset> rnnoise;
  std::unique_ptr<SpectrumPreset> spectrum;

  template <typename T>
  auto get_default(const Glib::RefPtr<Gio::Settings>& settings, const std::string& key) -> T {
    Glib::Variant<T> value;

    settings->get_default_value(key, value);

    return value.get();
  }

  template <typename T>
  void update_key(const boost::property_tree::ptree& root,
                  const Glib::RefPtr<Gio::Settings>& settings,
                  const std::string& key,
                  const std::string& json_key) {
    Glib::Variant<T> aux;

    settings->get_value(key, aux);

    T current_value = aux.get();

    T new_value = root.get<T>(json_key, get_default<T>(settings, key));

    if (is_different(current_value, new_value)) {
      auto v = Glib::Variant<T>::create(new_value);

      settings->set_value(key, v);
    }
  }

  void update_string_key(const boost::property_tree::ptree& root,
                         const Glib::RefPtr<Gio::Settings>& settings,
                         const std::string& key,
                         const std::string& json_key) {
    std::string current_value = settings->get_string(key);

    std::string new_value = root.get<std::string>(json_key, get_default<std::string>(settings, key));

    if (current_value != new_value) {
      settings->set_string(key, new_value);
    }
  }

  template <typename T>
  auto is_different(const T& a, const T& b) -> bool {
    return a != b;
  }

  void create_user_directory(const boost::filesystem::path& path);

  void save_blocklist(PresetType preset_type, boost::property_tree::ptree& root);

  void load_blocklist(PresetType preset_type, const boost::property_tree::ptree& root);
};

#endif
