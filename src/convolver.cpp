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

#include "convolver.hpp"
#include <fftw3.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qpoint.h>
#include <qstandardpaths.h>
#include <qthread.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <sched.h>
#include <sndfile.h>
#include <sys/types.h>
#include <zita-convolver.h>
#include <QString>
#include <algorithm>
#include <cstddef>
#include <format>
#include <mutex>
#include <sndfile.hh>
#include <span>
#include <string>
#include <vector>
#include "convolver_kernel_fft.hpp"
#include "convolver_kernel_manager.hpp"
#include "db_manager.hpp"
#include "easyeffects_db_convolver.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

Convolver::Convolver(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::convolver,
                 tags::plugin_package::Package::zita,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(
          db::Manager::self().get_plugin_db<db::Convolver>(pipe_type,
                                                           tags::plugin_name::BaseName::convolver + "#" + instance_id)),
      kernel_manager(ConvolverKernelManager(settings, pipe_type)),
      worker(new ConvolverWorker) {
  init_common_controls<db::Convolver>(settings);

  dry = (settings->dry() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->dry()));

  wet = (settings->wet() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->wet()));

  connect(settings, &db::Convolver::kernelNameChanged, [&]() { load_kernel_file(true, rate); });

  connect(settings, &db::Convolver::irWidthChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);
    zita.update_ir_width_and_autogain(settings->irWidth(), settings->autogain(), true);
  });

  connect(settings, &db::Convolver::autogainChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);
    zita.update_ir_width_and_autogain(settings->irWidth(), settings->autogain(), true);
  });

  connect(settings, &db::Convolver::dryChanged, [&]() {
    dry =
        (settings->dry() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->dry()));
  });

  connect(settings, &db::Convolver::wetChanged, [&]() {
    wet =
        (settings->wet() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->wet()));
  });

  // Preparing the worker thread

  worker->moveToThread(&workerThread);

  connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);

  connect(
      worker, &ConvolverWorker::onNewKernel, this,
      [this](ConvolverKernelManager::KernelData data, bool init_zita) {
        kernel_is_initialized = data.isValid();

        if (kernel_is_initialized) {
          kernelIsSofa = data.is_sofa;
          kernelRate = QString::fromStdString(util::to_string(data.original_rate));
          kernelSamples = QString::fromStdString(util::to_string(data.sampleCount()));
          kernelDuration = QString::fromStdString(util::to_string(data.duration()));
          kernelChannels = data.channels;

          Q_EMIT kernelIsSofaChanged();
          Q_EMIT kernelRateChanged();
          Q_EMIT kernelDurationChanged();
          Q_EMIT kernelSamplesChanged();
          Q_EMIT kernelChannelsChanged();
          Q_EMIT newKernelLoaded(data.name, true);

          if (data.is_sofa) {
            sofaDatabase = data.sofaMetadata.database;
            sofaMeasurements = data.sofaMetadata.measurements;
            sofaIndex = data.sofaMetadata.index;
            sofaAzimuth = data.sofaMetadata.azimuth;
            sofaElevation = data.sofaMetadata.elevation;
            sofaRadius = data.sofaMetadata.radius;

            sofaMinAzimuth = data.sofaMetadata.min_azimuth;
            sofaMaxAzimuth = data.sofaMetadata.max_azimuth;
            sofaMinElevation = data.sofaMetadata.min_elevation;
            sofaMaxElevation = data.sofaMetadata.max_elevation;
            sofaMinRadius = data.sofaMetadata.min_radius;
            sofaMaxRadius = data.sofaMetadata.max_radius;

            Q_EMIT sofaDatabaseChanged();
            Q_EMIT sofaMeasurementsChanged();
            Q_EMIT sofaIndexChanged();
            Q_EMIT sofaAzimuthChanged();
            Q_EMIT sofaElevationChanged();
            Q_EMIT sofaRadiusChanged();

            Q_EMIT sofaMinAzimuthChanged();
            Q_EMIT sofaMaxAzimuthChanged();
            Q_EMIT sofaMinElevationChanged();
            Q_EMIT sofaMaxElevationChanged();
            Q_EMIT sofaMinRadiusChanged();
            Q_EMIT sofaMaxRadiusChanged();
          }

          if (init_zita) {
            std::scoped_lock<std::mutex> lock(data_mutex);

            auto success = zita.init(data, blocksize, settings->irWidth(), settings->autogain());

            if (!success) {
              util::warning(std::format("{} Zita init failed", log_tag));
            }

            ready = success;
          }
        }
      },
      Qt::QueuedConnection);

  connect(
      worker, &ConvolverWorker::onInvalidKernel, this,
      [this](QString name) {
        clear_chart_data();

        Q_EMIT newKernelLoaded(name, false);

        util::warning(
            std::format("{}{}: irs filename is invalid. Entering passthrough mode...", log_tag, name.toStdString()));
      },
      Qt::QueuedConnection);

  connect(
      worker, &ConvolverWorker::onNewChartMag, this,
      [this](QList<QPointF> mag_L, QList<QPointF> mag_R) {
        chartMagL.swap(mag_L);
        chartMagR.swap(mag_R);

        Q_EMIT chartMagLChanged();
        Q_EMIT chartMagRChanged();
      },
      Qt::QueuedConnection);

  connect(
      worker, &ConvolverWorker::onNewSpectrum, this,
      [this](QList<QPointF> linear_L, QList<QPointF> linear_R, QList<QPointF> log_L, QList<QPointF> log_R) {
        chartMagLfftLinear.swap(linear_L);
        chartMagRfftLinear.swap(linear_R);
        chartMagLfftLog.swap(log_L);
        chartMagRfftLog.swap(log_R);

        Q_EMIT chartMagLfftLinearChanged();
        Q_EMIT chartMagRfftLinearChanged();
        Q_EMIT chartMagLfftLogChanged();
        Q_EMIT chartMagRfftLogChanged();
      },
      Qt::QueuedConnection);

  QMetaObject::invokeMethod(
      worker,
      [this] {
        if (ready || destructor_called) {
          return;
        }

        /**
         * Setting valid rate instead of zero allows the convolver ui to properly show the impulse response file
         * parameters even if nothing is playing audio.
         */

        uint r = 0;

        util::str_to_num(pw::Manager::self().defaultClockRate.toStdString(), r);

        load_kernel_file(false, r);
      },
      Qt::QueuedConnection);
}

Convolver::~Convolver() {
  std::scoped_lock<std::mutex> lock(data_mutex);

  destructor_called = true;
  ready = false;

  if (connected_to_pw) {
    disconnect_from_pw();
  }

  zita.stop();

  settings->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void Convolver::reset() {
  settings->setDefaults();
}

void Convolver::setup() {
  std::scoped_lock<std::mutex> lock(data_mutex);

  ready = false;

  /**
   * As zita uses fftw we have to be careful when reinitializing it.
   * The thread that creates the fftw plan has to be the same that destroys it.
   * Otherwise segmentation faults can happen. As we do not want to do this
   * initializing in the plugin realtime thread we send it to the worker thread
   */

  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)

  QMetaObject::invokeMethod(
      worker,
      [this] {
        if (destructor_called) {
          return;
        }

        blocksize = n_samples;

        n_samples_is_power_of_2 = (n_samples & (n_samples - 1U)) == 0U && n_samples != 0U;

        if (!n_samples_is_power_of_2) {
          while ((blocksize & (blocksize - 1)) != 0 && blocksize > 2) {
            blocksize--;
          }
        }

        blocksize = std::max<uint>(blocksize, 64);  // zita does not work with less than 64

        buf_in_L.clear();
        buf_in_R.clear();
        buf_out_L.clear();
        buf_out_R.clear();

        data_L.resize(blocksize);
        data_R.resize(blocksize);

        notify_latency = true;

        latency_n_frames = 0U;

        load_kernel_file(true, rate);
      },
      Qt::QueuedConnection);

  // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
}

void Convolver::process(std::span<float>& left_in,
                        std::span<float>& right_in,
                        std::span<float>& left_out,
                        std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    return;
  }

  if (!ready) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    if (output_gain != 1.0F) {
      apply_gain(left_out, right_out, output_gain);
    }

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  if (n_samples_is_power_of_2) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    zita.process(left_out, right_out);
  } else {
    buf_in_L.insert(buf_in_L.end(), left_in.begin(), left_in.end());
    buf_in_R.insert(buf_in_R.end(), right_in.begin(), right_in.end());

    while (buf_in_L.size() >= blocksize) {
      util::copy_bulk(buf_in_L, data_L);
      util::copy_bulk(buf_in_R, data_R);

      zita.process(data_L, data_R);

      buf_out_L.insert(buf_out_L.end(), data_L.begin(), data_L.end());
      buf_out_R.insert(buf_out_R.end(), data_R.begin(), data_R.end());
    }

    // copying the processed samples to the output buffers

    if (buf_out_L.size() >= n_samples) {
      util::copy_bulk(buf_out_L, left_out);
      util::copy_bulk(buf_out_R, right_out);
    } else {
      const uint offset = n_samples - buf_out_L.size();

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
  }

  for (size_t n = 0; n < left_out.size(); n++) {
    left_out[n] = (wet * left_out[n]) + (dry * left_in[n]);

    right_out[n] = (wet * right_out[n]) + (dry * right_in[n]);
  }

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (notify_latency) {
    latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(std::format("{}{} latency: {} s", log_tag, name.toStdString(), latency_value));

    update_filter_params();

    notify_latency = false;
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void Convolver::process([[maybe_unused]] std::span<float>& left_in,
                        [[maybe_unused]] std::span<float>& right_in,
                        [[maybe_unused]] std::span<float>& left_out,
                        [[maybe_unused]] std::span<float>& right_out,
                        [[maybe_unused]] std::span<float>& probe_left,
                        [[maybe_unused]] std::span<float>& probe_right) {}

void Convolver::load_kernel_file(const bool& init_zita, const uint& server_sampling_rate) {
  if (destructor_called) {
    return;
  }

  const auto name = settings->kernelName();

  auto kernel_data = kernel_manager.loadKernel(name.toStdString());

  if (!kernel_data.isValid()) {
    Q_EMIT worker->onInvalidKernel(name);

    return;
  }

  if (server_sampling_rate != 0 && kernel_data.rate != server_sampling_rate) {
    util::debug(std::format("{}{} kernel has {} rate. Resampling it to {}", log_tag, name.toStdString(),
                            kernel_data.rate, server_sampling_rate));

    kernel_data = ConvolverKernelManager::resampleKernel(kernel_data, server_sampling_rate);
  }

  const auto dt = 1.0 / kernel_data.rate;

  std::vector<double> time_axis(kernel_data.sampleCount());

  for (size_t n = 0U; n < time_axis.size(); n++) {
    time_axis[n] = static_cast<double>(n) * dt;
  }

  auto x_linear = util::linspace(time_axis.front(), time_axis.back(), interpPoints);

  std::vector<double> copy_helper(kernel_data.sampleCount());

  std::ranges::copy(kernel_data.channel_L, copy_helper.begin());

  auto magL = util::interpolate(time_axis, copy_helper, x_linear);

  std::ranges::copy(kernel_data.channel_R, copy_helper.begin());

  auto magR = util::interpolate(time_axis, copy_helper, x_linear);

  QList<QPointF> chart_mag_L(interpPoints);
  QList<QPointF> chart_mag_R(interpPoints);

  for (qsizetype n = 0; n < interpPoints; n++) {
    chart_mag_L[n] = QPointF(x_linear[n], magL[n]);
    chart_mag_R[n] = QPointF(x_linear[n], magR[n]);
  }

  util::debug(std::format("{}{}: kernel correctly loaded", log_tag, name.toStdString()));

  std::scoped_lock<std::mutex> lock(data_mutex);

  ConvolverKernelFFT kernel_fft;

  kernel_fft.calculate_fft(kernel_data.channel_L, kernel_data.channel_R, kernel_data.original_rate, interpPoints);

  Q_EMIT worker->onNewChartMag(chart_mag_L, chart_mag_R);

  Q_EMIT worker->onNewSpectrum(kernel_fft.linear_L, kernel_fft.linear_R, kernel_fft.log_L, kernel_fft.log_R);

  Q_EMIT worker->onNewKernel(kernel_data, init_zita);
}

auto Convolver::get_latency_seconds() -> float {
  return this->latency_value;
}

void Convolver::combine_kernels(const std::string& kernel_1_name,
                                const std::string& kernel_2_name,
                                const std::string& output_file_name) {
  kernel_manager.combineKernels(kernel_1_name, kernel_2_name, output_file_name);

  Q_EMIT kernelCombinationStopped();
}

void Convolver::combineKernels(const QString& kernel1, const QString& kernel2, const QString& outputName) {
  QMetaObject::invokeMethod(
      worker,
      [this, kernel1, kernel2, outputName] {
        combine_kernels(kernel1.toStdString(), kernel2.toStdString(), outputName.toStdString());
      },
      Qt::QueuedConnection);
}

void Convolver::clear_chart_data() {
  chartMagL.clear();
  chartMagR.clear();
  chartMagLfftLinear.clear();
  chartMagRfftLinear.clear();
  chartMagLfftLog.clear();
  chartMagRfftLog.clear();
}

void Convolver::applySofaOrientation() {
  setup();
}