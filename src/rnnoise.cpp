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

#include "rnnoise.hpp"
#include <qstandardpaths.h>
#include <algorithm>
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
#include <cstdio>
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
      app_config_dir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation).toStdString()),
      data_L(0),
      data_R(0) {
  data_L.reserve(blocksize);
  data_R.reserve(blocksize);
  data_tmp.reserve(blocksize);

  init_common_controls<db::RNNoise>(settings);

  // Initialize directories for local and community models
  local_dir_rnnoise = app_config_dir + "/rnnoise";

  // Flatpak specific path (.flatpak-info always present for apps running in the flatpak sandbox)
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
#endif
}

RNNoise::~RNNoise() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  std::scoped_lock<std::mutex> lock(data_mutex);

  resampler_ready = false;

#ifdef ENABLE_RNNOISE
  free_rnnoise();
#endif

  util::debug(log_tag + name.toStdString() + " destroyed");
}

void RNNoise::reset() {
  settings->setDefaults();
}

void RNNoise::setup() {
  std::scoped_lock<std::mutex> lock(data_mutex);

  resampler_ready = false;

  latency_n_frames = 0U;

  resample = rate != rnnoise_rate;

  data_L.resize(0U);
  data_R.resize(0U);

  deque_out_L.resize(0U);
  deque_out_R.resize(0U);

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

  if (bypass || !rnnoise_ready) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  if (resample) {
    if (resampler_ready) {
      const auto resampled_inL = resampler_inL->process(left_in, false);
      const auto resampled_inR = resampler_inR->process(right_in, false);

      resampled_data_L.resize(0U);
      resampled_data_R.resize(0U);

#ifdef ENABLE_RNNOISE
      remove_noise(resampled_inL, resampled_inR, resampled_data_L, resampled_data_R);
#endif

      auto resampled_outL = resampler_outL->process(resampled_data_L, false);
      auto resampled_outR = resampler_outR->process(resampled_data_R, false);

      for (const auto& v : resampled_outL) {
        deque_out_L.push_back(v);
      }

      for (const auto& v : resampled_outR) {
        deque_out_R.push_back(v);
      }
    } else {
      for (const auto& v : left_in) {
        deque_out_L.push_back(v);
      }

      for (const auto& v : right_in) {
        deque_out_R.push_back(v);
      }
    }
  } else {
#ifdef ENABLE_RNNOISE
    remove_noise(left_in, right_in, deque_out_L, deque_out_R);
#endif
  }

  if (deque_out_L.size() >= left_out.size()) {
    for (float& v : left_out) {
      v = deque_out_L.front();

      deque_out_L.pop_front();
    }

    for (float& v : right_out) {
      v = deque_out_R.front();

      deque_out_R.pop_front();
    }
  } else {
    const uint offset = 2U * (left_out.size() - deque_out_L.size());

    if (offset != latency_n_frames) {
      latency_n_frames = offset;

      notify_latency = true;
    }

    for (uint n = 0U; !deque_out_L.empty() && n < left_out.size(); n++) {
      if (n < offset) {
        left_out[n] = 0.0F;
        right_out[n] = 0.0F;
      } else {
        left_out[n] = deque_out_L.front();
        right_out[n] = deque_out_R.front();

        deque_out_R.pop_front();
        deque_out_L.pop_front();
      }
    }
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

  get_peaks(left_in, right_in, left_out, right_out);
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

  const auto community_package = (pipeline_type == PipelineType::input) ? db::Main::lastLoadedInputCommunityPackage()
                                                                        : db::Main::lastLoadedOutputCommunityPackage();

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
  RNNModel* m = nullptr;

  const auto name = settings->modelName().toStdString();

  // Standard Model
  if (settings->useStandardModel()) {
    standard_model = true;

    util::warning(log_tag + " using the standard model");

    return m;
  }

  const auto path = search_model_path(name);

  // Standard Model
  if (path.empty()) {
    standard_model = true;

    util::debug(log_tag + name + " model does not exist on the filesystem, using the standard model.");

    return m;
  }

  // Custom Model
  util::debug(log_tag + name + " loading custom model from path: " + path);

  if (FILE* f = fopen(path.c_str(), "r"); f != nullptr) {
    m = rnnoise_model_from_file(f);

    fclose(f);
  }

  standard_model = (m == nullptr);

  if (standard_model) {
    util::warning(log_tag + name + " failed to load the custom model. Using the standard one.");
  }

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

  state_left = nullptr;
  state_right = nullptr;
  model = nullptr;
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
