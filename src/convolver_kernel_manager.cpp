/**
 * Copyright © 2017-2025 Wellington Wallace
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
#include <qstandardpaths.h>
#include <qtypes.h>
#include <sndfile.h>
#include <algorithm>
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
#include "pipeline_type.hpp"
#include "resampler.hpp"
#include "tags_app.hpp"
#include "util.hpp"

ConvolverKernelManager::ConvolverKernelManager(const PipelineType& pipeline_type)
    : pipeline_type(pipeline_type),
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
  return rate > 0 && !left_channel.empty() && !right_channel.empty() && left_channel.size() == right_channel.size();
}

auto ConvolverKernelManager::KernelData::duration() const -> double {
  if (rate == 0 || left_channel.empty()) {
    return 0.0;
  }

  return static_cast<double>(left_channel.size() - 1) / static_cast<double>(rate);
}

auto ConvolverKernelManager::KernelData::sampleCount() const -> size_t {
  return left_channel.size();
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

  auto kernel_data = readKernelFile(file_path);
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

  const auto target_rate = std::max(kernel1.rate, kernel2.rate);
  const auto resampled_kernel1 = (kernel1.rate != target_rate) ? resampleKernel(kernel1, target_rate) : kernel1;
  const auto resampled_kernel2 = (kernel2.rate != target_rate) ? resampleKernel(kernel2, target_rate) : kernel2;

  auto combined_kernel_L = directConvolution(resampled_kernel1.left_channel, resampled_kernel2.left_channel);
  auto combined_kernel_R = directConvolution(resampled_kernel1.right_channel, resampled_kernel2.right_channel);

  KernelData combined_kernel;

  combined_kernel.rate = target_rate;
  combined_kernel.left_channel = std::move(combined_kernel_L);
  combined_kernel.right_channel = std::move(combined_kernel_R);
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
  const auto irs_filename = name + irs_ext;

  const auto community_package = (pipeline_type == PipelineType::input) ? DbMain::lastLoadedInputCommunityPackage()
                                                                        : DbMain::lastLoadedOutputCommunityPackage();

  std::string irs_full_path;

  if (community_package.isEmpty()) {
    // Search local irs file.
    const auto local_irs_file = std::filesystem::path{local_dir_irs + "/" + irs_filename};

    if (std::filesystem::exists(local_irs_file)) {
      irs_full_path = local_irs_file.string();
    }
  } else {
    // Search irs file in community package paths.
    for (const auto& xdg_irs_dir : system_data_dir_irs) {
      if (util::search_filename(std::filesystem::path{xdg_irs_dir + "/" + community_package.toStdString()},
                                irs_filename, irs_full_path, 3U)) {
        break;
      }
    }
  }

  return irs_full_path;
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

  KernelData resampled_kernel;

  resampled_kernel.rate = target_rate;
  resampled_kernel.name = kernel.name;

  util::debug(
      std::format("Resampling kernel '{}' from {} Hz to {} Hz", kernel.name.toStdString(), kernel.rate, target_rate));

  // Resample left channel

  auto resampler = std::make_unique<Resampler>(kernel.rate, target_rate);

  resampled_kernel.left_channel = resampler->process(kernel.left_channel, true);

  // Resample right channel

  resampler = std::make_unique<Resampler>(kernel.rate, target_rate);

  resampled_kernel.right_channel = resampler->process(kernel.right_channel, true);

  return resampled_kernel;
}

void ConvolverKernelManager::normalizeKernel(KernelData& kernel) {
  if (!kernel.isValid()) {
    return;
  }

  // Find the maximum absolute value across both channels
  const auto max_abs_left =
      std::ranges::max(kernel.left_channel, [](const auto& a, const auto& b) { return std::fabs(a) < std::fabs(b); });

  const auto max_abs_right =
      std::ranges::max(kernel.right_channel, [](const auto& a, const auto& b) { return std::fabs(a) < std::fabs(b); });

  const auto peak = std::max(std::fabs(max_abs_left), std::fabs(max_abs_right));

  if (peak <= 1e-6) {
    return;
  }

  // Normalize both channels
  const auto normalize_lambda = [peak](auto& sample) { sample /= peak; };

  std::ranges::for_each(kernel.left_channel, normalize_lambda);
  std::ranges::for_each(kernel.right_channel, normalize_lambda);
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
    std::vector<float> buffer(kernel.left_channel.size() * 2);

    for (size_t i = 0; i < kernel.left_channel.size(); ++i) {
      buffer[2 * i] = kernel.left_channel[i];
      buffer[(2 * i) + 1] = kernel.right_channel[i];
    }

    // Write to file
    const auto mode = SFM_WRITE;
    const auto format = SF_FORMAT_WAV | SF_FORMAT_PCM_32;
    const auto channels = 2;

    SndfileHandle sndfile(file_path.string(), mode, format, channels, kernel.rate);

    if (sndfile.error() != 0) {
      util::warning(std::format("Failed to create kernel file: {}", sndfile.strError()));
      return false;
    }

    const auto frames_written = sndfile.writef(buffer.data(), static_cast<sf_count_t>(kernel.left_channel.size()));

    if (frames_written != static_cast<sf_count_t>(kernel.left_channel.size())) {
      util::warning(std::format("Failed to write all kernel data: {} of {} frames written", frames_written,
                                kernel.left_channel.size()));
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

    if (sndfile.channels() != 2 || sndfile.frames() == 0) {
      util::warning(std::format("Kernel file must be stereo and non-empty: {}", file_path));
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
    kernel_data.left_channel.resize(sndfile.frames());
    kernel_data.right_channel.resize(sndfile.frames());

    for (size_t i = 0; i < static_cast<size_t>(sndfile.frames()); ++i) {
      kernel_data.left_channel[i] = buffer[2 * i];
      kernel_data.right_channel[i] = buffer[(2 * i) + 1];
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

  if (kernel.left_channel.empty() || kernel.right_channel.empty()) {
    util::warning("Kernel channels are empty");
    return false;
  }

  if (kernel.left_channel.size() != kernel.right_channel.size()) {
    util::warning("Kernel channels have different sizes");
    return false;
  }

  const auto has_invalid_values = [](const std::vector<float>& channel) {
    return std::ranges::any_of(channel, [](const auto& value) { return std::isnan(value) || std::isinf(value); });
  };

  if (has_invalid_values(kernel.left_channel) || has_invalid_values(kernel.right_channel)) {
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