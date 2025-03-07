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
#include <qcontainerfwd.h>
#include <qfilesystemwatcher.h>
#include <qqml.h>
#include <qsortfilterproxymodel.h>
#include <qstandardpaths.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <qurl.h>
#include <KLocalizedString>
#include <QString>
#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <sndfile.hh>
#include <stdexcept>
#include <string>
#include <vector>
#include "autogain_preset.hpp"
#include "bass_enhancer_preset.hpp"
#include "compressor_preset.hpp"
#include "config.h"
#include "convolver_preset.hpp"
#include "crossfeed_preset.hpp"
#include "crystalizer_preset.hpp"
#include "easyeffects_db.h"
#include "easyeffects_db_streaminputs.h"
#include "easyeffects_db_streamoutputs.h"
#include "exciter_preset.hpp"
#include "filter_preset.hpp"
#include "gate_preset.hpp"
#include "limiter_preset.hpp"
#include "maximizer_preset.hpp"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_list_model.hpp"
#include "rnnoise_preset.hpp"
#include "speex_preset.hpp"
#include "stereo_tools_preset.hpp"
#include "tags_app.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

namespace presets {

Manager::Manager()
    : app_config_dir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation).toStdString()),
      user_input_dir(app_config_dir + "/input"),
      user_output_dir(app_config_dir + "/output"),
      user_irs_dir(app_config_dir + "/irs"),
      user_rnnoise_dir(app_config_dir + "/rnnoise"),
      autoload_input_dir(app_config_dir + "/autoload/input"),
      autoload_output_dir(app_config_dir + "/autoload/output"),
      outputListModel(new ListModel(this)),
      inputListModel(new ListModel(this)),
      communityOutputListModel(new ListModel(this, ListModel::ModelType::Community)),
      communityInputListModel(new ListModel(this, ListModel::ModelType::Community)),
      autoloadingOutputListmodel(new ListModel(this, ListModel::ModelType::Autoloading)),
      autoloadingInputListmodel(new ListModel(this, ListModel::ModelType::Autoloading)),
      irsListModel(new ListModel(this, ListModel::ModelType::IRS)),
      rnnoiseListModel(new ListModel(this, ListModel::ModelType::RNNOISE)) {
  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDelete)
  qmlRegisterSingletonInstance<presets::Manager>("ee.presets", VERSION_MAJOR, VERSION_MINOR, "Manager", this);

  qmlRegisterSingletonInstance<QSortFilterProxyModel>("ee.presets", VERSION_MAJOR, VERSION_MINOR,
                                                      "SortedInputListModel", inputListModel->getProxy());

  qmlRegisterSingletonInstance<QSortFilterProxyModel>("ee.presets", VERSION_MAJOR, VERSION_MINOR,
                                                      "SortedOutputListModel", outputListModel->getProxy());

  qmlRegisterSingletonInstance<QSortFilterProxyModel>("ee.presets", VERSION_MAJOR, VERSION_MINOR,
                                                      "SortedCommunityOutputListModel",
                                                      communityOutputListModel->getProxy());

  qmlRegisterSingletonInstance<QSortFilterProxyModel>(
      "ee.presets", VERSION_MAJOR, VERSION_MINOR, "SortedCommunityInputListModel", communityInputListModel->getProxy());

  qmlRegisterSingletonInstance<QSortFilterProxyModel>("ee.presets", VERSION_MAJOR, VERSION_MINOR,
                                                      "SortedAutoloadingInputListModel",
                                                      autoloadingInputListmodel->getProxy());

  qmlRegisterSingletonInstance<QSortFilterProxyModel>("ee.presets", VERSION_MAJOR, VERSION_MINOR,
                                                      "SortedAutoloadingOutputListModel",
                                                      autoloadingOutputListmodel->getProxy());

  qmlRegisterSingletonInstance<QSortFilterProxyModel>("ee.presets", VERSION_MAJOR, VERSION_MINOR,
                                                      "SortedImpulseListModel", irsListModel->getProxy());

  qmlRegisterSingletonInstance<QSortFilterProxyModel>("ee.presets", VERSION_MAJOR, VERSION_MINOR,
                                                      "SortedRNNoiseListModel", rnnoiseListModel->getProxy());
  // NOLINTEND(clang-analyzer-cplusplus.NewDelete)

  // Initialize input and output directories for community presets.
  // Flatpak specific path (.flatpak-info always present for apps running in the flatpak sandbox).
  if (std::filesystem::is_regular_file(tags::app::flatpak_info_file)) {
    system_data_dir_input.emplace_back("/app/extensions/Presets/input");
    system_data_dir_output.emplace_back("/app/extensions/Presets/output");
    system_data_dir_irs.emplace_back("/app/extensions/Presets/irs");
    system_data_dir_rnnoise.emplace_back("/app/extensions/Presets/rnnoise");
  }

  // Regular paths.
  for (auto& dir : QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)) {
    dir += dir.endsWith("/") ? "" : "/";

    system_data_dir_input.push_back(dir.toStdString() + "input");
    system_data_dir_output.push_back(dir.toStdString() + "output");
    system_data_dir_irs.push_back(dir.toStdString() + "irs");
    system_data_dir_rnnoise.push_back(dir.toStdString() + "rnnoise");
  }

  // create user presets directories

  create_user_directory(user_input_dir);
  create_user_directory(user_output_dir);
  create_user_directory(user_irs_dir);
  create_user_directory(user_rnnoise_dir);
  create_user_directory(autoload_input_dir);
  create_user_directory(autoload_output_dir);

  refresh_list_models(inputListModel, [this]() { return get_local_presets_paths(PipelineType::input); });
  refresh_list_models(outputListModel, [this]() { return get_local_presets_paths(PipelineType::output); });

  refresh_list_models(autoloadingInputListmodel,
                      [this]() { return get_autoloading_profiles_paths(PipelineType::input); });
  refresh_list_models(autoloadingOutputListmodel,
                      [this]() { return get_autoloading_profiles_paths(PipelineType::output); });

  refresh_list_models(irsListModel, [this]() { return get_local_irs_paths(); });

  refresh_list_models(rnnoiseListModel, [this]() { return get_local_rnnoise_paths(); });

  refreshCommunityPresets(PipelineType::input);
  refreshCommunityPresets(PipelineType::output);

  prepare_filesystem_watchers();
  prepare_last_used_preset_key(PipelineType::input);
  prepare_last_used_preset_key(PipelineType::output);
}

void Manager::create_user_directory(const std::filesystem::path& path) {
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

void Manager::refresh_list_models(ListModel* model, std::function<QList<std::filesystem::path>()> get_paths) {
  auto model_list = model->getList();
  auto local_list = get_paths();

  model->begin_reset();

  for (const auto& v : local_list) {
    if (!model_list.contains(v)) {
      model->append(v);
    }
  }

  for (const auto& v : model_list) {
    if (!local_list.contains(v)) {
      model->remove(v);
    }
  }

  model->end_reset();
}

void Manager::prepare_filesystem_watchers() {
  user_input_watcher.addPath(QString::fromStdString(user_input_dir.string()));
  user_output_watcher.addPath(QString::fromStdString(user_output_dir.string()));
  autoload_input_watcher.addPath(QString::fromStdString(autoload_input_dir.string()));
  autoload_output_watcher.addPath(QString::fromStdString(autoload_output_dir.string()));
  irs_watcher.addPath(QString::fromStdString(user_irs_dir.string()));
  rnnoise_watcher.addPath(QString::fromStdString(user_rnnoise_dir.string()));

  connect(&user_input_watcher, &QFileSystemWatcher::directoryChanged, [&]() {
    refresh_list_models(inputListModel, [this]() { return get_local_presets_paths(PipelineType::input); });
  });

  connect(&user_output_watcher, &QFileSystemWatcher::directoryChanged, [&]() {
    refresh_list_models(outputListModel, [this]() { return get_local_presets_paths(PipelineType::output); });
  });

  connect(&autoload_input_watcher, &QFileSystemWatcher::directoryChanged, [&]() {
    refresh_list_models(autoloadingInputListmodel,
                        [this]() { return get_autoloading_profiles_paths(PipelineType::input); });
  });

  connect(&autoload_output_watcher, &QFileSystemWatcher::directoryChanged, [&]() {
    refresh_list_models(autoloadingOutputListmodel,
                        [this]() { return get_autoloading_profiles_paths(PipelineType::output); });
  });

  connect(&irs_watcher, &QFileSystemWatcher::directoryChanged,
          [&]() { refresh_list_models(irsListModel, [this]() { return get_local_irs_paths(); }); });

  connect(&rnnoise_watcher, &QFileSystemWatcher::directoryChanged,
          [&]() { refresh_list_models(rnnoiseListModel, [this]() { return get_local_rnnoise_paths(); }); });
}

void Manager::prepare_last_used_preset_key(const PipelineType& pipeline_type) {
  const auto preset_name =
      pipeline_type == PipelineType::input ? db::Main::lastLoadedInputPreset() : db::Main::lastLoadedOutputPreset();

  bool reset_key = true;

  if (!preset_name.isEmpty()) {
    for (const auto& p : get_local_presets_paths(pipeline_type)) {
      if (p.stem().string() == preset_name.toStdString()) {
        reset_key = false;

        break;
      }
    }
  } else {
    reset_key = false;
  }

  if (reset_key) {
    switch (pipeline_type) {
      case PipelineType::input:
        db::Main::setLastLoadedInputPreset(db::Main::defaultLastLoadedInputPresetValue());
        break;
      case PipelineType::output:
        db::Main::setLastLoadedOutputPreset(db::Main::defaultLastLoadedOutputPresetValue());
        break;
    }
  }
}

auto Manager::search_presets_path(std::filesystem::directory_iterator& it, const std::string& file_extension)
    -> QList<std::filesystem::path> {
  QList<std::filesystem::path> paths;

  try {
    while (it != std::filesystem::directory_iterator{}) {
      if (std::filesystem::is_regular_file(it->status()) && it->path().extension().string() == file_extension) {
        paths.append(it->path());
      }

      ++it;
    }
  } catch (const std::exception& e) {
    util::warning(e.what());
  }

  return paths;
}

auto Manager::get_local_presets_paths(const PipelineType& pipeline_type) -> QList<std::filesystem::path> {
  const auto conf_dir = (pipeline_type == PipelineType::output) ? user_output_dir : user_input_dir;

  auto it = std::filesystem::directory_iterator{conf_dir};

  auto paths = search_presets_path(it);

  return paths;
}

auto Manager::get_local_irs_paths() -> QList<std::filesystem::path> {
  auto it = std::filesystem::directory_iterator{user_irs_dir};

  auto paths = search_presets_path(it, irs_ext);

  return paths;
}

auto Manager::get_local_rnnoise_paths() -> QList<std::filesystem::path> {
  auto it = std::filesystem::directory_iterator{user_rnnoise_dir};

  auto paths = search_presets_path(it, rnnoise_ext);

  return paths;
}

auto Manager::get_autoloading_profiles_paths(const PipelineType& pipeline_type) -> QList<std::filesystem::path> {
  const auto conf_dir = (pipeline_type == PipelineType::output) ? autoload_output_dir : autoload_input_dir;

  auto it = std::filesystem::directory_iterator{conf_dir};

  auto paths = search_presets_path(it);

  return paths;
}

auto Manager::get_all_community_presets_paths(const PipelineType& pipeline_type) -> QList<std::filesystem::path> {
  QList<std::filesystem::path> cp_paths;

  const auto scan_level = 2U;

  const auto cp_dir_vect = (pipeline_type == PipelineType::output) ? system_data_dir_output : system_data_dir_input;

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

          const auto sub_cp_vect =
              scan_community_package_recursive(package_it, scan_level, QString::fromStdString(package_path_name));

          cp_paths.append(sub_cp_vect);
        }

        ++it;
      }
    } catch (const std::exception& e) {
      util::warning(e.what());
    }
  }

  return cp_paths;
}

auto Manager::scan_community_package_recursive(std::filesystem::directory_iterator& it,
                                               const uint& top_scan_level,
                                               const QString& origin) -> QList<std::filesystem::path> {
  const auto scan_level = top_scan_level - 1U;

  QList<std::filesystem::path> cp_paths;

  try {
    while (it != std::filesystem::directory_iterator{}) {
      if (std::filesystem::is_regular_file(it->status()) && it->path().extension().string() == json_ext) {
        // cp_paths.append(origin + "/" + QString::fromStdString(it->path().stem().string()));
        cp_paths.append(it->path());
      } else if (scan_level > 0U && std::filesystem::is_directory(it->status())) {
        if (auto path = it->path(); !path.empty()) {
          auto subdir_it = std::filesystem::directory_iterator{path};

          const auto sub_cp_vect = scan_community_package_recursive(
              subdir_it, scan_level, origin + "/" + QString::fromStdString(path.filename().string()));

          cp_paths.append(sub_cp_vect);
        }
      }

      ++it;
    }
  } catch (const std::exception& e) {
    util::warning(e.what());
  }

  return cp_paths;
}

void Manager::refreshCommunityPresets(const PipelineType& pipeline_type) {
  switch (pipeline_type) {
    case PipelineType::input: {
      refresh_list_models(communityInputListModel,
                          [this]() { return get_all_community_presets_paths(PipelineType::input); });

      break;
    }
    case PipelineType::output: {
      refresh_list_models(communityOutputListModel,
                          [this]() { return get_all_community_presets_paths(PipelineType::output); });

      break;
    }
  }
}

void Manager::save_blocklist(const PipelineType& pipeline_type, nlohmann::json& json) {
  std::vector<std::string> blocklist;

  switch (pipeline_type) {
    case PipelineType::output: {
      const auto list = db::StreamOutputs::blocklist();

      for (const auto& l : list) {
        blocklist.push_back(l.toStdString());
      }

      json["output"]["blocklist"] = blocklist;

      break;
    }
    case PipelineType::input: {
      const auto list = db::StreamInputs::blocklist();

      for (const auto& l : list) {
        blocklist.push_back(l.toStdString());
      }

      json["input"]["blocklist"] = blocklist;

      break;
    }
  }
}

auto Manager::load_blocklist(const PipelineType& pipeline_type, const nlohmann::json& json) -> bool {
  std::vector<std::string> blocklist;

  switch (pipeline_type) {
    case PipelineType::input: {
      try {
        auto list = json.at("input").at("blocklist").get<std::vector<std::string>>();

        auto new_list = QStringList();

        for (const auto& app : list) {
          new_list.append(QString::fromStdString(app));
        }

        db::StreamInputs::setBlocklist(new_list);
      } catch (const nlohmann::json::exception& e) {
        db::StreamInputs::setBlocklist(QStringList{});

        notify_error(PresetError::blocklist_format);

        util::warning(e.what());

        return false;
      } catch (...) {
        db::StreamInputs::setBlocklist(QStringList{});

        notify_error(PresetError::blocklist_generic);

        return false;
      }

      break;
    }
    case PipelineType::output: {
      try {
        auto list = json.at("output").at("blocklist").get<std::vector<std::string>>();

        auto new_list = QStringList();

        for (const auto& app : list) {
          new_list.append(QString::fromStdString(app));
        }

        db::StreamOutputs::setBlocklist(new_list);
      } catch (const nlohmann::json::exception& e) {
        db::StreamOutputs::setBlocklist(QStringList{});

        notify_error(PresetError::blocklist_format);

        util::warning(e.what());

        return false;
      } catch (...) {
        db::StreamOutputs::setBlocklist(QStringList{});

        notify_error(PresetError::blocklist_generic);

        return false;
      }

      break;
    }
  }

  return true;
}

bool Manager::savePresetFile(const PipelineType& pipeline_type, const QString& name) {
  /*
    Todo: we have to test if the file was really save instead of assuming we always succeed
  */

  nlohmann::json json;

  std::filesystem::path output_file;

  save_blocklist(pipeline_type, json);

  switch (pipeline_type) {
    case PipelineType::output: {
      const auto plugins = db::StreamOutputs::plugins();

      std::vector<std::string> list;

      list.reserve(plugins.size());

      for (const auto& p : plugins) {
        list.push_back(p.toStdString());
      }

      json["output"]["plugins_order"] = list;

      write_plugins_preset(pipeline_type, plugins, json);

      output_file = user_output_dir / std::filesystem::path{name.toStdString() + json_ext};

      break;
    }
    case PipelineType::input: {
      const auto plugins = db::StreamInputs::plugins();

      std::vector<std::string> list;

      list.reserve(plugins.size());

      for (const auto& p : plugins) {
        list.push_back(p.toStdString());
      }

      json["input"]["plugins_order"] = list;

      write_plugins_preset(pipeline_type, plugins, json);

      output_file = user_input_dir / std::filesystem::path{name.toStdString() + json_ext};

      break;
    }
  }

  std::ofstream o(output_file.string());

  o << std::setw(4) << json << '\n';

  // std::cout << std::setw(4) << json << std::endl;

  util::debug("saved preset: " + output_file.string());

  return true;
}

bool Manager::add(const PipelineType& pipeline_type, const QString& name) {
  // This method assumes the filename is valid.

  for (const auto& p : get_local_presets_paths(pipeline_type)) {
    if (p.stem().string() == name.toStdString()) {
      return false;
    }
  }

  savePresetFile(pipeline_type, name);

  return true;
}

bool Manager::remove(const PipelineType& pipeline_type, const QString& name) {
  std::filesystem::path preset_file;

  const auto conf_dir = (pipeline_type == PipelineType::output) ? user_output_dir : user_input_dir;

  preset_file = conf_dir / std::filesystem::path{name.toStdString() + json_ext};

  if (std::filesystem::exists(preset_file)) {
    std::filesystem::remove(preset_file);

    util::debug("removed preset: " + preset_file.string());

    return true;
  }

  return false;
}

auto Manager::read_effects_pipeline_from_preset(const PipelineType& pipeline_type,
                                                const std::filesystem::path& input_file,
                                                nlohmann::json& json,
                                                std::vector<std::string>& plugins) -> bool {
  const auto* pipeline_type_str = (pipeline_type == PipelineType::input) ? "input" : "output";

  try {
    std::ifstream is(input_file);

    is >> json;

    for (const auto& p : json.at(pipeline_type_str).at("plugins_order").get<std::vector<std::string>>()) {
      for (const auto& v : tags::plugin_name::Model::self().getBaseNames()) {
        if (p.starts_with(v.toStdString())) {
          /*
            Old format presets do not have the instance id number in the filter names. They are equal to the
            base name.
          */

          if (p != v.toStdString()) {
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

  auto new_list = QStringList();

  for (const auto& app : plugins) {
    new_list.append(QString::fromStdString(app));
  }

  switch (pipeline_type) {
    case PipelineType::input:
      db::StreamInputs::setPlugins(new_list);
      break;
    case PipelineType::output:
      db::StreamOutputs::setPlugins(new_list);
      break;
  }

  return true;
}

auto Manager::read_plugins_preset(const PipelineType& pipeline_type,
                                  const std::vector<std::string>& plugins,
                                  const nlohmann::json& json) -> bool {
  for (const auto& name : plugins) {
    if (auto wrapper = create_wrapper(pipeline_type, QString::fromStdString(name)); wrapper != std::nullopt) {
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

void Manager::write_plugins_preset(const PipelineType& pipeline_type,
                                   const QStringList& plugins,
                                   nlohmann::json& json) {
  for (const auto& name : plugins) {
    if (auto wrapper = create_wrapper(pipeline_type, name); wrapper != std::nullopt) {
      if (wrapper.has_value()) {
        wrapper.value()->write(json);
      }
    }
  }
}

auto Manager::load_preset_file(const PipelineType& pipeline_type, const std::filesystem::path& input_file) -> bool {
  nlohmann::json json;

  std::vector<std::string> plugins;

  // Read effects_pipeline
  if (!read_effects_pipeline_from_preset(pipeline_type, input_file, json, plugins)) {
    return false;
  }

  // After the plugin order list, load the blocklist and then
  // apply the parameters of the loaded plugins.
  if (load_blocklist(pipeline_type, json) && read_plugins_preset(pipeline_type, plugins, json)) {
    util::debug("successfully loaded the preset: " + input_file.string());

    return true;
  }

  return false;
}

bool Manager::loadLocalPresetFile(const PipelineType& pipeline_type, const QString& name) {
  const auto conf_dir = (pipeline_type == PipelineType::output) ? user_output_dir : user_input_dir;

  const auto input_file = conf_dir / std::filesystem::path{name.toStdString() + json_ext};

  // Check preset existence
  if (!std::filesystem::exists(input_file)) {
    util::debug("can't find the local preset \"" + name.toStdString() + "\" on the filesystem");

    return false;
  }

  set_last_preset_keys(pipeline_type, name);

  const auto loaded = load_preset_file(pipeline_type, input_file);

  if (!loaded) {
    set_last_preset_keys(pipeline_type);
  }

  return loaded;
}

bool Manager::loadCommunityPresetFile(const PipelineType& pipeline_type,
                                      const QString& file_path,
                                      const QString& package_name) {
  const auto input_file = std::filesystem::path{file_path.toStdString()};

  // Check preset existence
  if (!std::filesystem::exists(input_file)) {
    util::warning("the community preset \"" + input_file.string() + "\" does not exist on the filesystem");

    return false;
  }

  set_last_preset_keys(pipeline_type, QString::fromStdString(input_file.stem().string()), package_name);

  const auto loaded = load_preset_file(pipeline_type, input_file);

  if (!loaded) {
    set_last_preset_keys(pipeline_type);
  }

  return loaded;
}

bool Manager::importPresets(const PipelineType& pipeline_type, const QList<QString>& url_list) {
  return std::ranges::all_of(url_list, [&](auto u) {
    auto url = QUrl(u);

    if (url.isLocalFile()) {
      auto input_path = std::filesystem::path{url.toLocalFile().toStdString()};

      const auto conf_dir = (pipeline_type == PipelineType::output) ? user_output_dir : user_input_dir;

      const std::filesystem::path out_path = conf_dir / input_path.filename();

      try {
        std::filesystem::copy_file(input_path, out_path, std::filesystem::copy_options::overwrite_existing);

        util::debug("imported preset to: " + out_path.string());

        return true;
      } catch (const std::exception& e) {
        util::warning("can't import preset to: " + out_path.string());
        util::warning(e.what());

        return false;
      }
    } else {
      util::warning(url.toString().toStdString() + " is not a local file!");

      return false;
    }

    return false;
  });
}

auto Manager::import_irs_file(const std::string& file_path) -> ImpulseImportState {
  std::filesystem::path p{file_path};

  if (!std::filesystem::is_regular_file(p)) {
    util::warning(p.string() + " is not a file!");

    return ImpulseImportState::no_regular_file;
  }

  auto file = SndfileHandle(file_path);

  if (file.frames() == 0) {
    util::warning("Cannot import the impulse response! The format may be corrupted or unsupported.");
    util::warning(file_path + " loading failed");

    return ImpulseImportState::no_frame;
  }

  if (file.channels() != 2) {
    util::warning("Only stereo impulse files are supported!");
    util::warning(file_path + " loading failed");

    return ImpulseImportState::no_stereo;
  }

  auto out_path = user_irs_dir / p.filename();

  out_path.replace_extension(irs_ext);

  std::filesystem::copy_file(p, out_path, std::filesystem::copy_options::overwrite_existing);

  util::debug("Irs file successfully imported to: " + out_path.string());

  return ImpulseImportState::success;
}

int Manager::importImpulses(const QList<QString>& url_list) {
  for (const auto& u : url_list) {
    auto url = QUrl(u);

    if (url.isLocalFile()) {
      auto path = std::filesystem::path{url.toLocalFile().toStdString()};

      if (auto import_state = import_irs_file(path); import_state != ImpulseImportState::success) {
        return static_cast<int>(import_state);
      }
    }
  }

  return static_cast<int>(ImpulseImportState::success);
}

auto Manager::import_rnnoise_file(const std::string& file_path) -> RNNoiseImportState {
  std::filesystem::path p{file_path};

  if (!std::filesystem::is_regular_file(p)) {
    util::warning(p.string() + " is not a file!");

    return RNNoiseImportState::no_regular_file;
  }

  auto out_path = user_rnnoise_dir / p.filename();

  out_path.replace_extension(rnnoise_ext);

  std::filesystem::copy_file(p, out_path, std::filesystem::copy_options::overwrite_existing);

  util::debug("Irs file successfully imported to: " + out_path.string());

  return RNNoiseImportState::success;
}

int Manager::importRNNoiseModel(const QList<QString>& url_list) {
  for (const auto& u : url_list) {
    auto url = QUrl(u);

    if (url.isLocalFile()) {
      auto path = std::filesystem::path{url.toLocalFile().toStdString()};

      if (auto import_state = import_rnnoise_file(path); import_state != RNNoiseImportState::success) {
        return static_cast<int>(import_state);
      }
    }
  }

  return static_cast<int>(RNNoiseImportState::success);
}

void Manager::removeImpulseFile(const QString& filePath) {
  if (std::filesystem::exists(filePath.toStdString())) {
    std::filesystem::remove(filePath.toStdString());

    util::debug("removed irs file: " + filePath.toStdString());
  }
}

void Manager::removeRNNoiseModel(const QString& filePath) {
  if (std::filesystem::exists(filePath.toStdString())) {
    std::filesystem::remove(filePath.toStdString());

    util::debug("removed the rnnoise model: " + filePath.toStdString());
  }
}

auto Manager::import_addons_from_community_package(const PipelineType& pipeline_type,
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

    std::vector<std::string> conv_irs;
    std::vector<std::string> rn_models;

    const auto* pt_key = (pipeline_type == PipelineType::output) ? "output" : "input";

    // Fill conv_irs and rn_models vectors extracting the addon names from
    // the json preset and append the respective file extension.
    for (const auto& plugin : json.at(pt_key).at("plugins_order").get<std::vector<std::string>>()) {
      if (plugin.starts_with(tags::plugin_name::BaseName::convolver.toStdString())) {
        conv_irs.push_back(json.at(pt_key).at(plugin).at("kernel-name").get<std::string>() + irs_ext);
      }

      if (plugin.starts_with(tags::plugin_name::BaseName::rnnoise.toStdString())) {
        rn_models.push_back(json.at(pt_key).at(plugin).at("model-name").get<std::string>() + rnnn_ext);
      }
    }

    // For every filename of both vectors, search the full path and copy the file locally.
    for (const auto& irs_name : conv_irs) {
      std::string path;

      bool found = false;

      for (auto xdg_dir : system_data_dir_irs) {
        xdg_dir.append("/");
        xdg_dir.append(package);

        if (util::search_filename(std::filesystem::path{xdg_dir}, irs_name, path, 3U)) {
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

      for (auto xdg_dir : system_data_dir_rnnoise) {
        xdg_dir.append("/");
        xdg_dir.append(package);

        if (util::search_filename(std::filesystem::path{xdg_dir}, model_name, path, 3U)) {
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

bool Manager::importFromCommunityPackage(const PipelineType& pipeline_type,
                                         const QString& file_path,
                                         const QString& package) {
  // When importing presets from a community package, we do NOT overwrite
  // the local preset if it has the same name.

  std::filesystem::path p{file_path.toStdString()};

  if (!std::filesystem::exists(p) || package.isEmpty()) {
    util::warning(p.string() + " does not exist! Please reload the community preset list");

    return false;
  }

  if (!std::filesystem::is_regular_file(p)) {
    util::warning(p.string() + " is not a file! Please reload the community preset list");

    return false;
  }

  if (p.extension().string() != json_ext) {
    return false;
  }

  bool preset_can_be_copied = false;

  // We limit the max copy attempts in order to not flood the local directory
  // if the user keeps clicking the import button.
  uint i = 0U;

  static const auto max_copy_attempts = 10;

  const auto conf_dir = (pipeline_type == PipelineType::output) ? user_output_dir.string() : user_input_dir.string();

  std::filesystem::path out_path;

  try {
    do {
      // In case of destination file already existing, we try to append
      // an incremental numeric suffix.
      const auto suffix = (i == 0U) ? "" : "-" + util::to_string(i);

      out_path = conf_dir + "/" + p.stem().string() + suffix + json_ext;

      if (!std::filesystem::exists(out_path)) {
        preset_can_be_copied = true;

        break;
      }
    } while (++i < max_copy_attempts);
  } catch (const std::exception& e) {
    util::warning("can't import the community preset: " + p.string());

    util::warning(e.what());

    return false;
  }

  if (!preset_can_be_copied) {
    util::warning("can't import the community preset: " + p.string());

    util::warning("exceeded the maximum copy attempts; please delete or rename your local preset");

    return false;
  }

  // Now we know that the preset is OK to be copied, but we first check for addons.
  if (!import_addons_from_community_package(pipeline_type, p, package.toStdString())) {
    util::warning("can't import addons for the community preset: " + p.string() +
                  "; import stage aborted, please reload the community preset list");

    util::warning("if the issue goes on, contact the maintainer of the community package");

    return false;
  }

  std::filesystem::copy_file(p, out_path);

  util::debug("successfully imported the community preset to: " + out_path.string());

  return true;
}

void Manager::addAutoload(const PipelineType& pipelineType,
                          const QString& presetName,
                          const QString& deviceName,
                          const QString& deviceDescription,
                          const QString& deviceProfile) {
  nlohmann::json json;

  std::filesystem::path output_file;

  switch (pipelineType) {
    case PipelineType::output:
      output_file = autoload_output_dir /
                    std::filesystem::path{deviceName.toStdString() + ":" + deviceProfile.toStdString() + json_ext};
      break;
    case PipelineType::input:
      output_file = autoload_input_dir /
                    std::filesystem::path{deviceName.toStdString() + ":" + deviceProfile.toStdString() + json_ext};
      break;
  }

  bool already_exists = std::filesystem::exists(output_file);

  std::ofstream o(output_file);

  json["device"] = deviceName.toStdString();
  json["device-description"] = deviceDescription.toStdString();
  json["device-profile"] = deviceProfile.toStdString();
  json["preset-name"] = presetName.toStdString();

  o << std::setw(4) << json << '\n';

  util::debug("added autoload preset file: " + output_file.string());

  o.close();

  if (already_exists) {
    switch (pipelineType) {
      case PipelineType::output:
        autoloadingOutputListmodel->emit_data_changed(output_file);
        break;
      case PipelineType::input:
        autoloadingInputListmodel->emit_data_changed(output_file);
        break;
    }
  }
}

void Manager::removeAutoload(const PipelineType& pipelineType,
                             const QString& presetName,
                             const QString& deviceName,
                             const QString& deviceProfile) {
  std::filesystem::path input_file;

  switch (pipelineType) {
    case PipelineType::output:
      input_file = autoload_output_dir /
                   std::filesystem::path{deviceName.toStdString() + ":" + deviceProfile.toStdString() + json_ext};
      break;
    case PipelineType::input:
      input_file = autoload_input_dir /
                   std::filesystem::path{deviceName.toStdString() + ":" + deviceProfile.toStdString() + json_ext};
      break;
  }

  if (!std::filesystem::is_regular_file(input_file)) {
    return;
  }

  nlohmann::json json;

  std::ifstream is(input_file);

  is >> json;

  if (presetName.toStdString() == json.value("preset-name", "") &&
      deviceProfile.toStdString() == json.value("device-profile", "")) {
    std::filesystem::remove(input_file);

    util::debug("removed autoload: " + input_file.string());
  }
}

auto Manager::find_autoload(const PipelineType& pipeline_type,
                            const QString& device_name,
                            const QString& device_profile) -> std::string {
  std::filesystem::path input_file;

  switch (pipeline_type) {
    case PipelineType::output:
      input_file = autoload_output_dir /
                   std::filesystem::path{device_name.toStdString() + ":" + device_profile.toStdString() + json_ext};
      break;
    case PipelineType::input:
      input_file = autoload_input_dir /
                   std::filesystem::path{device_name.toStdString() + ":" + device_profile.toStdString() + json_ext};
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

void Manager::autoload(const PipelineType& pipeline_type, const QString& device_name, const QString& device_profile) {
  const auto name = find_autoload(pipeline_type, device_name, device_profile);

  if (name.empty()) {
    switch (pipeline_type) {
      case PipelineType::input: {
        if (db::Main::inputAutoloadingUsesFallback()) {
          util::debug("autoloading fallback preset " + name + " for device " + device_name.toStdString());

          loadLocalPresetFile(pipeline_type, db::Main::inputAutoloadingFallbackPreset());
        }
        break;
      }
      case PipelineType::output: {
        if (db::Main::outputAutoloadingUsesFallback()) {
          util::debug("autoloading fallback preset " + name + " for device " + device_name.toStdString());

          loadLocalPresetFile(pipeline_type, db::Main::outputAutoloadingFallbackPreset());
        }
        break;
      }
    }

    return;
  }

  util::debug("autoloading local preset " + name + " for device " + device_name.toStdString());

  loadLocalPresetFile(pipeline_type, QString::fromStdString(name));
}

auto Manager::get_autoload_profiles(const PipelineType& pipeline_type) -> std::vector<nlohmann::json> {
  std::filesystem::path autoload_dir;
  std::vector<nlohmann::json> list;

  switch (pipeline_type) {
    case PipelineType::output:
      autoload_dir = autoload_output_dir;

      break;
    case PipelineType::input:
      autoload_dir = autoload_input_dir;
      break;
  }

  auto it = std::filesystem::directory_iterator{autoload_dir};

  try {
    while (it != std::filesystem::directory_iterator{}) {
      if (std::filesystem::is_regular_file(it->status())) {
        if (it->path().extension().string() == json_ext) {
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

void Manager::set_last_preset_keys(const PipelineType& pipeline_type,
                                   const QString& preset_name,
                                   const QString& package_name) {
  // In order to avoid race conditions, the community package key should be set before the preset name.

  if (package_name.isEmpty()) {
    switch (pipeline_type) {
      case PipelineType::input:
        db::Main::setLastLoadedInputCommunityPackage(db::Main::defaultLastLoadedInputCommunityPackageValue());
        break;
      case PipelineType::output:
        db::Main::setLastLoadedOutputCommunityPackage(db::Main::defaultLastLoadedOutputCommunityPackageValue());
        break;
    }
  } else {
    switch (pipeline_type) {
      case PipelineType::input:
        db::Main::setLastLoadedInputCommunityPackage(package_name);
        break;
      case PipelineType::output:
        db::Main::setLastLoadedOutputCommunityPackage(package_name);
        break;
    }
  }

  if (preset_name.isEmpty()) {
    switch (pipeline_type) {
      case PipelineType::input:
        db::Main::setLastLoadedInputPreset(db::Main::defaultLastLoadedInputPresetValue());
        break;
      case PipelineType::output:
        db::Main::setLastLoadedOutputPreset(db::Main::defaultLastLoadedOutputPresetValue());
        break;
    }
  } else {
    switch (pipeline_type) {
      case PipelineType::input:
        db::Main::setLastLoadedInputPreset(preset_name);
        break;
      case PipelineType::output:
        db::Main::setLastLoadedOutputPreset(preset_name);
        break;
    }
  }
}

auto Manager::preset_file_exists(const PipelineType& pipeline_type, const std::string& name) -> bool {
  const auto conf_dir = (pipeline_type == PipelineType::output) ? user_output_dir : user_input_dir;

  const auto input_file = conf_dir / std::filesystem::path{name + json_ext};

  return std::filesystem::exists(input_file);
}

void Manager::notify_error(const PresetError& preset_error, const std::string& plugin_name) {
  QString plugin_translated;

  try {
    const auto base_name = tags::plugin_name::Model::self().getBaseName(QString::fromStdString(plugin_name));
    plugin_translated = tags::plugin_name::Model::self().translate(base_name) + ": ";
  } catch (std::out_of_range& e) {
    util::debug(e.what());
  }

  switch (preset_error) {
    case PresetError::blocklist_format: {
      util::warning(
          "A parsing error occurred while trying to load the blocklist from the preset. The file could be invalid "
          "or corrupted. Please check its content.");

      Q_EMIT presetLoadError(i18n("Preset Not Loaded Correctly"), i18n("Wrong Format in Excluded Apps List"));

      break;
    }
    case PresetError::blocklist_generic: {
      util::warning("A generic error occurred while trying to load the blocklist from the preset.");

      Q_EMIT presetLoadError(i18n("Preset Not Loaded Correctly"),
                             i18n("Generic Error While Loading Excluded Apps List"));

      break;
    }
    case PresetError::pipeline_format: {
      util::warning(
          "A parsing error occurred while trying to load the pipeline from the preset. The file could be invalid "
          "or corrupted. Please check its content.");

      Q_EMIT presetLoadError(i18n("Preset Not Loaded Correctly"), i18n("Wrong Format in Effects List"));

      break;
    }
    case PresetError::pipeline_generic: {
      util::warning("A generic error occurred while trying to load the pipeline from the preset.");

      Q_EMIT presetLoadError(i18n("Preset Not Loaded Correctly"), i18n("Generic Error While Loading Effects List"));

      break;
    }
    case PresetError::plugin_format: {
      util::warning("A parsing error occurred while trying to load the " + plugin_name +
                    " plugin from the preset. The file could be invalid or "
                    "corrupted. Please check its content.");

      Q_EMIT presetLoadError(i18n("Preset Not Loaded Correctly"),
                             plugin_translated + i18n("One or More Parameters Have a Wrong Format"));

      break;
    }
    case PresetError::plugin_generic: {
      util::warning("A generic error occurred while trying to load the " + plugin_name + " plugin from the preset.");

      Q_EMIT presetLoadError(i18n("Preset Not Loaded Correctly"),
                             plugin_translated + i18n("Generic Error While Loading The Effect"));

      break;
    }
    default:
      break;
  }
}

auto Manager::create_wrapper(const PipelineType& pipeline_type, const QString& filter_name)
    -> std::optional<std::unique_ptr<PluginPresetBase>> {
  if (filter_name.startsWith(tags::plugin_name::BaseName::autogain)) {
    return std::make_unique<AutoGainPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::bassEnhancer)) {
    return std::make_unique<BassEnhancerPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::bassLoudness)) {
    // return std::make_unique<BassLoudnessPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::compressor)) {
    return std::make_unique<CompressorPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::convolver)) {
    return std::make_unique<ConvolverPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::crossfeed)) {
    return std::make_unique<CrossfeedPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::crystalizer)) {
    return std::make_unique<CrystalizerPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::deesser)) {
    // return std::make_unique<DeesserPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::delay)) {
    // return std::make_unique<DelayPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::deepfilternet)) {
    // return std::make_unique<DeepFilterNetPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::echoCanceller)) {
    // return std::make_unique<EchoCancellerPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::equalizer)) {
    // return std::make_unique<EqualizerPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::exciter)) {
    return std::make_unique<ExciterPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::expander)) {
    // return std::make_unique<ExpanderPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::filter)) {
    return std::make_unique<FilterPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::gate)) {
    return std::make_unique<GatePreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::levelMeter)) {
    // return std::make_unique<LevelMeterPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::limiter)) {
    return std::make_unique<LimiterPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::loudness)) {
    // return std::make_unique<LoudnessPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::maximizer)) {
    return std::make_unique<MaximizerPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::multibandCompressor)) {
    // return std::make_unique<MultibandCompressorPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::multibandGate)) {
    // return std::make_unique<MultibandGatePreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::pitch)) {
    // return std::make_unique<PitchPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::reverb)) {
    // return std::make_unique<ReverbPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::rnnoise)) {
    return std::make_unique<RNNoisePreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::speex)) {
    return std::make_unique<SpeexPreset>(pipeline_type, filter_name.toStdString());
  }

  if (filter_name.startsWith(tags::plugin_name::BaseName::stereoTools)) {
    return std::make_unique<StereoToolsPreset>(pipeline_type, filter_name.toStdString());
  }

  util::warning("The filter name " + filter_name.toStdString() + " base name could not be recognized");

  return std::nullopt;
}

}  // namespace presets
