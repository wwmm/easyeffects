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

#include "convolver_zita.hpp"
#include <qtypes.h>
#include <sched.h>
#include <zita-convolver.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <format>
#include <span>
#include <thread>
#include "convolver_kernel_manager.hpp"
#include "util.hpp"

namespace {

constexpr auto ZITA_SCHED_PRIORITY = 0;
constexpr auto ZITA_SCHED_CLASS = SCHED_FIFO;

}  // namespace

ConvolverZita::ConvolverZita() = default;

ConvolverZita::~ConvolverZita() {
  stop();

  delete conv;

  conv = nullptr;
}

void ConvolverZita::stop() {
  ready = false;

  if (conv) {
    conv->stop_process();

    while (!conv->check_stop()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
}

auto ConvolverZita::init(ConvolverKernelManager::KernelData data, uint bufferSize) -> bool {
  ready = false;

  if (conv != nullptr) {
    conv->stop_process();

    while (!conv->check_stop()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    delete conv;

    conv = nullptr;
  }

  conv = new Convproc();

  conv->set_options(0);

  kernel = data;
  original_kernel = kernel;

  this->bufferSize = bufferSize;

  if (auto ret = conv->configure(2, 2, kernel.sampleCount(), bufferSize, bufferSize, bufferSize, 0.0F); ret != 0) {
    util::warning(std::format("Zita: configure failed: {}", ret));
    return false;
  }

  if (auto ret = conv->impdata_create(0, 0, 1, kernel.left_channel.data(), 0, static_cast<int>(kernel.sampleCount()));
      ret != 0) {
    util::warning(std::format("Zita: left impdata_create failed: {}", ret));
    return false;
  }

  if (auto ret = conv->impdata_create(1, 1, 1, kernel.right_channel.data(), 0, static_cast<int>(kernel.sampleCount()));
      ret != 0) {
    util::warning(std::format("Zita: right impdata_create failed: {}", ret));
    return false;
  }

  if (auto ret = conv->start_process(ZITA_SCHED_PRIORITY, ZITA_SCHED_CLASS); ret != 0) {
    util::warning(std::format("Zita: start_process failed: {}", ret));

    conv->cleanup();

    return false;
  }

  ready = true;

  return ready;
}

void ConvolverZita::process(std::span<float> left, std::span<float> right) {
  if (!ready || !conv || conv->state() != Convproc::ST_PROC) {
    return;
  }

  if (left.size() != bufferSize || right.size() != bufferSize) {
    util::warning(
        std::format("Mismatch in buffer sizes! Zita wants {} but Pipewire is using {}. Aborting zita process!",
                    bufferSize, left.size()));

    ready = false;

    return;
  }

  auto convLeftIn = std::span{conv->inpdata(0), bufferSize};
  auto convRightIn = std::span{conv->inpdata(1), bufferSize};
  auto convLeftOut = std::span{conv->outdata(0), bufferSize};
  auto convRightOut = std::span{conv->outdata(1), bufferSize};

  std::ranges::copy(left, convLeftIn.begin());
  std::ranges::copy(right, convRightIn.begin());

  if (auto ret = conv->process(true); ret != 0) {
    util::warning(std::format("Zita: process failed: {}", ret));

    ready = false;

    return;
  }

  std::ranges::copy(convLeftOut, left.begin());
  std::ranges::copy(convRightOut, right.begin());
}

void ConvolverZita::reset_kernel_to_original() {
  kernel = original_kernel;
}

void ConvolverZita::apply_kernel_autogain() {
  if (!kernel.isValid()) {
    return;
  }

  ConvolverKernelManager::normalizeKernel(kernel);

  // find average power

  float power_L = 0.0F;
  float power_R = 0.0F;

  std::ranges::for_each(kernel.left_channel, [&](const auto& v) { power_L += v * v; });
  std::ranges::for_each(kernel.right_channel, [&](const auto& v) { power_R += v * v; });

  const float power = std::max(power_L, power_R);

  const float autogain = std::min(1.0F, 1.0F / std::sqrt(power));

  util::debug(std::format("autogain factor: {}", autogain));

  std::ranges::for_each(kernel.left_channel, [&](auto& v) { v *= autogain; });
  std::ranges::for_each(kernel.right_channel, [&](auto& v) { v *= autogain; });
}

/**
 * Mid-Side based Stereo width effect
 * taken from https://github.com/tomszilagyi/ir.lv2/blob/automatable/ir.cc
 */
void ConvolverZita::set_kernel_stereo_width(const int& ir_width) {
  if (!kernel.isValid()) {
    return;
  }

  const float w = static_cast<float>(ir_width) * 0.01F;
  const float x = (1.0F - w) / (1.0F + w);  // M-S coeff.; L_out = L + x*R; R_out = R + x*L

  for (uint i = 0U; i < kernel.sampleCount(); i++) {
    const auto L = kernel.left_channel[i];
    const auto R = kernel.right_channel[i];

    kernel.left_channel[i] = L + (x * R);
    kernel.right_channel[i] = R + (x * L);
  }
}
