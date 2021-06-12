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

#include "presets_manager.hpp"

PresetsManager::PresetsManager()
    : user_presets_dir(Glib::get_user_config_dir() + "/easyeffects/"),
      user_input_dir(Glib::get_user_config_dir() + "/easyeffects/input"),
      user_output_dir(Glib::get_user_config_dir() + "/easyeffects/output"),
      autoload_dir(Glib::get_user_config_dir() + "/easyeffects/autoload"),
      settings(Gio::Settings::create("com.github.wwmm.easyeffects")),
      soe_settings(Gio::Settings::create("com.github.wwmm.easyeffects.streamoutputs")),
      sie_settings(Gio::Settings::create("com.github.wwmm.easyeffects.streaminputs")),
      autogain(std::make_unique<AutoGainPreset>()),
      bass_enhancer(std::make_unique<BassEnhancerPreset>()),
      compressor(std::make_unique<CompressorPreset>()),
      convolver(std::make_unique<ConvolverPreset>()),
      crossfeed(std::make_unique<CrossfeedPreset>()),
      crystalizer(std::make_unique<CrystalizerPreset>()),
      deesser(std::make_unique<DeesserPreset>()),
      delay(std::make_unique<DelayPreset>()),
      echo_canceller(std::make_unique<EchoCancellerPreset>()),
      equalizer(std::make_unique<EqualizerPreset>()),
      exciter(std::make_unique<ExciterPreset>()),
      filter(std::make_unique<FilterPreset>()),
      gate(std::make_unique<GatePreset>()),
      limiter(std::make_unique<LimiterPreset>()),
      loudness(std::make_unique<LoudnessPreset>()),
      maximizer(std::make_unique<MaximizerPreset>()),
      multiband_compressor(std::make_unique<MultibandCompressorPreset>()),
      multiband_gate(std::make_unique<MultibandGatePreset>()),
      pitch(std::make_unique<PitchPreset>()),
      reverb(std::make_unique<ReverbPreset>()),
      rnnoise(std::make_unique<RNNoisePreset>()),
      spectrum(std::make_unique<SpectrumPreset>()),
      stereo_tools(std::make_unique<StereoToolsPreset>()) {
  // system presets directories provided by Glib

  for (const auto& scd : Glib::get_system_config_dirs()) {
    system_input_dir.emplace_back(scd + "/easyeffects/input");
    system_output_dir.emplace_back(scd + "/easyeffects/output");
  }

  // add "/etc" to system config folders array and remove duplicates
  system_input_dir.emplace_back("/etc/easyeffects/input");
  system_output_dir.emplace_back("/etc/easyeffects/output");
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

  user_output_monitor = Gio::File::create_for_path(user_output_dir.string())->monitor_directory();

  user_output_monitor->signal_changed().connect(
      [=, this](const Glib::RefPtr<Gio::File>& file, auto other_f, auto event) {
        switch (event) {
          case Gio::FileMonitor::Event::CREATED: {
            user_output_preset_created.emit(file);
            break;
          }
          case Gio::FileMonitor::Event::DELETED: {
            user_output_preset_removed.emit(file);
            break;
          }
          default:
            break;
        }
      });

  user_input_monitor = Gio::File::create_for_path(user_input_dir.string())->monitor_directory();

  user_input_monitor->signal_changed().connect(
      [=, this](const Glib::RefPtr<Gio::File>& file, auto other_f, auto event) {
        switch (event) {
          case Gio::FileMonitor::Event::CREATED: {
            user_input_preset_created.emit(file);
            break;
          }
          case Gio::FileMonitor::Event::DELETED: {
            user_input_preset_removed.emit(file);
            break;
          }
          default:
            break;
        }
      });
}

PresetsManager::~PresetsManager() {
  user_output_monitor->cancel();
  user_input_monitor->cancel();

  util::debug(log_tag + "destroyed");
}

void PresetsManager::create_user_directory(const std::filesystem::path& path) {
  auto dir_exists = std::filesystem::is_directory(path);

  if (!dir_exists) {
    if (std::filesystem::create_directories(path)) {
      util::debug(log_tag + "user presets directory created: " + path.string());
    } else {
      util::warning(log_tag + "failed to create user presets directory: " + path.string());
    }

  } else {
    util::debug(log_tag + "user presets directory already exists: " + path.string());
  }
}

auto PresetsManager::get_names(PresetType preset_type) -> std::vector<Glib::ustring> {
  std::filesystem::directory_iterator it;
  std::vector<Glib::ustring> names;
  std::vector<std::filesystem::path> sys_dirs;

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
    if (std::filesystem::exists(dir)) {
      it = std::filesystem::directory_iterator{dir};
      auto vn = search_names(it);
      names.insert(names.end(), vn.begin(), vn.end());
    }
  }

  // user directory search
  auto& user_dir = (preset_type == PresetType::output) ? user_output_dir : user_input_dir;
  it = std::filesystem::directory_iterator{user_dir};

  auto vn = search_names(it);
  names.insert(names.end(), vn.begin(), vn.end());

  // removing duplicates
  std::sort(names.begin(), names.end());
  names.erase(std::unique(names.begin(), names.end()), names.end());

  return names;
}

auto PresetsManager::search_names(std::filesystem::directory_iterator& it) -> std::vector<std::string> {
  std::vector<std::string> names;

  try {
    while (it != std::filesystem::directory_iterator{}) {
      if (std::filesystem::is_regular_file(it->status())) {
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

void PresetsManager::add(PresetType preset_type, const Glib::ustring& name) {
  for (const auto& p : get_names(preset_type)) {
    if (p == name) {
      return;
    }
  }

  save(preset_type, name);
}

void PresetsManager::save_blocklist(PresetType preset_type, boost::property_tree::ptree& root) {
  std::vector<Glib::ustring> blocklist;
  boost::property_tree::ptree node_in;

  switch (preset_type) {
    case PresetType::output: {
      blocklist = soe_settings->get_string_array("blocklist");

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
      blocklist = sie_settings->get_string_array("blocklist");

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

void PresetsManager::load_blocklist(PresetType preset_type, const nlohmann::json& json) {
  std::vector<Glib::ustring> blocklist;

  switch (preset_type) {
    case PresetType::input: {
      try {
        auto list = json.at("input").at("blocklist").get<std::vector<std::string>>();

        for (auto& l : list) {
          blocklist.emplace_back(l);
        }

        sie_settings->set_string_array("blocklist", blocklist);
      } catch (const nlohmann::json::exception& e) {
        sie_settings->reset("blocklist");

        util::warning(log_tag + e.what());
      }

      break;
    }
    case PresetType::output: {
      try {
        auto list = json.at("output").at("blocklist").get<std::vector<std::string>>();

        for (auto& l : list) {
          blocklist.emplace_back(l);
        }

        soe_settings->set_string_array("blocklist", blocklist);
      } catch (const nlohmann::json::exception& e) {
        soe_settings->reset("blocklist");

        util::warning(log_tag + e.what());
      }

      break;
    }
  }
}

void PresetsManager::save(PresetType preset_type, const std::string& name) {
  boost::property_tree::ptree root;
  boost::property_tree::ptree node_in;
  boost::property_tree::ptree node_out;
  std::filesystem::path output_file;

  // spectrum->write(preset_type, root);
  // save_blocklist(preset_type, root);

  // switch (preset_type) {
  //   case PresetType::output: {
  //     std::vector<Glib::ustring> output_plugins = soe_settings->get_string_array("plugins");

  //     for (const auto& p : output_plugins) {
  //       boost::property_tree::ptree node;
  //       node.put("", p);
  //       node_out.push_back(std::make_pair("", node));
  //     }

  //     root.add_child("output.plugins_order", node_out);

  //     output_file = user_output_dir / std::filesystem::path{name + ".json"};

  //     break;
  //   }
  //   case PresetType::input: {
  //     std::vector<Glib::ustring> input_plugins = sie_settings->get_string_array("plugins");

  //     for (const auto& p : input_plugins) {
  //       boost::property_tree::ptree node;
  //       node.put("", p);
  //       node_in.push_back(std::make_pair("", node));
  //     }

  //     root.add_child("input.plugins_order", node_in);

  //     output_file = user_input_dir / std::filesystem::path{name + ".json"};

  //     break;
  //   }
  // }

  // autogain->write(preset_type, root);
  // bass_enhancer->write(preset_type, root);
  // compressor->write(preset_type, root);
  // convolver->write(preset_type, root);
  // crossfeed->write(preset_type, root);
  // crystalizer->write(preset_type, root);
  // deesser->write(preset_type, root);
  // delay->write(preset_type, root);
  // echo_canceller->write(preset_type, root);
  // equalizer->write(preset_type, root);
  // exciter->write(preset_type, root);
  // filter->write(preset_type, root);
  // gate->write(preset_type, root);
  // limiter->write(preset_type, root);
  // loudness->write(preset_type, root);
  // maximizer->write(preset_type, root);
  // multiband_compressor->write(preset_type, root);
  // multiband_gate->write(preset_type, root);
  // pitch->write(preset_type, root);
  // reverb->write(preset_type, root);
  // rnnoise->write(preset_type, root);
  // stereo_tools->write(preset_type, root);

  // boost::property_tree::write_json(output_file.string(), root);

  // util::debug(log_tag + "saved preset: " + output_file.string());
}

void PresetsManager::remove(PresetType preset_type, const std::string& name) {
  std::filesystem::path preset_file;
  auto& user_dir = (preset_type == PresetType::output) ? user_output_dir : user_input_dir;

  preset_file = user_dir / std::filesystem::path{name + ".json"};

  if (std::filesystem::exists(preset_file)) {
    std::filesystem::remove(preset_file);

    util::debug(log_tag + "removed preset: " + preset_file.string());
  }
}

void PresetsManager::load(PresetType preset_type, const std::string& name) {
  nlohmann::json json;

  boost::property_tree::ptree root;

  std::vector<Glib::ustring> input_plugins;
  std::vector<Glib::ustring> output_plugins;
  std::vector<std::filesystem::path> conf_dirs;

  std::filesystem::path input_file;

  bool preset_found = false;

  switch (preset_type) {
    case PresetType::output: {
      conf_dirs.emplace_back(user_output_dir);
      conf_dirs.insert(conf_dirs.end(), system_output_dir.begin(), system_output_dir.end());

      for (const auto& dir : conf_dirs) {
        input_file = dir / std::filesystem::path{name + ".json"};
        if (std::filesystem::exists(input_file)) {
          preset_found = true;
          break;
        }
      }

      if (preset_found) {
        boost::property_tree::read_json(input_file.string(), root);

        try {
          std::ifstream is(input_file);

          is >> json;

          auto j_plugins_order = json.at("output").at("plugins_order").get<std::vector<std::string>>();

          for (const auto& p : j_plugins_order) {
            for (const auto& v : plugin_name::list) {
              if (v == p) {
                output_plugins.emplace_back(p);

                break;
              }
            }
          }

        } catch (const nlohmann::json::exception& e) {
          output_plugins.clear();

          util::warning(log_tag + e.what());
        }

        soe_settings->set_string_array("plugins", output_plugins);
      } else {
        util::debug("can't found the preset " + name + " on the filesystem");
      }

      break;
    }
    case PresetType::input: {
      conf_dirs.emplace_back(user_input_dir);
      conf_dirs.insert(conf_dirs.end(), system_input_dir.begin(), system_input_dir.end());

      for (const auto& dir : conf_dirs) {
        input_file = dir / std::filesystem::path{name + ".json"};
        if (std::filesystem::exists(input_file)) {
          preset_found = true;
          break;
        }
      }

      if (preset_found) {
        boost::property_tree::read_json(input_file.string(), root);

        try {
          std::ifstream is(input_file);

          is >> json;

          auto j_plugins_order = json.at("input").at("plugins_order").get<std::vector<std::string>>();

          for (const auto& p : j_plugins_order) {
            for (const auto& v : plugin_name::list) {
              if (v == p) {
                input_plugins.emplace_back(p);

                break;
              }
            }
          }

        } catch (const nlohmann::json::exception& e) {
          input_plugins.clear();

          util::warning(log_tag + e.what());
        }

        sie_settings->set_string_array("plugins", input_plugins);
      } else {
        util::debug("can't found the preset " + name + " on the filesystem");
      }

      break;
    }
  }

  load_blocklist(preset_type, json);

  autogain->read(preset_type, json);
  bass_enhancer->read(preset_type, json);
  compressor->read(preset_type, json);
  convolver->read(preset_type, json);
  crossfeed->read(preset_type, json);
  crystalizer->read(preset_type, json);
  deesser->read(preset_type, json);
  delay->read(preset_type, json);
  echo_canceller->read(preset_type, json);
  equalizer->read(preset_type, json);
  exciter->read(preset_type, json);
  filter->read(preset_type, json);
  gate->read(preset_type, json);
  limiter->read(preset_type, json);
  loudness->read(preset_type, json);
  maximizer->read(preset_type, json);
  multiband_compressor->read(preset_type, json);

  multiband_gate->read(preset_type, root);
  pitch->read(preset_type, root);
  reverb->read(preset_type, root);
  rnnoise->read(preset_type, root);
  spectrum->read(preset_type, root);
  stereo_tools->read(preset_type, root);

  util::debug(log_tag + "loaded preset: " + input_file.string());
}

void PresetsManager::import(PresetType preset_type, const std::string& file_path) {
  std::filesystem::path p{file_path};

  if (std::filesystem::is_regular_file(p)) {
    if (p.extension().string() == ".json") {
      std::filesystem::path out_path;
      auto& user_dir = (preset_type == PresetType::output) ? user_output_dir : user_input_dir;

      out_path = user_dir / p.filename();

      std::filesystem::copy_file(p, out_path, std::filesystem::copy_options::overwrite_existing);

      util::debug(log_tag + "imported preset to: " + out_path.string());
    }
  } else {
    util::warning(log_tag + p.string() + " is not a file!");
  }
}

void PresetsManager::add_autoload(const std::string& device, const std::string& name) {
  boost::property_tree::ptree root;
  std::filesystem::path output_file;

  output_file = autoload_dir / std::filesystem::path{device + ".json"};

  root.put("name", name);

  boost::property_tree::write_json(output_file.string(), root);

  util::debug(log_tag + "added autoload preset file: " + output_file.string());
}

void PresetsManager::remove_autoload(const std::string& device, const std::string& name) {
  auto input_file = autoload_dir / std::filesystem::path{device + ".json"};

  if (std::filesystem::is_regular_file(input_file)) {
    boost::property_tree::ptree root;

    boost::property_tree::read_json(input_file.string(), root);

    auto current_autoload = root.get<std::string>("name", "");

    if (current_autoload == name) {
      std::filesystem::remove(input_file);

      util::debug(log_tag + "removed autoload: " + input_file.string());
    }
  }
}

auto PresetsManager::find_autoload(const std::string& device) -> std::string {
  auto input_file = autoload_dir / std::filesystem::path{device + ".json"};

  if (std::filesystem::is_regular_file(input_file)) {
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
  std::filesystem::path input_file;
  std::vector<std::filesystem::path> conf_dirs;

  switch (preset_type) {
    case PresetType::output: {
      conf_dirs.emplace_back(user_output_dir);
      conf_dirs.insert(conf_dirs.end(), system_output_dir.begin(), system_output_dir.end());

      for (const auto& dir : conf_dirs) {
        input_file = dir / std::filesystem::path{name + ".json"};
        if (std::filesystem::exists(input_file)) {
          return true;
        }
      }

      break;
    }
    case PresetType::input: {
      conf_dirs.emplace_back(user_input_dir);
      conf_dirs.insert(conf_dirs.end(), system_input_dir.begin(), system_input_dir.end());

      for (const auto& dir : conf_dirs) {
        input_file = dir / std::filesystem::path{name + ".json"};
        if (std::filesystem::exists(input_file)) {
          return true;
        }
      }

      break;
    }
  }

  return false;
}
