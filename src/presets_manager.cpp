/*
 *  Copyright © 2017-2023 Wellington Wallace
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

PresetsManager::PresetsManager()
    : user_config_dir(g_get_user_config_dir()),
      user_presets_dir(user_config_dir + "/easyeffects/"),
      user_input_dir(user_config_dir + "/easyeffects/input"),
      user_output_dir(user_config_dir + "/easyeffects/output"),
      autoload_input_dir(user_config_dir + "/easyeffects/autoload/input"),
      autoload_output_dir(user_config_dir + "/easyeffects/autoload/output"),
      settings(g_settings_new(tags::app::id)),
      soe_settings(g_settings_new(tags::schema::id_output)),
      sie_settings(g_settings_new(tags::schema::id_input)) {
  // system presets directories provided by Glib

  for (const auto* scd = g_get_system_config_dirs(); *scd != nullptr; scd++) {
    system_input_dir.emplace_back(std::string(*scd) +
                                  "/easyeffects/input");  // it should be fine to use emplace_back in these cases
    system_output_dir.emplace_back(std::string(*scd) + "/easyeffects/output");
  }

  // add "/etc" to system config folders array and remove duplicates
  system_input_dir.emplace_back("/etc/easyeffects/input");
  system_output_dir.emplace_back("/etc/easyeffects/output");
  std::sort(system_input_dir.begin(), system_input_dir.end());
  std::sort(system_output_dir.begin(), system_output_dir.end());
  system_input_dir.erase(std::unique(system_input_dir.begin(), system_input_dir.end()), system_input_dir.end());
  system_output_dir.erase(std::unique(system_output_dir.begin(), system_output_dir.end()), system_output_dir.end());

  for (const auto& scd : system_input_dir) {
    util::debug("system input presets directory: \"" + scd.string() + "\"; ");
  }
  for (const auto& scd : system_output_dir) {
    util::debug("system output presets directory: \"" + scd.string() + "\"; ");
  }

  // user presets directories

  create_user_directory(user_presets_dir);
  create_user_directory(user_input_dir);
  create_user_directory(user_output_dir);
  create_user_directory(autoload_input_dir);
  create_user_directory(autoload_output_dir);

  auto* gfile = g_file_new_for_path(user_output_dir.c_str());

  user_output_monitor = g_file_monitor_directory(gfile, G_FILE_MONITOR_NONE, nullptr, nullptr);

  g_signal_connect(user_output_monitor, "changed",
                   G_CALLBACK(+[](GFileMonitor* monitor, GFile* file, GFile* other_file, GFileMonitorEvent event_type,
                                  gpointer user_data) {
                     auto self = static_cast<PresetsManager*>(user_data);

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
                     auto self = static_cast<PresetsManager*>(user_data);

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
                     auto self = static_cast<PresetsManager*>(user_data);

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
                     auto self = static_cast<PresetsManager*>(user_data);

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
  if (!std::filesystem::is_directory(path)) {
    if (std::filesystem::create_directories(path)) {
      util::debug("user presets directory created: " + path.string());

      return;
    }

    util::warning("failed to create user presets directory: " + path.string());

  } else {
    util::debug("user presets directory already exists: " + path.string());
  }
}

auto PresetsManager::get_names(const PresetType& preset_type) -> std::vector<std::string> {
  std::filesystem::directory_iterator it;
  std::vector<std::string> names;

  // system directories search
  std::vector<std::filesystem::path> sys_dirs;

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

      const auto vn = search_names(it);
      names.insert(names.end(), vn.begin(), vn.end());
    }
  }

  // user directory search
  const auto user_dir = (preset_type == PresetType::output) ? user_output_dir : user_input_dir;
  it = std::filesystem::directory_iterator{user_dir};

  const auto vn = search_names(it);
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
        if (it->path().extension().c_str() == json_ext) {
          names.emplace_back(it->path().stem().c_str());
        }
      }

      ++it;
    }
  } catch (const std::exception& e) {
    util::warning(e.what());
  }

  return names;
}

void PresetsManager::add(const PresetType& preset_type, const std::string& name) {
  for (const auto& p : get_names(preset_type)) {
    if (p == name) {
      return;
    }
  }

  save_preset_file(preset_type, name);
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

  o << std::setw(4) << json << std::endl;

  // std::cout << std::setw(4) << json << std::endl;

  util::debug("saved preset: " + output_file.string());
}

void PresetsManager::write_plugins_preset(const PresetType& preset_type,
                                          const std::vector<std::string>& plugins,
                                          nlohmann::json& json) {
  for (const auto& name : plugins) {
    if (auto wrapper = create_wrapper(preset_type, name); wrapper != std::nullopt) {
      wrapper.value()->write(json);
    }
  }
}

void PresetsManager::remove(const PresetType& preset_type, const std::string& name) {
  std::filesystem::path preset_file;

  const auto user_dir = (preset_type == PresetType::output) ? user_output_dir : user_input_dir;

  preset_file = user_dir / std::filesystem::path{name + json_ext};

  if (std::filesystem::exists(preset_file)) {
    std::filesystem::remove(preset_file);

    util::debug("removed preset: " + preset_file.string());
  }
}

auto PresetsManager::load_preset_file(const PresetType& preset_type, const std::string& name) -> bool {
  nlohmann::json json;

  std::vector<std::string> plugins;

  std::vector<std::filesystem::path> conf_dirs;

  std::filesystem::path input_file;

  auto preset_found = false;

  switch (preset_type) {
    case PresetType::output: {
      conf_dirs.push_back(user_output_dir);

      conf_dirs.insert(conf_dirs.end(), system_output_dir.begin(), system_output_dir.end());

      for (const auto& dir : conf_dirs) {
        input_file = dir / std::filesystem::path{name + json_ext};

        if (std::filesystem::exists(input_file)) {
          preset_found = true;

          break;
        }
      }

      if (preset_found) {
        try {
          std::ifstream is(input_file);

          is >> json;

          for (const auto& p : json.at("output").at("plugins_order").get<std::vector<std::string>>()) {
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

        g_settings_set_strv(soe_settings, "plugins", util::make_gchar_pointer_vector(plugins).data());
      } else {
        util::debug("can't find the preset " + name + " on the filesystem");

        return false;
      }

      break;
    }
    case PresetType::input: {
      conf_dirs.push_back(user_input_dir);

      conf_dirs.insert(conf_dirs.end(), system_input_dir.begin(), system_input_dir.end());

      for (const auto& dir : conf_dirs) {
        input_file = dir / std::filesystem::path{name + json_ext};

        if (std::filesystem::exists(input_file)) {
          preset_found = true;

          break;
        }
      }

      if (preset_found) {
        try {
          std::ifstream is(input_file);

          is >> json;

          for (const auto& p : json.at("input").at("plugins_order").get<std::vector<std::string>>()) {
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

        g_settings_set_strv(sie_settings, "plugins", util::make_gchar_pointer_vector(plugins).data());
      } else {
        util::debug("can't find the preset " + name + " on the filesystem");

        return false;
      }

      break;
    }
  }

  if (load_blocklist(preset_type, json) && read_plugins_preset(preset_type, plugins, json)) {
    util::debug("successfully loaded preset: " + input_file.string());

    return true;
  }

  return false;
}

auto PresetsManager::read_plugins_preset(const PresetType& preset_type,
                                         const std::vector<std::string>& plugins,
                                         const nlohmann::json& json) -> bool {
  for (const auto& name : plugins) {
    if (auto wrapper = create_wrapper(preset_type, name); wrapper != std::nullopt) {
      try {
        wrapper.value()->read(json);
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

void PresetsManager::import(const PresetType& preset_type, const std::string& file_path) {
  std::filesystem::path p{file_path};

  if (std::filesystem::is_regular_file(p)) {
    if (p.extension().c_str() == json_ext) {
      std::filesystem::path out_path;

      const auto user_dir = (preset_type == PresetType::output) ? user_output_dir : user_input_dir;

      out_path = user_dir / p.filename();

      std::filesystem::copy_file(p, out_path, std::filesystem::copy_options::overwrite_existing);

      util::debug("imported preset to: " + out_path.string());
    }
  } else {
    util::warning(p.string() + " is not a file!");
  }
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

  o << std::setw(4) << json << std::endl;

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

  if (std::filesystem::is_regular_file(input_file)) {
    nlohmann::json json;

    std::ifstream is(input_file);

    is >> json;

    if (preset_name == json.value("preset-name", "") && device_profile == json.value("device-profile", "")) {
      std::filesystem::remove(input_file);

      util::debug("removed autoload: " + input_file.string());
    }
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

  if (std::filesystem::is_regular_file(input_file)) {
    nlohmann::json json;

    std::ifstream is(input_file);

    is >> json;

    return json.value("preset-name", "");
  }

  return "";
}

void PresetsManager::autoload(const PresetType& preset_type,
                              const std::string& device_name,
                              const std::string& device_profile) {
  const auto name = find_autoload(preset_type, device_name, device_profile);

  if (!name.empty()) {
    util::debug("autoloading preset " + name + " for device " + device_name);

    const auto* key = (preset_type == PresetType::output) ? "last-used-output-preset" : "last-used-input-preset";

    if (load_preset_file(preset_type, name)) {
      g_settings_set_string(settings, key, name.c_str());
    } else {
      g_settings_reset(settings, key);
    }
  }
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

auto PresetsManager::preset_file_exists(const PresetType& preset_type, const std::string& name) -> bool {
  std::filesystem::path input_file;
  std::vector<std::filesystem::path> conf_dirs;

  switch (preset_type) {
    case PresetType::output: {
      conf_dirs.push_back(user_output_dir);

      conf_dirs.insert(conf_dirs.end(), system_output_dir.begin(), system_output_dir.end());

      for (const auto& dir : conf_dirs) {
        input_file = dir / std::filesystem::path{name + json_ext};

        if (std::filesystem::exists(input_file)) {
          return true;
        }
      }

      break;
    }
    case PresetType::input: {
      conf_dirs.push_back(user_input_dir);

      conf_dirs.insert(conf_dirs.end(), system_input_dir.begin(), system_input_dir.end());

      for (const auto& dir : conf_dirs) {
        input_file = dir / std::filesystem::path{name + json_ext};

        if (std::filesystem::exists(input_file)) {
          return true;
        }
      }

      break;
    }
  }

  return false;
}

void PresetsManager::notify_error(const PresetError& preset_error, const std::string& plugin_name) {
  std::string plugin_translated;

  try {
    plugin_translated = tags::plugin_name::get_translated().at(plugin_name);
  } catch (...) {
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
                             plugin_translated + ": " + _("One or More Parameters Have a Wrong Format"));

      break;
    }
    case PresetError::plugin_generic: {
      util::warning("A generic error occurred while trying to load the " + plugin_name + " plugin from the preset.");

      preset_load_error.emit(_("Preset Not Loaded Correctly"),
                             plugin_translated + ": " + _("Generic Error While Loading The Effect"));

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

  if (filter_name.starts_with(tags::plugin_name::echo_canceller)) {
    return std::make_unique<EchoCancellerPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::equalizer)) {
    return std::make_unique<EqualizerPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::exciter)) {
    return std::make_unique<ExciterPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::filter)) {
    return std::make_unique<FilterPreset>(preset_type, instance_id);
  }

  if (filter_name.starts_with(tags::plugin_name::gate)) {
    return std::make_unique<GatePreset>(preset_type, instance_id);
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