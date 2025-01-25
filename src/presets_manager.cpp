/*
 *  Copyright © 2017-2025 Wellington Wallace
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "presets_manager.hpp"
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <sys/types.h>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <ostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "autogain_preset.hpp"
#include "bass_enhancer_preset.hpp"
#include "bass_loudness_preset.hpp"
#include "compressor_preset.hpp"
#include "convolver_preset.hpp"
#include "crossfeed_preset.hpp"
#include "crystalizer_preset.hpp"
#include "deepfilternet_preset.hpp"
#include "deesser_preset.hpp"
#include "delay_preset.hpp"
#include "echo_canceller_preset.hpp"
#include "equalizer_preset.hpp"
#include "exciter_preset.hpp"
#include "expander_preset.hpp"
#include "filter_preset.hpp"
#include "gate_preset.hpp"
#include "level_meter_preset.hpp"
#include "limiter_preset.hpp"
#include "loudness_preset.hpp"
#include "maximizer_preset.hpp"
#include "multiband_compressor_preset.hpp"
#include "multiband_gate_preset.hpp"
#include "pitch_preset.hpp"
#include "plugin_preset_base.hpp"
#include "preset_type.hpp"
#include "reverb_preset.hpp"
#include "rnnoise_preset.hpp"
#include "speex_preset.hpp"
#include "stereo_tools_preset.hpp"
#include "tags_app.hpp"
#include "tags_plugin_name.hpp"
#include "tags_resources.hpp"
#include "tags_schema.hpp"
#include "util.hpp"

PresetsManager::PresetsManager()
    : user_config_dir(g_get_user_config_dir()),
      user_input_dir(user_config_dir + "/easyeffects/input"),
      user_output_dir(user_config_dir + "/easyeffects/output"),
      user_irs_dir(user_config_dir + "/easyeffects/irs"),
      user_rnnoise_dir(user_config_dir + "/easyeffects/rnnoise"),
      autoload_input_dir(user_config_dir + "/easyeffects/autoload/input"),
      autoload_output_dir(user_config_dir + "/easyeffects/autoload/output"),
      settings(g_settings_new(tags::app::id)),
      soe_settings(g_settings_new(tags::schema::id_output)),
      sie_settings(g_settings_new(tags::schema::id_input)) {
  // Initialize input and output directories for community presets.
  // Flatpak specific path (.flatpak-info always present for apps running in the flatpak sandbox).
  if (std::filesystem::is_regular_file(tags::resources::flatpak_info_file)) {
    system_data_dir_input.push_back("/app/extensions/Presets/input");
    system_data_dir_output.push_back("/app/extensions/Presets/output");
    system_data_dir_irs.push_back("/app/extensions/Presets/irs");
    system_data_dir_rnnoise.push_back("/app/extensions/Presets/rnnoise");
  }

  // Regular paths.
  for (const gchar* const* xdg_data_dirs = g_get_system_data_dirs(); *xdg_data_dirs != nullptr;) {
    std::string dir = *xdg_data_dirs++;

    dir += dir.ends_with("/") ? "" : "/";

    system_data_dir_input.push_back(dir + "easyeffects/input");
    system_data_dir_output.push_back(dir + "easyeffects/output");
    system_data_dir_irs.push_back(dir + "easyeffects/irs");
    system_data_dir_rnnoise.push_back(dir + "easyeffects/rnnoise");
  }

  // create user presets directories

  create_user_directory(user_input_dir);
  create_user_directory(user_output_dir);
  create_user_directory(user_irs_dir);
  create_user_directory(user_rnnoise_dir);
  create_user_directory(autoload_input_dir);
  create_user_directory(autoload_output_dir);

  auto* gfile = g_file_new_for_path(user_output_dir.c_str());

  user_output_monitor = g_file_monitor_directory(gfile, G_FILE_MONITOR_NONE, nullptr, nullptr);

  g_signal_connect(user_output_monitor, "changed",
                   G_CALLBACK(+[](GFileMonitor* monitor, GFile* file, GFile* other_file, GFileMonitorEvent event_type,
                                  gpointer user_data) {
                     auto* self = static_cast<PresetsManager*>(user_data);

                     switch (event_type) {
                       case G_FILE_MONITOR_EVENT_CREATED: {
                         const auto preset_name = util::remove_filename_extension(g_file_get_basename(file));

                         self->user_output_preset_created.emit(preset_name);

                         break;
                       }
                       case G_FILE_MONITOR_EVENT_DELETED: {
                         const auto preset_name = util::remove_filename_extension(g_file_get_basename(file));

                         self->user_output_preset_removed.emit(preset_name);

                         break;
                       }
                       default:
                         break;
                     }
                   }),
                   this);

  g_object_unref(gfile);

  gfile = g_file_new_for_path(user_input_dir.c_str());

  user_input_monitor = g_file_monitor_directory(gfile, G_FILE_MONITOR_NONE, nullptr, nullptr);

  g_signal_connect(user_input_monitor, "changed",
                   G_CALLBACK(+[](GFileMonitor* monitor, GFile* file, GFile* other_file, GFileMonitorEvent event_type,
                                  gpointer user_data) {
                     auto* self = static_cast<PresetsManager*>(user_data);

                     switch (event_type) {
                       case G_FILE_MONITOR_EVENT_CREATED: {
                         const auto preset_name = util::remove_filename_extension(g_file_get_basename(file));

                         self->user_input_preset_created.emit(preset_name);

                         break;
                       }
                       case G_FILE_MONITOR_EVENT_DELETED: {
                         const auto preset_name = util::remove_filename_extension(g_file_get_basename(file));

                         self->user_input_preset_removed.emit(preset_name);

                         break;
                       }
                       default:
                         break;
                     }
                   }),
                   this);

  g_object_unref(gfile);

  gfile = g_file_new_for_path(autoload_input_dir.c_str());

  autoload_input_monitor = g_file_monitor_directory(gfile, G_FILE_MONITOR_NONE, nullptr, nullptr);

  g_signal_connect(autoload_input_monitor, "changed",
                   G_CALLBACK(+[](GFileMonitor* monitor, GFile* file, GFile* other_file, GFileMonitorEvent event_type,
                                  gpointer user_data) {
                     auto* self = static_cast<PresetsManager*>(user_data);

                     if (event_type == G_FILE_MONITOR_EVENT_CREATED || event_type == G_FILE_MONITOR_EVENT_DELETED) {
                       const auto profiles = self->get_autoload_profiles(PresetType::input);

                       self->autoload_input_profiles_changed.emit(profiles);
                     }
                   }),
                   this);

  g_object_unref(gfile);

  gfile = g_file_new_for_path(autoload_output_dir.c_str());

  autoload_output_monitor = g_file_monitor_directory(gfile, G_FILE_MONITOR_NONE, nullptr, nullptr);

  g_signal_connect(autoload_output_monitor, "changed",
                   G_CALLBACK(+[](GFileMonitor* monitor, GFile* file, GFile* other_file, GFileMonitorEvent event_type,
                                  gpointer user_data) {
                     auto* self = static_cast<PresetsManager*>(user_data);

                     if (event_type == G_FILE_MONITOR_EVENT_CREATED || event_type == G_FILE_MONITOR_EVENT_DELETED) {
                       const auto profiles = self->get_autoload_profiles(PresetType::output);

                       self->autoload_output_profiles_changed.emit(profiles);
                     }
                   }),
                   this);

  g_object_unref(gfile);
}

PresetsManager::~PresetsManager() {
  g_file_monitor_cancel(user_output_monitor);
  g_file_monitor_cancel(user_input_monitor);
  g_file_monitor_cancel(autoload_input_monitor);
  g_file_monitor_cancel(autoload_output_monitor);

  g_object_unref(user_output_monitor);
  g_object_unref(user_input_monitor);
  g_object_unref(autoload_input_monitor);
  g_object_unref(autoload_output_monitor);
  g_object_unref(settings);
  g_object_unref(sie_settings);
  g_object_unref(soe_settings);

  util::debug("destroyed");
}

void PresetsManager::create_user_directory(const std::filesystem::path& path) {
  if (std::filesystem::is_directory(path)) {
    util::debug("user presets directory already exists: " + path.string());

    return;
  }

  if (std::filesystem::create_directories(path)) {
    util::debug("user presets directory created: " + path.string());

    return;
  }

  util::warning("failed to create user presets directory: " + path.string());
}

auto PresetsManager::get_local_presets_name(const PresetType& preset_type) -> std::vector<std::string> {
  const auto conf_dir = (preset_type == PresetType::output) ? user_output_dir : user_input_dir;

  auto it = std::filesystem::directory_iterator{conf_dir};

  auto names = search_names(it);

  // Sort alphabetically not needed anymore because
  // the GtkSortListModel does it already.
  // std::sort(names.begin(), names.end());

  // Removing duplicates not needed anymore because
  // we get the presets from a single folder.
  // names.erase(std::unique(names.begin(), names.end()), names.end());

  return names;
}

auto PresetsManager::search_names(std::filesystem::directory_iterator& it) -> std::vector<std::string> {
  std::vector<std::string> names;

  try {
    while (it != std::filesystem::directory_iterator{}) {
      if (std::filesystem::is_regular_file(it->status()) && it->path().extension().c_str() == json_ext) {
        names.emplace_back(it->path().stem().c_str());
      }

      ++it;
    }
  } catch (const std::exception& e) {
    util::warning(e.what());
  }

  return names;
}

void PresetsManager::add(const PresetType& preset_type, const std::string& name) {
  // This method assumes the filename is valid.

  for (const auto& p : get_local_presets_name(preset_type)) {
    if (p == name) {
      return;
    }
  }

  save_preset_file(preset_type, name);
}

auto PresetsManager::get_all_community_presets_paths(const PresetType& preset_type) -> std::vector<std::string> {
  std::vector<std::string> cp_paths;

  const auto scan_level = 2U;

  const auto cp_dir_vect = (preset_type == PresetType::output) ? system_data_dir_output : system_data_dir_input;

  for (const auto& cp_dir : cp_dir_vect) {
    auto cp_fs_path = std::filesystem::path{cp_dir};

    if (!std::filesystem::exists(cp_fs_path)) {
      continue;
    }

    // Scan community package directories for 2 levels (the folder itself and only its subfolders).
    auto it = std::filesystem::directory_iterator{cp_fs_path};

    try {
      while (it != std::filesystem::directory_iterator{}) {
        if (auto package_path = it->path(); std::filesystem::is_directory(it->status())) {
          const auto package_path_name = package_path.string();

          util::debug("scan directory for community presets: " + package_path_name);

          auto package_it = std::filesystem::directory_iterator{package_path};

          /* When C++23 is available, the following line is enough:
          cp_paths.append_range(
              scan_community_package_recursive(package_it, scan_level, package_path_name));
          */

          const auto sub_cp_vect = scan_community_package_recursive(package_it, scan_level, package_path_name);

          cp_paths.insert(cp_paths.end(), sub_cp_vect.cbegin(), sub_cp_vect.cend());
        }

        ++it;
      }
    } catch (const std::exception& e) {
      util::warning(e.what());
    }
  }

  return cp_paths;
}

auto PresetsManager::scan_community_package_recursive(std::filesystem::directory_iterator& it,
                                                      const uint& top_scan_level,
                                                      const std::string& origin) -> std::vector<std::string> {
  const auto scan_level = top_scan_level - 1U;

  std::vector<std::string> cp_paths;

  try {
    while (it != std::filesystem::directory_iterator{}) {
      if (std::filesystem::is_regular_file(it->status()) && it->path().extension().c_str() == json_ext) {
        cp_paths.emplace_back(origin + "/" + it->path().stem().c_str());
      } else if (scan_level > 0U && std::filesystem::is_directory(it->status())) {
        if (auto path = it->path(); !path.empty()) {
          auto subdir_it = std::filesystem::directory_iterator{path};

          /* When C++23 is available, the following line is enough:
          cp_paths.append_range(
              scan_community_package_recursive(subdir_it, scan_level, origin + "/" + path.filename().c_str()));
          */

          const auto sub_cp_vect =
              scan_community_package_recursive(subdir_it, scan_level, origin + "/" + path.filename().c_str());

          cp_paths.insert(cp_paths.end(), sub_cp_vect.cbegin(), sub_cp_vect.cend());
        }
      }

      ++it;
    }
  } catch (const std::exception& e) {
    util::warning(e.what());
  }

  return cp_paths;
}

auto PresetsManager::get_community_preset_info(const PresetType& preset_type, const std::string& path)
    -> std::pair<std::string, std::string> {
  // Given the full path of a community preset, extract and return:
  // 1. the preset name
  // 2. the package name

  static const auto re_pack_name = std::regex(R"(^\/([^/]+))");
  static const auto re_preset_name = std::regex(R"([^/]+$)");

  const auto cp_dir_vect = (preset_type == PresetType::output) ? system_data_dir_output : system_data_dir_input;

  for (const auto& cp_dir : cp_dir_vect) {
    // In order to extract the package name, let's check if
    // the preset belongs to the selected system data directory.

    const auto cp_dir_size = cp_dir.size();

    // Skip if the lenght of the path is shorter than `cp_dir + "/"`.
    if (path.size() <= cp_dir_size + 1U) {
      continue;
    }

    // Check if the preset is contained in the selected system data directory.
    // starts_with gets a string_view, so we use the version with character array.
    if (!path.starts_with(cp_dir.c_str())) {
      continue;
    }

    // Extract the package name.
    std::smatch pack_match;

    // Get the subdirectory/package.
    // - Full match: "/pack_name"
    // - Group 1: "pack_name"
    std::regex_search(path.cbegin() + cp_dir_size, path.cend(), pack_match, re_pack_name);

    // Skip if the match failed.
    if (pack_match.size() != 2U) {
      continue;
    }

    // Extract the preset name.
    std::smatch name_match;

    std::regex_search(path.cbegin() + cp_dir_size, path.cend(), name_match, re_preset_name);

    // Skip if the match failed.
    if (name_match.size() != 1U) {
      continue;
    }

    return std::make_pair(name_match.str(0), pack_match.str(1));
  }

  util::warning("can't extract info strings for the community preset: " + path);

  // Placeholders in case of issues
  return std::make_pair(std::string(_("Community Preset")), std::string(_("Package")));
}

void PresetsManager::save_blocklist(const PresetType& preset_type, nlohmann::json& json) {
  std::vector<std::string> blocklist;

  switch (preset_type) {
    case PresetType::output: {
      const auto list = util::gchar_array_to_vector(g_settings_get_strv(soe_settings, "blocklist"));

      for (const auto& l : list) {
        blocklist.push_back(l);
      }

      json["output"]["blocklist"] = blocklist;

      break;
    }
    case PresetType::input: {
      const auto list = util::gchar_array_to_vector(g_settings_get_strv(sie_settings, "blocklist"));

      for (const auto& l : list) {
        blocklist.push_back(l);
      }

      json["input"]["blocklist"] = blocklist;

      break;
    }
  }
}

auto PresetsManager::load_blocklist(const PresetType& preset_type, const nlohmann::json& json) -> bool {
  std::vector<std::string> blocklist;

  switch (preset_type) {
    case PresetType::input: {
      try {
        auto list = json.at("input").at("blocklist").get<std::vector<std::string>>();

        g_settings_set_strv(sie_settings, "blocklist", util::make_gchar_pointer_vector(list).data());
      } catch (const nlohmann::json::exception& e) {
        g_settings_reset(sie_settings, "blocklist");

        notify_error(PresetError::blocklist_format);

        util::warning(e.what());

        return false;
      } catch (...) {
        g_settings_reset(sie_settings, "blocklist");

        notify_error(PresetError::blocklist_generic);

        return false;
      }

      break;
    }
    case PresetType::output: {
      try {
        auto list = json.at("output").at("blocklist").get<std::vector<std::string>>();

        g_settings_set_strv(soe_settings, "blocklist", util::make_gchar_pointer_vector(list).data());
      } catch (const nlohmann::json::exception& e) {
        g_settings_reset(soe_settings, "blocklist");

        notify_error(PresetError::blocklist_format);

        util::warning(e.what());

        return false;
      } catch (...) {
        g_settings_reset(soe_settings, "blocklist");

        notify_error(PresetError::blocklist_generic);

        return false;
      }

      break;
    }
  }

  return true;
}

void PresetsManager::save_preset_file(const PresetType& preset_type, const std::string& name) {
  nlohmann::json json;

  std::filesystem::path output_file;

  save_blocklist(preset_type, json);

  switch (preset_type) {
    case PresetType::output: {
      const auto plugins = util::gchar_array_to_vector(g_settings_get_strv(soe_settings, "plugins"));

      std::vector<std::string> list;

      list.reserve(plugins.size());

      for (const auto& p : plugins) {
        list.push_back(p);
      }

      json["output"]["plugins_order"] = list;

      write_plugins_preset(preset_type, plugins, json);

      output_file = user_output_dir / std::filesystem::path{name + json_ext};

      break;
    }
    case PresetType::input: {
      const auto plugins = util::gchar_array_to_vector(g_settings_get_strv(sie_settings, "plugins"));

      std::vector<std::string> list;

      list.reserve(plugins.size());

      for (const auto& p : plugins) {
        list.push_back(p);
      }

      json["input"]["plugins_order"] = list;

      write_plugins_preset(preset_type, plugins, json);

      output_file = user_input_dir / std::filesystem::path{name + json_ext};

      break;
    }
  }

  std::ofstream o(output_file.c_str());

  o << std::setw(4) << json << '\n';

  // std::cout << std::setw(4) << json << std::endl;

  util::debug("saved preset: " + output_file.string());
}

auto PresetsManager::get_loaded_preset(const PresetType& preset_type) -> std::string {
  const auto* key = (preset_type == PresetType::input) ? "last-loaded-input-preset" : "last-loaded-output-preset";

  const auto* preset = g_settings_get_string(settings, key);

  return preset;
}

void PresetsManager::write_plugins_preset(const PresetType& preset_type,
                                          const std::vector<std::string>& plugins,
                                          nlohmann::json& json) {
  for (const auto& name : plugins) {
    if (auto wrapper = create_wrapper(preset_type, name); wrapper != std::nullopt) {
      if (wrapper.has_value()) {
        wrapper.value()->write(json);
      }
    }
  }
}

void PresetsManager::remove(const PresetType& preset_type, const std::string& name) {
  std::filesystem::path preset_file;

  const auto conf_dir = (preset_type == PresetType::output) ? user_output_dir : user_input_dir;

  preset_file = conf_dir / std::filesystem::path{name + json_ext};

  if (std::filesystem::exists(preset_file)) {
    std::filesystem::remove(preset_file);

    util::debug("removed preset: " + preset_file.string());
  }
}

auto PresetsManager::load_local_preset_file(const PresetType& preset_type, const std::string& name) -> bool {
  const auto conf_dir = (preset_type == PresetType::output) ? user_output_dir : user_input_dir;

  const auto input_file = conf_dir / std::filesystem::path{name + json_ext};

  // Check preset existence
  if (!std::filesystem::exists(input_file)) {
    util::debug("can't find the local preset \"" + name + "\" on the filesystem");

    return false;
  }

  set_last_preset_keys(preset_type, name);

  const auto loaded = load_preset_file(preset_type, input_file);

  if (!loaded) {
    set_last_preset_keys(preset_type);
  }

  return loaded;
}

auto PresetsManager::load_community_preset_file(const PresetType& preset_type,
                                                const std::string& full_path_stem,
                                                const std::string& package_name) -> bool {
  const auto input_file = std::filesystem::path{full_path_stem + json_ext};

  // Check preset existence
  if (!std::filesystem::exists(input_file)) {
    util::warning("the community preset \"" + input_file.string() + "\" does not exist on the filesystem");

    return false;
  }

  set_last_preset_keys(preset_type, input_file.stem().string(), package_name);

  const auto loaded = load_preset_file(preset_type, input_file);

  if (!loaded) {
    set_last_preset_keys(preset_type);
  }

  return loaded;
}

auto PresetsManager::load_preset_file(const PresetType& preset_type, const std::filesystem::path& input_file) -> bool {
  nlohmann::json json;

  std::vector<std::string> plugins;

  // Read effects_pipeline
  if (!read_effects_pipeline_from_preset(preset_type, input_file, json, plugins)) {
    return false;
  }

  // After the plugin order list, load the blocklist and then
  // apply the parameters of the loaded plugins.
  if (load_blocklist(preset_type, json) && read_plugins_preset(preset_type, plugins, json)) {
    util::debug("successfully loaded the preset: " + input_file.string());

    return true;
  }

  return false;
}

auto PresetsManager::read_effects_pipeline_from_preset(const PresetType& preset_type,
                                                       const std::filesystem::path& input_file,
                                                       nlohmann::json& json,
                                                       std::vector<std::string>& plugins) -> bool {
  const auto* preset_type_str = (preset_type == PresetType::input) ? "input" : "output";

  GSettings* settings = (preset_type == PresetType::input) ? sie_settings : soe_settings;

  try {
    std::ifstream is(input_file);

    is >> json;

    for (const auto& p : json.at(preset_type_str).at("plugins_order").get<std::vector<std::string>>()) {
      for (const auto& v : tags::plugin_name::list) {
        if (p.starts_with(v)) {
          /*
            Old format presets do not have the instance id number in the filter names. They are equal to the
            base name.
          */

          if (p != v) {
            plugins.push_back(p);
          } else {
            plugins.push_back(p + "#0");
          }

          break;
        }
      }
    }
  } catch (const nlohmann::json::exception& e) {
    notify_error(PresetError::pipeline_format);

    util::warning(e.what());

    return false;
  } catch (...) {
    notify_error(PresetError::pipeline_generic);

    return false;
  }

  g_settings_set_strv(settings, "plugins", util::make_gchar_pointer_vector(plugins).data());

  return true;
}

auto PresetsManager::read_plugins_preset(const PresetType& preset_type,
                                         const std::vector<std::string>& plugins,
                                         const nlohmann::json& json) -> bool {
  for (const auto& name : plugins) {
    if (auto wrapper = create_wrapper(preset_type, name); wrapper != std::nullopt) {
      try {
        if (wrapper.has_value()) {
          wrapper.value()->read(json);
        }
      } catch (const nlohmann::json::exception& e) {
        notify_error(PresetError::plugin_format, name);

        util::warning(e.what());

        return false;
      } catch (...) {
        notify_error(PresetError::plugin_generic, name);

        return false;
      }
    }
  }

  return true;
}

void PresetsManager::import_from_filesystem(const PresetType& preset_type, const std::string& file_path) {
  // When importing presets from the filesystem, we overwrite the file if it already exists.

  std::filesystem::path p{file_path};

  if (!std::filesystem::is_regular_file(p)) {
    util::warning(p.string() + " is not a file!");

    return;
  }

  if (p.extension().c_str() != json_ext) {
    return;
  }

  const auto conf_dir = (preset_type == PresetType::output) ? user_output_dir : user_input_dir;

  const std::filesystem::path out_path = conf_dir / p.filename();

  try {
    std::filesystem::copy_file(p, out_path, std::filesystem::copy_options::overwrite_existing);

    util::debug("imported preset to: " + out_path.string());
  } catch (const std::exception& e) {
    util::warning("can't import preset to: " + out_path.string());
    util::warning(e.what());
  }
}

auto PresetsManager::import_addons_from_community_package(const PresetType& preset_type,
                                                          const std::filesystem::path& path,
                                                          const std::string& package) -> bool {
  /* Here we parse the json community preset in order to import the list of addons:
   * 1. Convolver Impulse Response Files
   * 2. RNNoise Models
   */

  // This method assumes that the path is valid and package string is not empty,
  // their check has already been made in import_from_community_package();
  std::ifstream is(path);

  nlohmann::json json;

  const auto irs_ext = ".irs";
  const auto rnnn_ext = ".rnnn";

  try {
    is >> json;

    std::vector<std::string> conv_irs, rn_models;

    const auto* pt_key = (preset_type == PresetType::output) ? "output" : "input";

    // Fill conv_irs and rn_models vectors extracting the addon names from
    // the json preset and append the respective file extension.
    for (const auto& plugin : json.at(pt_key).at("plugins_order").get<std::vector<std::string>>()) {
      if (plugin.starts_with(tags::plugin_name::convolver)) {
        conv_irs.push_back(json.at(pt_key).at(plugin).at("kernel-name").get<std::string>() + irs_ext);
      }

      if (plugin.starts_with(tags::plugin_name::rnnoise)) {
        rn_models.push_back(json.at(pt_key).at(plugin).at("model-name").get<std::string>() + rnnn_ext);
      }
    }

    // For every filename of both vectors, search the full path and copy the file locally.
    for (const auto& irs_name : conv_irs) {
      std::string path;

      bool found = false;

      for (const auto& xdg_dir : system_data_dir_irs) {
        if (util::search_filename(std::filesystem::path{xdg_dir + "/" + package}, irs_name, path, 3U)) {
          const auto out_path = std::filesystem::path{user_irs_dir} / irs_name;

          std::filesystem::copy_file(path, out_path, std::filesystem::copy_options::overwrite_existing);

          util::debug("successfully imported community preset addon " + irs_name + " locally");

          found = true;

          break;
        }
      }

      if (!found) {
        util::warning("community preset addon " + irs_name + " not found!");

        return false;
      }
    }

    for (const auto& model_name : rn_models) {
      std::string path;

      bool found = false;

      for (const auto& xdg_dir : system_data_dir_rnnoise) {
        if (util::search_filename(std::filesystem::path{xdg_dir + "/" + package}, model_name, path, 3U)) {
          const auto out_path = std::filesystem::path{user_rnnoise_dir} / model_name;

          std::filesystem::copy_file(path, out_path, std::filesystem::copy_options::overwrite_existing);

          util::debug("successfully imported community preset addon " + model_name + " locally");

          found = true;

          break;
        }
      }

      if (!found) {
        util::warning("community preset addon " + model_name + " not found!");

        return false;
      }
    }

    return true;
  } catch (const std::exception& e) {
    util::warning(e.what());

    return false;
  }
}

void PresetsManager::import_from_community_package(const PresetType& preset_type,
                                                   const std::string& file_path,
                                                   const std::string& package) {
  // When importing presets from a community package, we do NOT overwrite
  // the local preset if it has the same name.

  std::filesystem::path p{file_path};

  if (!std::filesystem::exists(p) || package.empty()) {
    util::warning(p.string() + " does not exist! Please reload the community preset list");

    return;
  }

  if (!std::filesystem::is_regular_file(p)) {
    util::warning(p.string() + " is not a file! Please reload the community preset list");

    return;
  }

  if (p.extension().c_str() != json_ext) {
    return;
  }

  bool preset_can_be_copied = false;

  // We limit the max copy attempts in order to not flood the local directory
  // if the user keeps clicking the import button.
  uint i = 0U;

  static const auto max_copy_attempts = 10;

  const auto conf_dir = (preset_type == PresetType::output) ? user_output_dir.string() : user_input_dir.string();

  std::filesystem::path out_path;

  try {
    do {
      // In case of destination file already existing, we try to append
      // an incremental numeric suffix.
      const auto suffix = (i == 0U) ? "" : "-" + util::to_string(i);

      out_path = conf_dir + "/" + p.stem().c_str() + suffix + json_ext;

      if (!std::filesystem::exists(out_path)) {
        preset_can_be_copied = true;

        break;
      }
    } while (++i < max_copy_attempts);
  } catch (const std::exception& e) {
    util::warning("can't import the community preset: " + p.string());

    util::warning(e.what());

    return;
  }

  if (!preset_can_be_copied) {
    util::warning("can't import the community preset: " + p.string());

    util::warning("exceeded the maximum copy attempts; please delete or rename your local preset");

    return;
  }

  // Now we know that the preset is OK to be copied, but we first check for addons.
  if (!import_addons_from_community_package(preset_type, p, package)) {
    util::warning("can't import addons for the community preset: " + p.string() +
                  "; import stage aborted, please reload the community preset list");

    util::warning("if the issue goes on, contact the maintainer of the community package");

    return;
  }

  std::filesystem::copy_file(p, out_path);

  util::debug("successfully imported the community preset to: " + out_path.string());
}

void PresetsManager::add_autoload(const PresetType& preset_type,
                                  const std::string& preset_name,
                                  const std::string& device_name,
                                  const std::string& device_description,
                                  const std::string& device_profile) {
  nlohmann::json json;

  std::filesystem::path output_file;

  switch (preset_type) {
    case PresetType::output:
      output_file = autoload_output_dir / std::filesystem::path{device_name + ":" + device_profile + json_ext};
      break;
    case PresetType::input:
      output_file = autoload_input_dir / std::filesystem::path{device_name + ":" + device_profile + json_ext};
      break;
  }

  std::ofstream o(output_file.c_str());

  json["device"] = device_name;
  json["device-description"] = device_description;
  json["device-profile"] = device_profile;
  json["preset-name"] = preset_name;

  o << std::setw(4) << json << '\n';

  util::debug("added autoload preset file: " + output_file.string());
}

void PresetsManager::remove_autoload(const PresetType& preset_type,
                                     const std::string& preset_name,
                                     const std::string& device_name,
                                     const std::string& device_profile) {
  std::filesystem::path input_file;

  switch (preset_type) {
    case PresetType::output:
      input_file = autoload_output_dir / std::filesystem::path{device_name + ":" + device_profile + json_ext};
      break;
    case PresetType::input:
      input_file = autoload_input_dir / std::filesystem::path{device_name + ":" + device_profile + json_ext};
      break;
  }

  if (!std::filesystem::is_regular_file(input_file)) {
    return;
  }

  nlohmann::json json;

  std::ifstream is(input_file);

  is >> json;

  if (preset_name == json.value("preset-name", "") && device_profile == json.value("device-profile", "")) {
    std::filesystem::remove(input_file);

    util::debug("removed autoload: " + input_file.string());
  }
}

auto PresetsManager::find_autoload(const PresetType& preset_type,
                                   const std::string& device_name,
                                   const std::string& device_profile) -> std::string {
  std::filesystem::path input_file;

  switch (preset_type) {
    case PresetType::output:
      input_file = autoload_output_dir / std::filesystem::path{device_name + ":" + device_profile + json_ext};
      break;
    case PresetType::input:
      input_file = autoload_input_dir / std::filesystem::path{device_name + ":" + device_profile + json_ext};
      break;
  }

  if (!std::filesystem::is_regular_file(input_file)) {
    return "";
  }

  nlohmann::json json;

  std::ifstream is(input_file);

  is >> json;

  return json.value("preset-name", "");
}

void PresetsManager::autoload(const PresetType& preset_type,
                              const std::string& device_name,
                              const std::string& device_profile) {
  const auto name = find_autoload(preset_type, device_name, device_profile);

  if (name.empty()) {
    return;
  }

  util::debug("autoloading local preset " + name + " for device " + device_name);

  load_local_preset_file(preset_type, name);
}

auto PresetsManager::get_autoload_profiles(const PresetType& preset_type) -> std::vector<nlohmann::json> {
  std::filesystem::path autoload_dir;
  std::vector<nlohmann::json> list;

  switch (preset_type) {
    case PresetType::output:
      autoload_dir = autoload_output_dir;

      break;
    case PresetType::input:
      autoload_dir = autoload_input_dir;
      break;
  }

  auto it = std::filesystem::directory_iterator{autoload_dir};

  try {
    while (it != std::filesystem::directory_iterator{}) {
      if (std::filesystem::is_regular_file(it->status())) {
        if (it->path().extension().c_str() == json_ext) {
          nlohmann::json json;

          std::ifstream is(autoload_dir / it->path());

          is >> json;

          list.push_back(json);
        }
      }

      ++it;
    }

    return list;
  } catch (const std::exception& e) {
    util::warning(e.what());

    return list;
  }
}

void PresetsManager::set_last_preset_keys(const PresetType& preset_type,
                                          const std::string& preset_name,
                                          const std::string& package_name) {
  const auto* lp_key = (preset_type == PresetType::input) ? "last-loaded-input-preset" : "last-loaded-output-preset";

  const auto* lcp_key = (preset_type == PresetType::input) ? "last-loaded-input-community-package"
                                                           : "last-loaded-output-community-package";

  // In order to avoid race conditions, the community package key should be set before the preset name.
  if (package_name.empty()) {
    g_settings_reset(settings, lcp_key);
  } else {
    g_settings_set_string(settings, lcp_key, package_name.c_str());
  }

  if (preset_name.empty()) {
    g_settings_reset(settings, lp_key);
  } else {
    g_settings_set_string(settings, lp_key, preset_name.c_str());
  }
}

auto PresetsManager::preset_file_exists(const PresetType& preset_type, const std::string& name) -> bool {
  const auto conf_dir = (preset_type == PresetType::output) ? user_output_dir : user_input_dir;

  const auto input_file = conf_dir / std::filesystem::path{name + json_ext};

  return std::filesystem::exists(input_file);
}

void PresetsManager::notify_error(const PresetError& preset_error, const std::string& plugin_name) {
  std::string plugin_translated;

  try {
    const auto base_name = tags::plugin_name::get_base_name(plugin_name);
    plugin_translated = tags::plugin_name::get_translated().at(base_name) + ": ";
  } catch (std::out_of_range& e) {
    util::debug(e.what());
  }

  switch (preset_error) {
    case PresetError::blocklist_format: {
      util::warning(
          "A parsing error occurred while trying to load the blocklist from the preset. The file could be invalid "
          "or corrupted. Please check its content.");

      preset_load_error.emit(_("Preset Not Loaded Correctly"), _("Wrong Format in Excluded Apps List"));

      break;
    }
    case PresetError::blocklist_generic: {
      util::warning("A generic error occurred while trying to load the blocklist from the preset.");

      preset_load_error.emit(_("Preset Not Loaded Correctly"), _("Generic Error While Loading Excluded Apps List"));

      break;
    }
    case PresetError::pipeline_format: {
      util::warning(
          "A parsing error occurred while trying to load the pipeline from the preset. The file could be invalid "
          "or corrupted. Please check its content.");

      preset_load_error.emit(_("Preset Not Loaded Correctly"), _("Wrong Format in Effects List"));

      break;
    }
    case PresetError::pipeline_generic: {
      util::warning("A generic error occurred while trying to load the pipeline from the preset.");

      preset_load_error.emit(_("Preset Not Loaded Correctly"), _("Generic Error While Loading Effects List"));

      break;
    }
    case PresetError::plugin_format: {
      util::warning("A parsing error occurred while trying to load the " + plugin_name +
                    " plugin from the preset. The file could be invalid or "
                    "corrupted. Please check its content.");

      preset_load_error.emit(_("Preset Not Loaded Correctly"),
                             plugin_translated + _("One or More Parameters Have a Wrong Format"));

      break;
    }
    case PresetError::plugin_generic: {
      util::warning("A generic error occurred while trying to load the " + plugin_name + " plugin from the preset.");

      preset_load_error.emit(_("Preset Not Loaded Correctly"),
                             plugin_translated + _("Generic Error While Loading The Effect"));

      break;
    }
    default:
      break;
  }
}

auto PresetsManager::create_wrapper(const PresetType& preset_type, std::string_view filter_name)
    -> std::optional<std::unique_ptr<PluginPresetBase>> {
  auto instance_id = tags::plugin_name::get_id(std::string(filter_name));

  if (filter_name.starts_with(tags::plugin_name::autogain)) {
    return std::make_unique<AutoGainPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::bass_enhancer)) {
    return std::make_unique<BassEnhancerPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::bass_loudness)) {
    return std::make_unique<BassLoudnessPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::compressor)) {
    return std::make_unique<CompressorPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::convolver)) {
    return std::make_unique<ConvolverPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::crossfeed)) {
    return std::make_unique<CrossfeedPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::crystalizer)) {
    return std::make_unique<CrystalizerPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::deesser)) {
    return std::make_unique<DeesserPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::delay)) {
    return std::make_unique<DelayPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::deepfilternet)) {
    return std::make_unique<DeepFilterNetPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::echo_canceller)) {
    return std::make_unique<EchoCancellerPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::equalizer)) {
    return std::make_unique<EqualizerPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::exciter)) {
    return std::make_unique<ExciterPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::expander)) {
    return std::make_unique<ExpanderPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::filter)) {
    return std::make_unique<FilterPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::gate)) {
    return std::make_unique<GatePreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::level_meter)) {
    return std::make_unique<LevelMeterPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::limiter)) {
    return std::make_unique<LimiterPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::loudness)) {
    return std::make_unique<LoudnessPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::maximizer)) {
    return std::make_unique<MaximizerPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::multiband_compressor)) {
    return std::make_unique<MultibandCompressorPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::multiband_gate)) {
    return std::make_unique<MultibandGatePreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::pitch)) {
    return std::make_unique<PitchPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::reverb)) {
    return std::make_unique<ReverbPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::rnnoise)) {
    return std::make_unique<RNNoisePreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::speex)) {
    return std::make_unique<SpeexPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::stereo_tools)) {
    return std::make_unique<StereoToolsPreset>(preset_type, instance_id);
  }

  util::warning("The filter name " + std::string(filter_name) + " base name could not be recognized");

  return std::nullopt;
}
