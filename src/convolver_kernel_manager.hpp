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

#pragma once

#include <qtypes.h>
#include <QString>
#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>
#include "easyeffects_db_convolver.h"
#include "pipeline_type.hpp"

class ConvolverKernelManager {
 public:
  struct KernelData {
    bool is_sofa = false;

    uint rate = 0;
    uint original_rate = 0;
    uint channels = 0;

    QString name;
    QString file_path;

    std::vector<float> channel_L;
    std::vector<float> channel_LR;
    std::vector<float> channel_RL;
    std::vector<float> channel_R;

    struct SofaMetadata {
      QString database;

      int index = 0;  // Which measurement to use
      int measurements = 1;

      float azimuth = 0.0;    // In degrees
      float elevation = 0.0;  // In degrees
      float radius = 0.0;     // In meters

      float min_azimuth = 0.0F;
      float max_azimuth = 0.0F;
      float min_elevation = 0.0F;
      float max_elevation = 0.0F;
      float min_radius = 0.0F;
      float max_radius = 0.0F;

    } sofaMetadata;

    [[nodiscard]] auto isValid() const -> bool;

    [[nodiscard]] auto duration() const -> double;

    [[nodiscard]] auto sampleCount() const -> size_t;
  };

  ConvolverKernelManager(db::Convolver* settings, const PipelineType& pipeline_type);

  auto loadKernel(const std::string& name) -> KernelData;

  auto combineKernels(const std::string& kernel1_name, const std::string& kernel2_name, const std::string& output_name)
      -> bool;

  auto searchKernelPath(const std::string& name) -> std::string;

  static auto resampleKernel(const KernelData& kernel, const uint& target_rate) -> KernelData;

  static void normalizeKernel(KernelData& kernel);

  auto saveKernel(const KernelData& kernel, const std::string& file_name) -> bool;

  auto readSofaKernelFile(const std::string& file_path) -> KernelData;

 private:
  db::Convolver* settings = nullptr;

  static constexpr std::string irs_ext = ".irs";
  static constexpr std::string sofa_ext = ".sofa";

  PipelineType pipeline_type;

  std::string app_data_dir;
  std::string local_dir_irs;

  std::vector<std::string> system_data_dir_irs;

  static auto readKernelFile(const std::string& file_path) -> KernelData;

  static auto validateKernel(const KernelData& kernel) -> bool;

  static auto findKernelInDirectory(const std::filesystem::path& directory, const std::string& kernel_name)
      -> std::string;

  static auto directConvolution(const std::vector<float>& a, const std::vector<float>& b) -> std::vector<float>;

  static auto getFileExtension(const std::string& file_path) -> std::string;
};
