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
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#ifdef ENABLE_RNNOISE
#include <rnnoise.h>
#endif
#include <sys/types.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include "pipe_manager.hpp"
#include "plugin_base.hpp"
#include "resampler.hpp"
#include "tags_plugin_name.hpp"
#include "tags_resources.hpp"
#include "util.hpp"

RNNoise::RNNoise(const std::string& tag,
                 const std::string& schema,
                 const std::string& schema_path,
                 PipeManager* pipe_manager,
                 PipelineType pipe_type)
    : PluginBase(tag,
                 tags::plugin_name::rnnoise,
                 tags::plugin_package::rnnoise,
                 schema,
                 schema_path,
                 pipe_manager,
                 pipe_type),
      enable_vad(g_settings_get_boolean(settings, "enable-vad")),
      vad_thres(g_settings_get_double(settings, "vad-thres") / 100.0F),
      data_L(0),
      data_R(0) {
  data_L.reserve(blocksize);
  data_R.reserve(blocksize);
  data_tmp.reserve(blocksize);

  // Initialize directories for local and community models
  local_dir_rnnoise = std::string{g_get_user_config_dir()} + "/easyeffects/rnnoise";

  // Flatpak specific path (.flatpak-info always present for apps running in the flatpak sandbox)
  if (std::filesystem::is_regular_file(tags::resources::flatpak_info_file)) {
    system_data_dir_rnnoise.push_back("/app/extensions/Presets/rnnoise");
  }

  // Regular paths
  for (const gchar* const* xdg_data_dirs = g_get_system_data_dirs(); *xdg_data_dirs != nullptr;) {
    std::string dir = *xdg_data_dirs++;

    dir += dir.ends_with("/") ? "" : "/";

    system_data_dir_rnnoise.push_back(dir + "easyeffects/rnnoise");
  }

  const auto key_v = g_settings_get_double(settings, "wet");

  wet_ratio = (key_v <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(key_v));

  gconnections.push_back(g_signal_connect(settings, "changed::model-name",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto* self = static_cast<RNNoise*>(user_data);

                                            self->data_mutex.lock();

                                            self->rnnoise_ready = false;

                                            self->data_mutex.unlock();

#ifdef ENABLE_RNNOISE
                                            self->free_rnnoise();

                                            auto* m = self->get_model_from_name();

                                            self->model = m;

                                            self->state_left = rnnoise_create(self->model);
                                            self->state_right = rnnoise_create(self->model);

                                            self->rnnoise_ready = true;
#endif
                                          }),
                                          this));

  setup_input_output_gain();

#ifdef ENABLE_RNNOISE

  init_release();

  gconnections.push_back(g_signal_connect(settings, "changed::enable-vad",
                                          G_CALLBACK(+[](GSettings* settings, char* key, RNNoise* self) {
                                            self->enable_vad = g_settings_get_boolean(settings, key);
                                          }),
                                          this));

  g_signal_connect(settings, "changed::vad-thres", G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                     auto self = static_cast<RNNoise*>(user_data);

                     self->vad_thres = static_cast<float>(g_settings_get_double(settings, key)) / 100.0F;
                   }),
                   this);

  g_signal_connect(settings, "changed::wet", G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                     auto self = static_cast<RNNoise*>(user_data);

                     const auto key_v = g_settings_get_double(settings, key);

                     self->wet_ratio =
                         (key_v <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(key_v));
                   }),
                   this);

  g_signal_connect(settings, "changed::release", G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                     auto self = static_cast<RNNoise*>(user_data);

                     self->init_release();
                   }),
                   this);

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

  enable_vad = false;
#endif
}

RNNoise::~RNNoise() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  resampler_ready = false;

#ifdef ENABLE_RNNOISE
  free_rnnoise();
#endif

  util::debug(log_tag + name + " destroyed");
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
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

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

    util::debug(log_tag + name + " latency: " + util::to_string(latency_value, "") + " s");

    util::idle_add([this]() {
      if (!post_messages || latency.empty()) {
        return;
      }

      latency.emit();
    });

    update_filter_params();

    notify_latency = false;
  }

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    if (send_notifications) {
      notify();
    }
  }
}

auto RNNoise::search_model_path(const std::string& name) -> std::string {
  // Given the model name without extension, search the full path on the filesystem.
  const auto model_filename = name + rnnn_ext;

  const auto* lcp_key = (pipeline_type == PipelineType::input) ? "last-loaded-input-community-package"
                                                               : "last-loaded-output-community-package";

  const auto community_package = util::gsettings_get_string(global_settings, lcp_key);

  std::string model_full_path;

  if (community_package.empty()) {
    // Search local model
    const auto local_model_file = std::filesystem::path{local_dir_rnnoise + "/" + model_filename};

    if (std::filesystem::exists(local_model_file)) {
      model_full_path = local_model_file.c_str();
    }
  } else {
    // Search model in community package paths
    for (const auto& xdg_model_dir : system_data_dir_rnnoise) {
      if (util::search_filename(std::filesystem::path{xdg_model_dir + "/" + community_package}, model_filename,
                                model_full_path, 3U)) {
        break;
      }
    }
  }

  return model_full_path;
}

#ifdef ENABLE_RNNOISE

auto RNNoise::get_model_from_name() -> RNNModel* {
  RNNModel* m = nullptr;

  const auto name = util::gsettings_get_string(settings, "model-name");

  // Standard Model
  if (name.empty()) {
    standard_model = true;

    util::warning(log_tag + " empty model name set, using the standard model");

    model_changed.emit(false);

    return m;
  }

  const auto path = search_model_path(name);

  // Standard Model
  if (path.empty()) {
    standard_model = true;

    util::debug(log_tag + name + " model does not exist on the filesystem, using the standard model.");

    model_changed.emit(false);

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

  model_changed.emit(standard_model);

  return m;
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

  const auto key_v = g_settings_get_double(settings, "release");

  const auto rate = static_cast<double>(rnnoise_rate);

  const auto bs = static_cast<double>(blocksize);

  // std::lrint returns a long type
  const auto release = static_cast<int>(std::lrint(rate * key_v / 1000.0 / bs));

  vad_grace_left = release;
  vad_grace_right = release;

#endif
}
