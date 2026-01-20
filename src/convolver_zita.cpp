/**
 * Copyright © 2017-2026 Wellington Wallace
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
#include <mutex>
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

auto ConvolverZita::init(ConvolverKernelManager::KernelData data,
                         uint bufferSize,
                         const int& ir_width,
                         const bool& apply_autogain) -> bool {
  std::scoped_lock<std::mutex> lock(util::fftw_lock());

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

  update_ir_width_and_autogain(ir_width, apply_autogain, false);

  float density = 0.5F;

  if (auto ret = conv->configure(2, 2, kernel.sampleCount(), bufferSize, bufferSize, Convproc::MAXPART, density);
      ret != 0) {
    util::warning(std::format("Zita: configure failed: {}", ret));
    return false;
  }

  if (auto ret = conv->impdata_create(0, 0, 1, kernel.channel_L.data(), 0, static_cast<int>(kernel.sampleCount()));
      ret != 0) {
    util::warning(std::format("Zita: left impdata_create failed: {}", ret));
    return false;
  }

  if (auto ret = conv->impdata_create(1, 1, 1, kernel.channel_R.data(), 0, static_cast<int>(kernel.sampleCount()));
      ret != 0) {
    util::warning(std::format("Zita: right impdata_create failed: {}", ret));
    return false;
  }

  if (kernel.channels == 4) {
    if (auto ret = conv->impdata_create(0, 1, 1, kernel.channel_LR.data(), 0, static_cast<int>(kernel.sampleCount()));
        ret != 0) {
      util::warning(std::format("Zita: LR impdata_create failed: {}", ret));
      return false;
    }

    if (auto ret = conv->impdata_create(1, 0, 1, kernel.channel_RL.data(), 0, static_cast<int>(kernel.sampleCount()));
        ret != 0) {
      util::warning(std::format("Zita: RL impdata_create failed: {}", ret));
      return false;
    }
  }

  if (auto ret = conv->start_process(ZITA_SCHED_PRIORITY, ZITA_SCHED_CLASS); ret != 0) {
    util::warning(std::format("Zita: start_process failed: {}", ret));

    conv->cleanup();

    return false;
  }

  ready = true;

  return ready;
}

auto ConvolverZita::process(std::span<float> left, std::span<float> right) -> bool {
  if (!ready || !conv || conv->state() != Convproc::ST_PROC) {
    return false;
  }

  if (left.size() != bufferSize || right.size() != bufferSize) {
    util::warning(
        std::format("Mismatch in buffer sizes! Zita wants {} but Pipewire is using {}. Aborting zita process!",
                    bufferSize, left.size()));

    ready = false;

    return false;
  }

  auto convLeftIn = std::span{conv->inpdata(0), bufferSize};
  auto convRightIn = std::span{conv->inpdata(1), bufferSize};
  auto convLeftOut = std::span{conv->outdata(0), bufferSize};
  auto convRightOut = std::span{conv->outdata(1), bufferSize};

  std::ranges::copy(left, convLeftIn.begin());
  std::ranges::copy(right, convRightIn.begin());

  std::scoped_lock<std::mutex> lock(util::fftw_lock());

  if (auto ret = conv->process(true); ret != 0) {
    util::warning(std::format("Zita: process failed: {}", ret));

    ready = false;

    return false;
  }

  std::ranges::copy(convLeftOut, left.begin());
  std::ranges::copy(convRightOut, right.begin());

  return true;
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

  float power_LL = 0.0F;
  float power_RR = 0.0F;
  float power_LR = 0.0F;
  float power_RL = 0.0F;

  for (uint i = 0; i < kernel.sampleCount(); i++) {
    power_LL += kernel.channel_L[i] * kernel.channel_L[i];
    power_RR += kernel.channel_R[i] * kernel.channel_R[i];

    if (kernel.channels == 4) {
      power_LR += kernel.channel_LR[i] * kernel.channel_LR[i];
      power_RL += kernel.channel_RL[i] * kernel.channel_RL[i];
    }
  }

  const float power = std::max({power_LL, power_RR, power_LR, power_RL});

  const float autogain = std::min(1.0F, 1.0F / std::sqrt(power));

  util::debug(std::format("autogain factor: {}", autogain));

  for (uint i = 0; i < kernel.sampleCount(); i++) {
    kernel.channel_L[i] *= autogain;
    kernel.channel_R[i] *= autogain;

    if (kernel.channels == 4) {
      kernel.channel_LR[i] *= autogain;
      kernel.channel_RL[i] *= autogain;
    }
  }
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

  for (uint i = 0; i < kernel.sampleCount(); i++) {
    const float LL = kernel.channel_L[i];
    const float RR = kernel.channel_R[i];

    float LR = 0.0F;
    float RL = 0.0F;

    if (kernel.channels == 4) {
      LR = kernel.channel_LR[i];
      RL = kernel.channel_RL[i];
    }

    // Apply width to direct paths
    float new_LL = LL + (x * RR);
    float new_RR = RR + (x * LL);

    // Apply complementary width to cross paths
    float new_LR = LR - (x * RL);
    float new_RL = RL - (x * LR);

    kernel.channel_L[i] = new_LL;
    kernel.channel_R[i] = new_RR;

    if (kernel.channels == 4) {
      kernel.channel_LR[i] = new_LR;
      kernel.channel_RL[i] = new_RL;
    }
  }
}

void ConvolverZita::update_ir_width_and_autogain(const int& ir_width,
                                                 const bool& apply_autogain,
                                                 const bool& clear_zita) {
  reset_kernel_to_original();

  set_kernel_stereo_width(ir_width);

  if (apply_autogain) {
    apply_kernel_autogain();
  }

  if (clear_zita && conv) {
    conv->impdata_clear(0, 0);
    conv->impdata_clear(1, 1);

    conv->impdata_update(0, 0, 1, kernel.channel_L.data(), 0, static_cast<int>(kernel.sampleCount()));
    conv->impdata_update(1, 1, 1, kernel.channel_R.data(), 0, static_cast<int>(kernel.sampleCount()));

    if (kernel.channels == 4) {
      conv->impdata_clear(0, 1);
      conv->impdata_clear(1, 0);

      conv->impdata_update(0, 1, 1, kernel.channel_LR.data(), 0, static_cast<int>(kernel.sampleCount()));
      conv->impdata_update(1, 0, 1, kernel.channel_RL.data(), 0, static_cast<int>(kernel.sampleCount()));
    }
  }
}
