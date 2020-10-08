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

#include "presets_manager.hpp"
#include <glibmm.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include "util.hpp"

PresetsManager::PresetsManager()
    : user_presets_dir(Glib::get_user_config_dir() + "/PulseEffects"),
      user_input_dir(Glib::get_user_config_dir() + "/PulseEffects/input"),
      user_output_dir(Glib::get_user_config_dir() + "/PulseEffects/output"),
      autoload_dir(Glib::get_user_config_dir() + "/PulseEffects/autoload"),
      settings(Gio::Settings::create("com.github.wwmm.pulseeffects")),
      sie_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs")),
      soe_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sourceoutputs")),
      limiter(std::make_unique<LimiterPreset>()),
      bass_enhancer(std::make_unique<BassEnhancerPreset>()),
      compressor(std::make_unique<CompressorPreset>()),
      crossfeed(std::make_unique<CrossfeedPreset>()),
      deesser(std::make_unique<DeesserPreset>()),
      equalizer(std::make_unique<EqualizerPreset>()),
      exciter(std::make_unique<ExciterPreset>()),
      filter(std::make_unique<FilterPreset>()),
      gate(std::make_unique<GatePreset>()),
      maximizer(std::make_unique<MaximizerPreset>()),
      pitch(std::make_unique<PitchPreset>()),
      reverb(std::make_unique<ReverbPreset>()),
      webrtc(std::make_unique<WebrtcPreset>()),
      multiband_compressor(std::make_unique<MultibandCompressorPreset>()),
      loudness(std::make_unique<LoudnessPreset>()),
      multiband_gate(std::make_unique<MultibandGatePreset>()),
      stereo_tools(std::make_unique<StereoToolsPreset>()),
      convolver(std::make_unique<ConvolverPreset>()),
      crystalizer(std::make_unique<CrystalizerPreset>()),
      autogain(std::make_unique<AutoGainPreset>()),
      delay(std::make_unique<DelayPreset>()),
      spectrum(std::make_unique<SpectrumPreset>()) {
  // system presets directories provided by Glib
  for (const auto& scd : Glib::get_system_config_dirs()) {
    system_input_dir.emplace_back(scd + "/PulseEffects/input");
    system_output_dir.emplace_back(scd + "/PulseEffects/output");
  }

  // add "/etc" to system config folders array and remove duplicates
  system_input_dir.emplace_back("/etc/PulseEffects/input");
  system_output_dir.emplace_back("/etc/PulseEffects/output");
  std::sort(system_input_dir.begin(), system_input_dir.end());
  std::sort(system_output_dir.begin(), system_output_dir.end());
  system_input_dir.erase(std::unique(system_input_dir.begin(), system_input_dir.end()), system_input_dir.end());
  system_output_dir.erase(std::unique(system_output_dir.begin(), system_output_dir.end()), system_output_dir.end());

  for (const auto& scd : system_input_dir) {
    util::debug("presets_manager: system input presets directory: \"" + scd.string() + "\"; ");
  }
  for (const auto& scd : system_output_dir) {
    util::debug("presets_manager: system output presets directory: \"" + scd.string() + "\"; ");
  }

  // user presets directories
  create_user_directory(user_presets_dir);
  create_user_directory(user_input_dir);
  create_user_directory(user_output_dir);
  create_user_directory(autoload_dir);
}

PresetsManager::~PresetsManager() {
  util::debug(log_tag + "destroyed");
}

void PresetsManager::create_user_directory(const boost::filesystem::path& path) {
  auto dir_exists = boost::filesystem::is_directory(path);

  if (!dir_exists) {
    if (boost::filesystem::create_directories(path)) {
      util::debug(log_tag + "user presets directory created: " + path.string());
    } else {
      util::warning(log_tag + "failed to create user presets directory: " + path.string());
    }

  } else {
    util::debug(log_tag + "user presets directory already exists: " + path.string());
  }
}

auto PresetsManager::get_names(PresetType preset_type) -> std::vector<std::string> {
  boost::filesystem::directory_iterator it;
  std::vector<std::string> names;
  std::vector<boost::filesystem::path> sys_dirs;

  // system directories search
  switch (preset_type) {
    case PresetType::output:
      sys_dirs.insert(sys_dirs.end(), system_output_dir.begin(), system_output_dir.end());
      break;
    case PresetType::input:
      sys_dirs.insert(sys_dirs.end(), system_input_dir.begin(), system_input_dir.end());
      break;
  }

  for (const auto& dir : sys_dirs) {
    if (boost::filesystem::exists(dir)) {
      it = boost::filesystem::directory_iterator{dir};
      auto vn = search_names(it);
      names.insert(names.end(), vn.begin(), vn.end());
    }
  }

  // user directory search
  auto& user_dir = (preset_type == PresetType::output) ? user_output_dir : user_input_dir;
  it = boost::filesystem::directory_iterator{user_dir};

  auto vn = search_names(it);
  names.insert(names.end(), vn.begin(), vn.end());

  // removing duplicates
  std::sort(names.begin(), names.end());
  names.erase(std::unique(names.begin(), names.end()), names.end());

  return names;
}

auto PresetsManager::search_names(boost::filesystem::directory_iterator& it) -> std::vector<std::string> {
  std::vector<std::string> names;

  try {
    while (it != boost::filesystem::directory_iterator{}) {
      if (boost::filesystem::is_regular_file(it->status())) {
        if (it->path().extension().string() == ".json") {
          names.emplace_back(it->path().stem().string());
        }
      }

      it++;
    }
  } catch (std::exception& e) {
  }

  return names;
}

void PresetsManager::add(PresetType preset_type, const std::string& name) {
  for (const auto& p : get_names(preset_type)) {
    if (p == name) {
      return;
    }
  }

  save(preset_type, name);
}

void PresetsManager::save_blocklist(PresetType preset_type, boost::property_tree::ptree& root) {
  std::vector<std::string> blocklist;
  boost::property_tree::ptree node_in;

  switch (preset_type) {
    case PresetType::output: {
      blocklist = settings->get_string_array("blocklist-out");

      node_in.clear();

      for (const auto& p : blocklist) {
        boost::property_tree::ptree node;
        node.put("", p);
        node_in.push_back(std::make_pair("", node));
      }

      root.add_child("output.blocklist", node_in);

      break;
    }
    case PresetType::input: {
      blocklist = settings->get_string_array("blocklist-in");

      node_in.clear();

      for (const auto& p : blocklist) {
        boost::property_tree::ptree node;
        node.put("", p);
        node_in.push_back(std::make_pair("", node));
      }

      root.add_child("input.blocklist", node_in);

      break;
    }
  }

}

void PresetsManager::load_blocklist(PresetType preset_type, const boost::property_tree::ptree& root) {
  std::vector<std::string> blocklist;

  switch (preset_type) {
    case PresetType::output: {
      try {
        for (const auto& p : root.get_child("input.blocklist")) {
          blocklist.emplace_back(p.second.data());
        }

        settings->set_string_array("blocklist-in", blocklist);
      }
      catch (const boost::property_tree::ptree_error& e) {
        settings->reset("blocklist-in");
      }

      break;
    }
    case PresetType::input: {
      try {
        for (const auto& p : root.get_child("output.blocklist")) {
          blocklist.emplace_back(p.second.data());
        }

        settings->set_string_array("blocklist-out", blocklist);
      }
      catch (const boost::property_tree::ptree_error& e) {
        settings->reset("blocklist-out");
      }

      break;
    }
  }

}

void PresetsManager::save(PresetType preset_type, const std::string& name) {
  boost::property_tree::ptree root;
  boost::property_tree::ptree node_in;
  boost::property_tree::ptree node_out;
  boost::filesystem::path output_file;

  spectrum->write(preset_type, root);
  save_blocklist(preset_type, root);

  switch (preset_type) {
    case PresetType::output: {
      std::vector<std::string> output_plugins = sie_settings->get_string_array("plugins");

      for (const auto& p : output_plugins) {
        boost::property_tree::ptree node;
        node.put("", p);
        node_out.push_back(std::make_pair("", node));
      }

      root.add_child("output.plugins_order", node_out);

      output_file = user_output_dir / boost::filesystem::path{name + ".json"};

      break;
    }
    case PresetType::input: {
      std::vector<std::string> input_plugins = soe_settings->get_string_array("plugins");

      for (const auto& p : input_plugins) {
        boost::property_tree::ptree node;
        node.put("", p);
        node_in.push_back(std::make_pair("", node));
      }

      root.add_child("input.plugins_order", node_in);

      output_file = user_input_dir / boost::filesystem::path{name + ".json"};

      break;
    }
  }

  bass_enhancer->write(preset_type, root);
  compressor->write(preset_type, root);
  crossfeed->write(preset_type, root);
  deesser->write(preset_type, root);
  equalizer->write(preset_type, root);
  exciter->write(preset_type, root);
  filter->write(preset_type, root);
  gate->write(preset_type, root);
  limiter->write(preset_type, root);
  maximizer->write(preset_type, root);
  pitch->write(preset_type, root);
  reverb->write(preset_type, root);
  webrtc->write(preset_type, root);
  multiband_compressor->write(preset_type, root);
  loudness->write(preset_type, root);
  multiband_gate->write(preset_type, root);
  stereo_tools->write(preset_type, root);
  convolver->write(preset_type, root);
  crystalizer->write(preset_type, root);
  autogain->write(preset_type, root);
  delay->write(preset_type, root);

  boost::property_tree::write_json(output_file.string(), root);

  util::debug(log_tag + "saved preset: " + output_file.string());
}

void PresetsManager::remove(PresetType preset_type, const std::string& name) {
  boost::filesystem::path preset_file;
  auto& user_dir = (preset_type == PresetType::output) ? user_output_dir : user_input_dir;

  preset_file = user_dir / boost::filesystem::path{name + ".json"};

  if (boost::filesystem::exists(preset_file)) {
    boost::filesystem::remove(preset_file);

    util::debug(log_tag + "removed preset: " + preset_file.string());
  }
}

void PresetsManager::load(PresetType preset_type, const std::string& name) {
  boost::property_tree::ptree root;
  std::vector<std::string> input_plugins;
  std::vector<std::string> output_plugins;
  std::vector<boost::filesystem::path> conf_dirs;
  boost::filesystem::path input_file;
  bool preset_found = false;

  switch (preset_type) {
    case PresetType::output: {
      conf_dirs.emplace_back(user_output_dir);
      conf_dirs.insert(conf_dirs.end(), system_output_dir.begin(), system_output_dir.end());

      for (const auto& dir : conf_dirs) {
        input_file = dir / boost::filesystem::path{name + ".json"};
        if (boost::filesystem::exists(input_file)) {
          preset_found = true;
          break;
        }
      }

      if (preset_found) {
        try {
          boost::property_tree::read_json(input_file.string(), root);

          Glib::Variant<std::vector<std::string>> aux;
          sie_settings->get_default_value("plugins", aux);

          for (const auto& p : root.get_child("output.plugins_order")) {
            auto& value = p.second.data();

            for (const auto& v : aux.get()) {
              if (v == value) {
                output_plugins.emplace_back(value);

                break;
              }
            }
          }

          for (const auto& v : aux.get()) {
            if (std::find(output_plugins.begin(), output_plugins.end(), v) == output_plugins.end()) {
              output_plugins.emplace_back(v);
            }
          }
        } catch (const boost::property_tree::ptree_error& e) {
          Glib::Variant<std::vector<std::string>> aux;
          sie_settings->get_default_value("plugins", aux);
          output_plugins = aux.get();
        }

        sie_settings->set_string_array("plugins", output_plugins);
      } else {
        util::debug("can't found the preset " + name + " on the filesystem");
      }

      break;
    }
    case PresetType::input: {
      conf_dirs.emplace_back(user_input_dir);
      conf_dirs.insert(conf_dirs.end(), system_input_dir.begin(), system_input_dir.end());

      for (const auto& dir : conf_dirs) {
        input_file = dir / boost::filesystem::path{name + ".json"};
        if (boost::filesystem::exists(input_file)) {
          preset_found = true;
          break;
        }
      }

      if (preset_found) {
        try {
          boost::property_tree::read_json(input_file.string(), root);

          Glib::Variant<std::vector<std::string>> aux;
          soe_settings->get_default_value("plugins", aux);

          for (const auto& p : root.get_child("input.plugins_order")) {
            auto& value = p.second.data();

            for (const auto& v : aux.get()) {
              if (v == value) {
                input_plugins.emplace_back(value);

                break;
              }
            }
          }

          for (const auto& v : aux.get()) {
            if (std::find(input_plugins.begin(), input_plugins.end(), v) == input_plugins.end()) {
              input_plugins.emplace_back(v);
            }
          }
        } catch (const boost::property_tree::ptree_error& e) {
          Glib::Variant<std::vector<std::string>> aux;
          soe_settings->get_default_value("plugins", aux);
          input_plugins = aux.get();
        }

        soe_settings->set_string_array("plugins", input_plugins);
      } else {
        util::debug("can't found the preset " + name + " on the filesystem");
      }

      break;
    }
  }

  load_blocklist(preset_type, root);

  spectrum->read(preset_type, root);
  bass_enhancer->read(preset_type, root);
  compressor->read(preset_type, root);
  crossfeed->read(preset_type, root);
  deesser->read(preset_type, root);
  equalizer->read(preset_type, root);
  exciter->read(preset_type, root);
  filter->read(preset_type, root);
  gate->read(preset_type, root);
  limiter->read(preset_type, root);
  maximizer->read(preset_type, root);
  pitch->read(preset_type, root);
  reverb->read(preset_type, root);
  webrtc->read(preset_type, root);
  multiband_compressor->read(preset_type, root);
  loudness->read(preset_type, root);
  multiband_gate->read(preset_type, root);
  stereo_tools->read(preset_type, root);
  convolver->read(preset_type, root);
  crystalizer->read(preset_type, root);
  autogain->read(preset_type, root);
  delay->read(preset_type, root);

  util::debug(log_tag + "loaded preset: " + input_file.string());
}

void PresetsManager::import(PresetType preset_type, const std::string& file_path) {
  boost::filesystem::path p{file_path};

  if (boost::filesystem::is_regular_file(p)) {
    if (p.extension().string() == ".json") {
      boost::filesystem::path out_path;
      auto& user_dir = (preset_type == PresetType::output) ? user_output_dir: user_input_dir;

      out_path = user_dir / p.filename();

      boost::filesystem::copy_file(p, out_path, boost::filesystem::copy_option::overwrite_if_exists);

      util::debug(log_tag + "imported preset to: " + out_path.string());
    }
  } else {
    util::warning(log_tag + p.string() + " is not a file!");
  }
}

void PresetsManager::add_autoload(const std::string& device, const std::string& name) {
  boost::property_tree::ptree root;
  boost::filesystem::path output_file;

  output_file = autoload_dir / boost::filesystem::path{device + ".json"};

  root.put("name", name);

  boost::property_tree::write_json(output_file.string(), root);

  util::debug(log_tag + "added autoload preset file: " + output_file.string());
}

void PresetsManager::remove_autoload(const std::string& device, const std::string& name) {
  auto input_file = autoload_dir / boost::filesystem::path{device + ".json"};

  if (boost::filesystem::is_regular_file(input_file)) {
    boost::property_tree::ptree root;

    boost::property_tree::read_json(input_file.string(), root);

    auto current_autoload = root.get<std::string>("name", "");

    if (current_autoload == name) {
      boost::filesystem::remove(input_file);

      util::debug(log_tag + "removed autoload: " + input_file.string());
    }
  }
}

auto PresetsManager::find_autoload(const std::string& device) -> std::string {
  auto input_file = autoload_dir / boost::filesystem::path{device + ".json"};

  if (boost::filesystem::is_regular_file(input_file)) {
    boost::property_tree::ptree root;

    boost::property_tree::read_json(input_file.string(), root);

    return root.get<std::string>("name", "");
  }

  return "";
}

void PresetsManager::autoload(PresetType preset_type, const std::string& device) {
  auto name = find_autoload(device);

  if (!name.empty()) {
    util::debug(log_tag + "autoloading preset " + name + " for device " + device);

    load(preset_type, name);

    switch (preset_type) {
      case PresetType::output:
        settings->set_string("last-used-output-preset", name);
        break;
      case PresetType::input:
        settings->set_string("last-used-input-preset", name);
        break;
    }

  }
}

auto PresetsManager::preset_file_exists(PresetType preset_type, const std::string& name) -> bool {
  boost::filesystem::path input_file;
  std::vector<boost::filesystem::path> conf_dirs;

  switch (preset_type) {
    case PresetType::output: {
      conf_dirs.emplace_back(user_output_dir);
      conf_dirs.insert(conf_dirs.end(), system_output_dir.begin(), system_output_dir.end());

      for (const auto& dir : conf_dirs) {
        input_file = dir / boost::filesystem::path{name + ".json"};
        if (boost::filesystem::exists(input_file)) {
          return true;
        }
      }

      break;
    }
    case PresetType::input: {
      conf_dirs.emplace_back(user_input_dir);
      conf_dirs.insert(conf_dirs.end(), system_input_dir.begin(), system_input_dir.end());

      for (const auto& dir : conf_dirs) {
        input_file = dir / boost::filesystem::path{name + ".json"};
        if (boost::filesystem::exists(input_file)) {
          return true;
        }
      }

      break;
    }
  }

  return false;
}
