/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "rnnoise.hpp"
#include <qstandardpaths.h>
#include <qtmetamacros.h>
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <format>
#include "easyeffects_db_rnnoise.h"
#include "pipeline_type.hpp"
#include "tags_app.hpp"
#ifdef ENABLE_RNNOISE
#include <rnnoise.h>
#endif
#include <sys/types.h>
#include <cmath>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "resampler.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

RNNoise::RNNoise(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::rnnoise,
                 tags::plugin_package::Package::rnnoise,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(
          db::Manager::self().get_plugin_db<db::RNNoise>(pipe_type,
                                                         tags::plugin_name::BaseName::rnnoise + "#" + instance_id)),
      app_data_dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString()),
      data_L(0),
      data_R(0) {
  data_L.reserve(blocksize);
  data_R.reserve(blocksize);
  data_tmp.reserve(blocksize);

  init_common_controls<db::RNNoise>(settings);

  // Initialize directories for local and community models
  local_dir_rnnoise = app_data_dir + "/rnnoise";

  // Flatpak specific path (.flatpak-info always present for apps
  // running in the flatpak sandbox)
  if (std::filesystem::is_regular_file(tags::app::flatpak_info_file)) {
    system_data_dir_rnnoise.emplace_back("/app/extensions/Presets/rnnoise");
  }

  // Regular paths.
  for (auto& dir : QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)) {
    dir += dir.endsWith("/") ? "" : "/";

    system_data_dir_rnnoise.push_back(dir.toStdString() + "rnnoise");
  }

#ifdef ENABLE_RNNOISE

  init_release();

  wet_ratio =
      (settings->wet() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->wet()));

  connect(settings, &db::RNNoise::useStandardModelChanged, [&]() { prepare_model(); });

  connect(settings, &db::RNNoise::modelNameChanged, [&]() { prepare_model(); });

  connect(settings, &db::RNNoise::wetChanged, [&]() {
    wet_ratio =
        (settings->wet() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->wet()));
  });

  connect(settings, &db::RNNoise::releaseChanged, [&]() { init_release(); });

  auto* m = get_model_from_name();

  model = m;

  state_left = rnnoise_create(model);
  state_right = rnnoise_create(model);

  vad_prob_left = 1.0F;
  vad_prob_right = 1.0F;
  vad_grace_left = release;
  vad_grace_right = release;

  rnnoise_ready = true;
#else
  util::warning("The RNNoise library was not available at compilation time. The noise reduction filter won't work");

  settings->setEnableVad(false);

  packageInstalled = false;

  Q_EMIT packageInstalledChanged();
#endif
}

RNNoise::~RNNoise() {
  stop_worker();

  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  std::scoped_lock<std::mutex> lock(data_mutex);

  resampler_ready = false;

#ifdef ENABLE_RNNOISE
  free_rnnoise();
#endif

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void RNNoise::reset() {
  settings->setDefaults();
}

void RNNoise::clear_data() {
  setup();
}

void RNNoise::setup() {
  if (rate == 0 || n_samples == 0) {
    // Some signals may be emitted before PipeWire calls our setup function
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  resampler_ready = false;

  latency_n_frames = 0U;

  resample = rate != rnnoise_rate;

  data_L.clear();
  data_R.clear();

  buf_out_L.clear();
  buf_out_R.clear();

  resampler_inL = std::make_unique<Resampler>(rate, rnnoise_rate);
  resampler_inR = std::make_unique<Resampler>(rate, rnnoise_rate);

  resampler_outL = std::make_unique<Resampler>(rnnoise_rate, rate);
  resampler_outR = std::make_unique<Resampler>(rnnoise_rate, rate);

  resampler_ready = true;
}

void RNNoise::process(std::span<float>& left_in,
                      std::span<float>& right_in,
                      std::span<float>& left_out,
                      std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    return;
  }

#ifdef ENABLE_RNNOISE
  if (!rnnoise_ready) {
    std::ranges::fill(left_out, 0.0F);
    std::ranges::fill(right_out, 0.0F);

    return;
  }
#else
  std::ranges::copy(left_in, left_out.begin());
  std::ranges::copy(right_in, right_out.begin());

  return;
#endif

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  if (resample) {
    if (resampler_ready) {
      const auto resampled_inL = resampler_inL->process(left_in);
      const auto resampled_inR = resampler_inR->process(right_in);

      resampled_data_L.resize(0U);
      resampled_data_R.resize(0U);

#ifdef ENABLE_RNNOISE
      remove_noise(resampled_inL, resampled_inR, resampled_data_L, resampled_data_R);
#endif

      auto resampled_outL = resampler_outL->process(resampled_data_L);
      auto resampled_outR = resampler_outR->process(resampled_data_R);

      buf_out_L.insert(buf_out_L.end(), resampled_outL.begin(), resampled_outL.end());
      buf_out_R.insert(buf_out_R.end(), resampled_outR.begin(), resampled_outR.end());
    } else {
      buf_out_L.insert(buf_out_L.end(), left_in.begin(), left_in.end());
      buf_out_R.insert(buf_out_R.end(), right_in.begin(), right_in.end());
    }
  } else {
#ifdef ENABLE_RNNOISE
    remove_noise(left_in, right_in, buf_out_L, buf_out_R);
#endif
  }

  if (buf_out_L.size() >= n_samples) {
    util::copy_bulk(buf_out_L, left_out);
    util::copy_bulk(buf_out_R, right_out);
  } else {
    const uint offset = left_out.size() - buf_out_L.size();

    if (offset != latency_n_frames) {
      latency_n_frames = offset;

      notify_latency = true;
    }

    // Fill beginning with zeros
    std::fill_n(left_out.begin(), offset, 0.0F);
    std::fill_n(right_out.begin(), offset, 0.0F);

    std::ranges::copy(buf_out_L, left_out.begin() + offset);
    std::ranges::copy(buf_out_R, right_out.begin() + offset);

    buf_out_L.clear();
    buf_out_R.clear();
  }

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (notify_latency) {
    latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(std::format("{} latency: {} s", log_tag + name.toStdString(), latency_value));

    update_filter_params();

    notify_latency = false;
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void RNNoise::process([[maybe_unused]] std::span<float>& left_in,
                      [[maybe_unused]] std::span<float>& right_in,
                      [[maybe_unused]] std::span<float>& left_out,
                      [[maybe_unused]] std::span<float>& right_out,
                      [[maybe_unused]] std::span<float>& probe_left,
                      [[maybe_unused]] std::span<float>& probe_right) {}

auto RNNoise::search_model_path(const std::string& name) -> std::string {
  // Given the model name without extension, search the full path on the filesystem.
  const auto model_filename = name + rnnn_ext;

  const auto community_package = (pipeline_type == PipelineType::input) ? DbMain::lastLoadedInputCommunityPackage()
                                                                        : DbMain::lastLoadedOutputCommunityPackage();

  std::string model_full_path;

  if (community_package.isEmpty()) {
    // Search local model
    const auto local_model_file = std::filesystem::path{local_dir_rnnoise + "/" + model_filename};

    if (std::filesystem::exists(local_model_file)) {
      model_full_path = local_model_file.string();
    }
  } else {
    // Search model in community package paths
    for (const auto& xdg_model_dir : system_data_dir_rnnoise) {
      if (util::search_filename(std::filesystem::path{xdg_model_dir + "/" + community_package.toStdString()},
                                model_filename, model_full_path, 3U)) {
        break;
      }
    }
  }

  return model_full_path;
}

#ifdef ENABLE_RNNOISE

auto RNNoise::get_model_from_name() -> RNNModel* {
  // Standard Model
  if (settings->useStandardModel()) {
    standard_model = true;

    util::debug(std::format("{}using the standard model", log_tag));

    Q_EMIT standardModelLoaded();

    return nullptr;
  }

  const auto name = settings->modelName().toStdString();

  const auto path = search_model_path(name);

  // Fallback to Standard Model on empty path.
  if (path.empty()) {
    standard_model = true;

    util::debug(std::format("{}{} model does not exist on the filesystem, using the standard model.", log_tag, name));

    /**
     * If the name is the default settings value, it's likely the user has
     * unchecked the standard model switch in the UI after a plugin reset,
     * so there's no need to emit the signal in that case.
     */
    if (name != settings->defaultModelNameValue()) {
      Q_EMIT customModelLoaded(settings->modelName(), false);
    }

    return nullptr;
  }

  // Try to load a Custom Model (fallback to Standard Model on error).
  util::debug(std::format("{}loading custom model {} from path: {}", log_tag, name, path));

  RNNModel* m = nullptr;

  if (model_file != nullptr) {
    fclose(model_file);
  }

  /**
   * We prefer using "rnnoise_model_from_file" because it's more robust than
   * "rnnoise_model_from_filename". Indeed, when an invalid model is loaded, it
   * does not crash and automatically switches to the Standard Model.
   *
   * Note that from RNNoise v0.1.1 a new binary model format is used, so the old
   * format may not work even if the file is correctly loaded (the signal does
   * not change like in a passthrough mode). See issue #4748.
   */
  if (model_file = fopen(path.c_str(), "rb"); model_file != nullptr) {
    m = rnnoise_model_from_file(model_file);
  }

  standard_model = (m == nullptr);

  if (standard_model) {
    util::warning(std::format("{}failed to load the custom model {}. Using the standard one.", log_tag, name));
  }

  Q_EMIT customModelLoaded(settings->modelName(), !standard_model);

  return m;
}

void RNNoise::prepare_model() {
  if (settings->modelName().isEmpty()) {
    settings->setUseStandardModel(true);
  }

  data_mutex.lock();

  rnnoise_ready = false;

  data_mutex.unlock();

  free_rnnoise();

  auto* m = get_model_from_name();

  model = m;

  state_left = rnnoise_create(model);
  state_right = rnnoise_create(model);

  rnnoise_ready = true;
}

void RNNoise::free_rnnoise() {
  rnnoise_ready = false;

  if (state_left != nullptr) {
    rnnoise_destroy(state_left);
  }

  if (state_right != nullptr) {
    rnnoise_destroy(state_right);
  }

  if (model != nullptr) {
    rnnoise_model_free(model);
  }

  if (model_file != nullptr) {
    fclose(model_file);
  }

  state_left = nullptr;
  state_right = nullptr;
  model = nullptr;
  model_file = nullptr;
}

#endif

auto RNNoise::get_latency_seconds() -> float {
  return latency_value;
}

void RNNoise::init_release() {
#ifdef ENABLE_RNNOISE

  const auto rate = static_cast<double>(rnnoise_rate);

  const auto bs = static_cast<double>(blocksize);

  // std::lrint returns a long type
  const auto release = static_cast<int>(std::lrint(rate * settings->release() / 1000.0 / bs));

  vad_grace_left = release;
  vad_grace_right = release;

#endif
}
