/**
 * Copyright © 2017-2025 Wellington Wallace
 *
 * This file is part of Easy Effects
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
#include <zita-convolver.h>
#include <span>
#include "convolver_kernel_manager.hpp"

class ConvolverZita {
 public:
  ConvolverZita();
  ~ConvolverZita();

  ConvolverZita(const ConvolverZita&) = delete;
  auto operator=(const ConvolverZita&) -> ConvolverZita& = delete;

  // Movable if needed
  ConvolverZita(ConvolverZita&&) noexcept = default;
  auto operator=(ConvolverZita&&) noexcept -> ConvolverZita& = default;

  auto init(ConvolverKernelManager::KernelData data, uint bufferSize) -> bool;

  void process(std::span<float> dataLeft, std::span<float> dataRight);

  void stop();

 private:
  bool ready = false;

  uint bufferSize = 0;

  ConvolverKernelManager::KernelData kernel;

  Convproc* conv = nullptr;
};
