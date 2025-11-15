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

#include <zita-convolver.h>
#include <cstdint>
#include <memory>
#include <span>

class ConvolverZita {
 public:
  ConvolverZita();
  ~ConvolverZita();

  ConvolverZita(const ConvolverZita&) = delete;
  auto operator=(const ConvolverZita&) -> ConvolverZita& = delete;

  // Movable if needed
  ConvolverZita(ConvolverZita&&) noexcept = default;
  auto operator=(ConvolverZita&&) noexcept -> ConvolverZita& = default;

  auto init(uint32_t sampleCount, uint32_t blockSize, std::span<float> kernelL, std::span<float> kernelR) -> bool;

  void process(std::span<float> dataLeft, std::span<float> dataRight);

 private:
  std::unique_ptr<Convproc> conv;

  bool ready = false;

  uint32_t bufferSize = 0;
};
