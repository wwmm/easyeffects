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

#include "convolver_kernel_manager.hpp"
#include <mysofa.h>
#include <qstandardpaths.h>
#include <qtypes.h>
#include <sndfile.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <exception>
#include <execution>
#include <filesystem>
#include <format>
#include <memory>
#include <numeric>
#include <sndfile.hh>
#include <string>
#include <utility>
#include <vector>
#include "db_manager.hpp"
#include "easyeffects_db_convolver.h"
#include "pipeline_type.hpp"
#include "resampler.hpp"
#include "tags_app.hpp"
#include "util.hpp"

ConvolverKernelManager::ConvolverKernelManager(db::Convolver* settings, const PipelineType& pipeline_type)
    : settings(settings),
      pipeline_type(pipeline_type),
      app_data_dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString()),
      local_dir_irs(app_data_dir + "/irs") {
  /**
   * Flatpak specific path (.flatpak-info always present for apps running in
   * the flatpak sandbox)
   */
  if (std::filesystem::is_regular_file(tags::app::flatpak_info_file)) {
    system_data_dir_irs.emplace_back("/app/extensions/Presets/irs");
  }

  // Regular paths.
  for (auto& dir : QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)) {
    dir += dir.endsWith("/") ? "" : "/";

    system_data_dir_irs.push_back(dir.toStdString() + "irs");
  }
}

auto ConvolverKernelManager::KernelData::isValid() const -> bool {
  return rate > 0 && !channel_L.empty() && !channel_R.empty() && channel_L.size() == channel_R.size();
}

auto ConvolverKernelManager::KernelData::duration() const -> double {
  if (rate == 0 || channel_L.empty()) {
    return 0.0;
  }

  return static_cast<double>(channel_L.size() - 1) / static_cast<double>(rate);
}

auto ConvolverKernelManager::KernelData::sampleCount() const -> size_t {
  return channel_L.size();
}

auto ConvolverKernelManager::loadKernel(const std::string& name) -> KernelData {
  if (name.empty()) {
    util::warning("Kernel name is empty");

    return KernelData{};
  }

  const auto file_path = searchKernelPath(name);

  if (file_path.empty()) {
    util::warning(std::format("Kernel '{}' not found", name));
    return KernelData{};
  }

  KernelData kernel_data;

  const auto extension = getFileExtension(file_path);

  if (extension == sofa_ext) {
    kernel_data = readSofaKernelFile(file_path);
  } else {
    kernel_data = readKernelFile(file_path);
  }

  kernel_data.name = QString::fromStdString(name);
  kernel_data.file_path = QString::fromStdString(file_path);

  if (!validateKernel(kernel_data)) {
    util::warning(std::format("Kernel '{}' is invalid", name));
    return KernelData{};
  }

  util::debug(std::format("Loaded kernel '{}': {} Hz, {} samples, {:.3f}s", name, kernel_data.rate,
                          kernel_data.sampleCount(), kernel_data.duration()));

  return kernel_data;
}

auto ConvolverKernelManager::combineKernels(const std::string& kernel1_name,
                                            const std::string& kernel2_name,
                                            const std::string& output_name) -> bool {
  if (output_name.empty()) {
    util::warning("Output kernel name is empty");
    return false;
  }

  const auto kernel1 = loadKernel(kernel1_name);
  const auto kernel2 = loadKernel(kernel2_name);

  if (!kernel1.isValid() || !kernel2.isValid()) {
    util::warning("One or both kernels are invalid for combination");
    return false;
  }

  if (kernel1.channels != kernel2.channels) {
    util::warning("The kernels must have the same number of channel to be combined!");
    return false;
  }

  const auto target_rate = std::max(kernel1.rate, kernel2.rate);
  const auto resampled_kernel1 = (kernel1.rate != target_rate) ? resampleKernel(kernel1, target_rate) : kernel1;
  const auto resampled_kernel2 = (kernel2.rate != target_rate) ? resampleKernel(kernel2, target_rate) : kernel2;

  auto combined_kernel_L = directConvolution(resampled_kernel1.channel_L, resampled_kernel2.channel_L);
  auto combined_kernel_R = directConvolution(resampled_kernel1.channel_R, resampled_kernel2.channel_R);

  KernelData combined_kernel;

  combined_kernel.rate = target_rate;
  combined_kernel.channel_L = std::move(combined_kernel_L);
  combined_kernel.channel_R = std::move(combined_kernel_R);
  combined_kernel.name = QString::fromStdString(output_name);

  // Save the combined kernel
  const auto success = saveKernel(combined_kernel, output_name);

  if (success) {
    util::debug(
        std::format("Successfully combined kernels '{}' and '{}' into '{}'", kernel1_name, kernel2_name, output_name));
  }

  return success;
}

auto ConvolverKernelManager::searchKernelPath(const std::string& name) -> std::string {
  // Given the irs name without extension, search the full path on the filesystem.

  const std::vector<std::string> extensions = {irs_ext, sofa_ext};

  const auto community_package = (pipeline_type == PipelineType::input) ? DbMain::lastLoadedInputCommunityPackage()
                                                                        : DbMain::lastLoadedOutputCommunityPackage();

  std::string kernel_full_path;

  if (community_package.isEmpty()) {
    for (const auto& ext : extensions) {
      std::string path = local_dir_irs;

      path.append("/");
      path.append(name);
      path.append(ext);

      auto local_kernel_file = std::filesystem::path{path};

      util::warning(local_kernel_file);

      if (std::filesystem::exists(local_kernel_file)) {
        kernel_full_path = local_kernel_file.string();

        break;
      }
    }
  } else {
    // Search kernel file in community package paths.

    for (const auto& ext : extensions) {
      auto kernel_filename = name;

      kernel_filename.append(ext);

      for (const auto& xdg_irs_dir : system_data_dir_irs) {
        if (util::search_filename(std::filesystem::path{xdg_irs_dir + "/" + community_package.toStdString()},
                                  kernel_filename, kernel_full_path, 3U)) {
          break;
        }
      }

      if (!kernel_full_path.empty()) {
        break;
      }
    }
  }

  return kernel_full_path;
}

auto ConvolverKernelManager::resampleKernel(const KernelData& kernel, const uint& target_rate) -> KernelData {
  if (target_rate == 0) {
    util::debug(std::format("Target rate value is zero. Aborting resampling of kernel {}", kernel.name.toStdString()));

    return kernel;
  }

  if (kernel.rate == target_rate || !kernel.isValid()) {
    util::debug(std::format("The kernel '{}' does not need resampling", kernel.name.toStdString()));

    return kernel;
  }

  KernelData resampled_kernel = kernel;

  resampled_kernel.rate = target_rate;

  util::debug(
      std::format("Resampling kernel '{}' from {} Hz to {} Hz", kernel.name.toStdString(), kernel.rate, target_rate));

  // Resample left channel

  auto resampler = std::make_unique<Resampler>(kernel.rate, target_rate);

  resampled_kernel.channel_L = resampler->process(kernel.channel_L);

  // Resample right channel

  resampler = std::make_unique<Resampler>(kernel.rate, target_rate);

  resampled_kernel.channel_R = resampler->process(kernel.channel_R);

  if (kernel.channels == 4) {
    // Resample LR channel

    resampler = std::make_unique<Resampler>(kernel.rate, target_rate);

    resampled_kernel.channel_LR = resampler->process(kernel.channel_LR);

    // Resample right channel

    resampler = std::make_unique<Resampler>(kernel.rate, target_rate);

    resampled_kernel.channel_RL = resampler->process(kernel.channel_RL);
  }

  return resampled_kernel;
}

void ConvolverKernelManager::normalizeKernel(KernelData& kernel) {
  if (!kernel.isValid()) {
    return;
  }

  // Find the maximum absolute value across both channels
  const auto max_abs_L =
      std::ranges::max(kernel.channel_L, [](const auto& a, const auto& b) { return std::fabs(a) < std::fabs(b); });

  const auto max_abs_R =
      std::ranges::max(kernel.channel_R, [](const auto& a, const auto& b) { return std::fabs(a) < std::fabs(b); });

  auto peak = std::max(std::fabs(max_abs_L), std::fabs(max_abs_R));

  if (kernel.channels == 4) {
    const auto max_abs_LR =
        std::ranges::max(kernel.channel_LR, [](const auto& a, const auto& b) { return std::fabs(a) < std::fabs(b); });

    const auto max_abs_RL =
        std::ranges::max(kernel.channel_RL, [](const auto& a, const auto& b) { return std::fabs(a) < std::fabs(b); });

    peak = std::max(peak, std::fabs(max_abs_LR));
    peak = std::max(peak, std::fabs(max_abs_RL));
  }

  if (peak <= 1e-6) {
    return;
  }

  // Normalize both channels
  const auto normalize_lambda = [peak](auto& sample) { sample /= peak; };

  std::ranges::for_each(kernel.channel_L, normalize_lambda);
  std::ranges::for_each(kernel.channel_R, normalize_lambda);

  if (kernel.channels == 4) {
    std::ranges::for_each(kernel.channel_LR, normalize_lambda);
    std::ranges::for_each(kernel.channel_RL, normalize_lambda);
  }
}

auto ConvolverKernelManager::saveKernel(const KernelData& kernel, const std::string& file_name) -> bool {
  if (!kernel.isValid() || file_name.empty()) {
    return false;
  }

  const auto file_path = std::filesystem::path{local_dir_irs} / (file_name + irs_ext);

  try {
    // Ensure directory exists
    std::filesystem::create_directories(file_path.parent_path());

    // Prepare interleaved buffer
    std::vector<float> buffer(kernel.sampleCount() * kernel.channels);

    for (size_t i = 0; i < kernel.sampleCount(); ++i) {
      if (kernel.channels == 2) {
        buffer[2 * i] = kernel.channel_L[i];
        buffer[(2 * i) + 1] = kernel.channel_R[i];
      } else if (kernel.channels == 4) {
        buffer[4 * i] = kernel.channel_L[i];
        buffer[(4 * i) + 1] = kernel.channel_LR[i];
        buffer[(4 * i) + 2] = kernel.channel_RL[i];
        buffer[(4 * i) + 3] = kernel.channel_R[i];
      }
    }

    // Write to file
    const auto mode = SFM_WRITE;
    const auto format = SF_FORMAT_WAV | SF_FORMAT_PCM_32;
    const auto channels = kernel.channels;

    SndfileHandle sndfile(file_path.string(), mode, format, channels, kernel.rate);

    if (sndfile.error() != 0) {
      util::warning(std::format("Failed to create kernel file: {}", sndfile.strError()));
      return false;
    }

    const auto frames_written = sndfile.writef(buffer.data(), static_cast<sf_count_t>(kernel.sampleCount()));

    if (frames_written != static_cast<sf_count_t>(kernel.sampleCount())) {
      util::warning(std::format("Failed to write all kernel data: {} of {} frames written", frames_written,
                                kernel.sampleCount()));
      return false;
    }

    util::debug(std::format("Saved kernel '{}' to {}", file_name, file_path.string()));

    return true;

  } catch (const std::exception& e) {
    util::warning(std::format("Exception while saving kernel: {}", e.what()));
    return false;
  }
}

auto ConvolverKernelManager::readKernelFile(const std::string& file_path) -> KernelData {
  KernelData kernel_data;

  try {
    SndfileHandle sndfile(file_path);

    if (sndfile.error() != 0) {
      util::warning(std::format("Failed to open kernel file: {} - {}", file_path, sndfile.strError()));
      return kernel_data;
    }

    if (sndfile.channels() != 1 && sndfile.channels() != 2 && sndfile.channels() != 4 && sndfile.frames() == 0) {
      util::warning(std::format("Only mono, stereo and true stereo impulse files are supported: {}", file_path));
      return kernel_data;
    }

    // Read interleaved data
    std::vector<float> buffer(sndfile.frames() * sndfile.channels());

    const auto frames_read = sndfile.readf(buffer.data(), sndfile.frames());

    if (frames_read != sndfile.frames()) {
      util::warning(
          std::format("Failed to read complete kernel file: {} of {} frames read", frames_read, sndfile.frames()));
      return kernel_data;
    }

    // Deinterleave channels
    kernel_data.rate = sndfile.samplerate();
    kernel_data.original_rate = sndfile.samplerate();
    kernel_data.channels = sndfile.channels() == 1 ? 2 : sndfile.channels();
    kernel_data.channel_L.resize(sndfile.frames());
    kernel_data.channel_R.resize(sndfile.frames());

    if (kernel_data.channels == 4) {
      kernel_data.channel_LR.resize(sndfile.frames());
      kernel_data.channel_RL.resize(sndfile.frames());
    }

    for (size_t i = 0; i < static_cast<size_t>(sndfile.frames()); ++i) {
      if (sndfile.channels() == 1) {
        kernel_data.channel_L[i] = buffer[i];
        kernel_data.channel_R[i] = buffer[i];
      } else if (sndfile.channels() == 2) {
        kernel_data.channel_L[i] = buffer[2 * i];
        kernel_data.channel_R[i] = buffer[(2 * i) + 1];
      } else if (sndfile.channels() == 4) {
        kernel_data.channel_L[i] = buffer[4 * i];
        kernel_data.channel_LR[i] = buffer[(4 * i) + 1];
        kernel_data.channel_RL[i] = buffer[(4 * i) + 2];
        kernel_data.channel_R[i] = buffer[(4 * i) + 3];
      }
    }

  } catch (const std::exception& e) {
    util::warning(std::format("Exception while reading kernel file {}: {}", file_path, e.what()));
  }

  return kernel_data;
}

auto ConvolverKernelManager::validateKernel(const KernelData& kernel) -> bool {
  if (kernel.rate <= 0) {
    util::warning("Kernel has invalid sample rate");
    return false;
  }

  if (kernel.channel_L.empty() || kernel.channel_R.empty()) {
    util::warning("Kernel channels are empty");
    return false;
  }

  if ((kernel.channels == 4) & (kernel.channel_LR.empty() || kernel.channel_RL.empty())) {
    util::warning("Kernel channels are empty");
    return false;
  }

  if (kernel.channel_L.size() != kernel.channel_R.size()) {
    util::warning("Kernel channels have different sizes");
    return false;
  }

  if ((kernel.channels == 4) & (kernel.channel_L.size() != kernel.channel_R.size())) {
    util::warning("Kernel LR and RL channels have different sizes");
    return false;
  }

  const auto has_invalid_values = [](const std::vector<float>& channel) {
    return std::ranges::any_of(channel, [](const auto& value) { return std::isnan(value) || std::isinf(value); });
  };

  if (has_invalid_values(kernel.channel_L) || has_invalid_values(kernel.channel_R)) {
    util::warning("Kernel contains NaN or infinite values");
    return false;
  }

  if ((kernel.channels == 4) & has_invalid_values(kernel.channel_LR) || has_invalid_values(kernel.channel_RL)) {
    util::warning("Kernel contains NaN or infinite values");
    return false;
  }

  return true;
}

auto ConvolverKernelManager::findKernelInDirectory(const std::filesystem::path& directory,
                                                   const std::string& kernel_name) -> std::string {
  if (!std::filesystem::exists(directory)) {
    return "";
  }

  const auto kernel_filename = kernel_name + irs_ext;

  try {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
      if (entry.is_regular_file() && entry.path().filename() == kernel_filename) {
        return entry.path().string();
      }
    }
  } catch (const std::exception& e) {
    util::warning(std::format("Error searching for kernel in {}: {}", directory.string(), e.what()));
  }

  return "";
}

auto ConvolverKernelManager::directConvolution(const std::vector<float>& a, const std::vector<float>& b)
    -> std::vector<float> {
  if (a.empty() || b.empty()) {
    return {};
  }

  const auto output_size = a.size() + b.size() - 1;

  std::vector<float> result(output_size, 0.0F);

  std::vector<size_t> indices(output_size);

  std::iota(indices.begin(), indices.end(), 0U);

  auto each = [&](const int n) {
    result[n] = 0.0F;

    const int a_size = static_cast<int>(a.size());
    const int b_size = static_cast<int>(b.size());

    for (int m = 0; m < b_size; m++) {
      if (const auto z = n - m; z >= 0 && z < a_size - 1) {
        result[n] += b[m] * a[z];
      }
    }
  };

#if defined(ENABLE_LIBCPP_WORKAROUNDS) || defined(_LIBCPP_HAS_NO_INCOMPLETE_PSTL) || \
    (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION < 170000)
  std::for_each(indices.begin(), indices.end(), each);
#else
  std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), each);
#endif

  return result;
}

auto ConvolverKernelManager::getFileExtension(const std::string& file_path) -> std::string {
  std::filesystem::path path(file_path);
  std::string ext = path.extension().string();

  // Convert to lowercase for case-insensitive comparison
  std::ranges::transform(ext, ext.begin(), [](unsigned char c) { return std::tolower(c); });

  return ext;
}

// https://www.sofaconventions.org/mediawiki/index.php/SOFA_specifications

auto ConvolverKernelManager::readSofaKernelFile(const std::string& file_path) -> KernelData {
  KernelData kernel_data;

  try {
    int error = 0;

    struct MYSOFA_HRTF* hrtf = mysofa_load(file_path.c_str(), &error);

    if (error != MYSOFA_OK) {
      util::warning(std::format("Error while trying to load the sofa file: {}", util::mysofa_error_to_string(error)));
    }

    if (!hrtf) {
      util::warning(std::format("Failed to load SOFA file: {} - Error: {}", file_path, error));
      return kernel_data;
    }

    // Validate the HRTF structure
    if (mysofa_check(hrtf) != MYSOFA_OK) {
      util::warning(std::format("SOFA file validation failed: {}", file_path));
    }

    kernel_data.rate = 48000;  // Default fallback
    kernel_data.original_rate = kernel_data.rate;

    if (hrtf->DataSamplingRate.elements > 0 && hrtf->DataSamplingRate.values) {
      kernel_data.rate = static_cast<uint>(hrtf->DataSamplingRate.values[0]);
      kernel_data.original_rate = kernel_data.rate;

      util::debug(std::format("Found SOFA sampling rate: {} Hz", kernel_data.rate));
    }

    // Get dimensions
    int M = hrtf->M;  // Number of measurements (source positions)
    int R = hrtf->R;  // Number of receivers (ears, usually 2)
    int N = hrtf->N;  // Filter length (samples per IR)
    int E = hrtf->E;  // Number of emitters

    util::debug(std::format("SOFA file measurements: {}", M));
    util::debug(std::format("SOFA file receivers: {}", R));
    util::debug(std::format("SOFA file filter length: {}", N));
    util::debug(std::format("SOFA file emitters: {}", E));

    // Set SOFA metadata
    kernel_data.is_sofa = true;
    kernel_data.sofaMetadata.measurements = M;
    kernel_data.sofaMetadata.database = mysofa_getAttribute(hrtf->attributes, const_cast<char*>("DatabaseName"));

    util::debug(std::format("Database: {}", kernel_data.sofaMetadata.database.toStdString()));

    if (M <= 0 || R < 1 || N <= 0) {
      util::warning(std::format("Invalid SOFA file structure: M={}, R={}, N={}", M, R, N));

      mysofa_free(hrtf);

      return kernel_data;
    }

    int m = 0;

    mysofa_tocartesian(hrtf);

    struct MYSOFA_LOOKUP* lookup = mysofa_lookup_init(hrtf);

    if (!lookup) {
      util::warning("Failed to create SOFA lookup structure.");
    } else {
      util::debug(std::format("Theta min: {}, max: {}", lookup->theta_min, lookup->theta_max));
      util::debug(std::format("Phi min: {}, max: {}", lookup->phi_min, lookup->phi_max));
      util::debug(std::format("Radius min: {}, max: {}", lookup->radius_min, lookup->radius_max));

      kernel_data.sofaMetadata.min_azimuth = lookup->phi_min;
      kernel_data.sofaMetadata.max_azimuth = lookup->phi_max;
      kernel_data.sofaMetadata.min_elevation = lookup->theta_min;
      kernel_data.sofaMetadata.max_elevation = lookup->theta_max;
      kernel_data.sofaMetadata.min_radius = lookup->radius_min;
      kernel_data.sofaMetadata.max_radius = lookup->radius_max;

      float coords[3] = {static_cast<float>(settings->targetSofaAzimuth()),
                         static_cast<float>(settings->targetSofaElevation()),
                         static_cast<float>(settings->targetSofaRadius())};

      mysofa_s2c(coords);

      m = mysofa_lookup(lookup, coords);

      kernel_data.sofaMetadata.index = m;

      util::debug(std::format(
          "For the desired azimuth = {}, elevation = {} and radius = {} the nearest SOFA measurement index is {}",
          settings->targetSofaAzimuth(), settings->targetSofaElevation(), settings->targetSofaRadius(), m));

      mysofa_lookup_free(lookup);

      mysofa_tospherical(hrtf);

      float selected_azimuth = hrtf->SourcePosition.values[(m * 3) + 0];
      float selected_elevation = hrtf->SourcePosition.values[(m * 3) + 1];
      float selected_radius = hrtf->SourcePosition.values[(m * 3) + 2];

      kernel_data.sofaMetadata.azimuth = selected_azimuth;
      kernel_data.sofaMetadata.elevation = selected_elevation;
      kernel_data.sofaMetadata.radius = selected_radius;

      util::debug(std::format("For measurement {} we have azimuth = {}, elevation = {} and radius = {}", m,
                              selected_azimuth, selected_elevation, selected_radius));
    }

    const int RxE_times_N = R * E * N;

    if (E == 1) {
      kernel_data.channel_L.resize(N);
      kernel_data.channel_R.resize(N);
      kernel_data.channels = 2;

      // Left ear (receiver 0)
      for (int n = 0; n < N; n++) {
        kernel_data.channel_L[n] = hrtf->DataIR.values[(m * RxE_times_N) + (0 * N) + n];
      }

      // Right ear (receiver 1)
      if (R > 1) {
        for (int n = 0; n < N; n++) {
          kernel_data.channel_R[n] = hrtf->DataIR.values[(m * RxE_times_N) + (1 * N) + n];
        }
      } else {
        kernel_data.channel_R = kernel_data.channel_L;
      }
    }

    if (R == 2 && E == 2) {
      // Assuming it is True Stereo HRTF: 4 channels

      kernel_data.channels = 4;
      kernel_data.channel_L.resize(N);   // LL
      kernel_data.channel_LR.resize(N);  // LR
      kernel_data.channel_RL.resize(N);  // RL
      kernel_data.channel_R.resize(N);   // RR

      // Left Channel (LL: Emitter 0 to Receiver 0)
      for (int n = 0; n < N; n++) {
        kernel_data.channel_L[n] = hrtf->DataIR.values[(m * RxE_times_N) + (0 * E * N) + (0 * N) + n];
      }

      // Left-to-Right Crosstalk (LR: Emitter 0 to Receiver 1)
      for (int n = 0; n < N; n++) {
        kernel_data.channel_LR[n] = hrtf->DataIR.values[(m * RxE_times_N) + (1 * E * N) + (0 * N) + n];
      }

      // Right-to-Left Crosstalk (RL: Emitter 1 to Receiver 0)
      for (int n = 0; n < N; n++) {
        kernel_data.channel_RL[n] = hrtf->DataIR.values[(m * RxE_times_N) + (0 * E * N) + (1 * N) + n];
      }

      // Right Channel (RR: Emitter 1 to Receiver 1)
      for (int n = 0; n < N; n++) {
        kernel_data.channel_R[n] = hrtf->DataIR.values[(m * RxE_times_N) + (1 * E * N) + (1 * N) + n];
      }
    }

    mysofa_free(hrtf);

    util::debug(std::format("Successfully loaded SOFA file '{}': {} Hz, {} samples", file_path, kernel_data.rate, N));

  } catch (const std::exception& e) {
    util::warning(std::format("Exception while reading SOFA file {}: {}", file_path, e.what()));
  }

  return kernel_data;
}
