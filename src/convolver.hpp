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

#include <qlist.h>
#include <qobject.h>
#include <qpoint.h>
#include <qtmetamacros.h>
#include <sys/types.h>
#include <zita-convolver.h>
#include <QString>
#include <QThread>
#include <span>
#include <string>
#include <vector>
#include "convolver_kernel_fft.hpp"
#include "convolver_kernel_manager.hpp"
#include "convolver_zita.hpp"
#include "easyeffects_db_convolver.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"

class ConvolverWorker : public QObject {
  Q_OBJECT

 public Q_SLOTS:
  void calculateFFT(std::vector<float> kernel_L, std::vector<float> kernel_R, int kernel_rate, int interpPoints);

 Q_SIGNALS:
  void onNewKernel(ConvolverKernelManager::KernelData data);

  void onNewChartMag(QList<QPointF> mag_L, QList<QPointF> mag_R);

  void onNewSpectrum(QList<QPointF> linear_L, QList<QPointF> linear_R, QList<QPointF> log_L, QList<QPointF> log_R);

  void onInvalidKernel(QString name);
};

class Convolver : public PluginBase {
  Q_OBJECT

  Q_PROPERTY(bool kernelIsInitialized MEMBER kernel_is_initialized NOTIFY kernelInitializedChanged)

  Q_PROPERTY(QString kernelRate MEMBER kernelRate NOTIFY kernelRateChanged)
  Q_PROPERTY(QString kernelSamples MEMBER kernelSamples NOTIFY kernelSamplesChanged)
  Q_PROPERTY(QString kernelDuration MEMBER kernelDuration NOTIFY kernelDurationChanged)

  Q_PROPERTY(int interpPoints MEMBER interpPoints NOTIFY interpPointsChanged)

  Q_PROPERTY(QList<QPointF> chartMagL MEMBER chartMagL NOTIFY chartMagLChanged)
  Q_PROPERTY(QList<QPointF> chartMagR MEMBER chartMagR NOTIFY chartMagRChanged)

  Q_PROPERTY(QList<QPointF> chartMagLfftLinear MEMBER chartMagLfftLinear NOTIFY chartMagLfftLinearChanged)
  Q_PROPERTY(QList<QPointF> chartMagRfftLinear MEMBER chartMagRfftLinear NOTIFY chartMagRfftLinearChanged)
  Q_PROPERTY(QList<QPointF> chartMagLfftLog MEMBER chartMagLfftLog NOTIFY chartMagLfftLogChanged)
  Q_PROPERTY(QList<QPointF> chartMagRfftLog MEMBER chartMagRfftLog NOTIFY chartMagRfftLogChanged)

 public:
  Convolver(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  Convolver(const Convolver&) = delete;
  auto operator=(const Convolver&) -> Convolver& = delete;
  Convolver(const Convolver&&) = delete;
  auto operator=(const Convolver&&) -> Convolver& = delete;
  ~Convolver() override;

  void reset() override;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out,
               std::span<float>& probe_left,
               std::span<float>& probe_right) override;

  auto get_latency_seconds() -> float override;

  Q_INVOKABLE void combineKernels(const QString& kernel1, const QString& kernel2, const QString& outputName);

 Q_SIGNALS:
  void newKernelLoaded(QString name, bool success);

  void kernelInitializedChanged();
  void kernelRateChanged();
  void kernelSamplesChanged();
  void kernelDurationChanged();

  void interpPointsChanged();

  void chartMagLChanged();
  void chartMagRChanged();

  void chartMagLfftLinearChanged();
  void chartMagRfftLinearChanged();
  void chartMagLfftLogChanged();
  void chartMagRfftLogChanged();

  void kernelCombinationStopped();

 private:
  db::Convolver* settings = nullptr;

  bool kernel_is_initialized = false;
  bool n_samples_is_power_of_2 = true;
  bool ready = false;
  bool destructor_called = false;
  bool notify_latency = false;

  uint blocksize = 512U;
  uint latency_n_frames = 0U;

  int interpPoints = 1000;

  float dry = 0.0F, wet = 1.0F;

  QString kernelRate;
  QString kernelSamples;
  QString kernelDuration;

  std::vector<float> data_L, data_R;
  std::vector<float> buf_in_L, buf_in_R;
  std::vector<float> buf_out_L, buf_out_R;

  QList<QPointF> chartMagL, chartMagR, chartMagLfftLinear, chartMagRfftLinear, chartMagLfftLog, chartMagRfftLog;

  ConvolverKernelManager kernel_manager;

  ConvolverKernelFFT kernel_fft;

  ConvolverZita zita;

  ConvolverWorker* worker;

  QThread workerThread;

  void load_kernel_file(const bool& init_zita);

  void combine_kernels(const std::string& kernel_1_name,
                       const std::string& kernel_2_name,
                       const std::string& output_file_name);

  void chart_kernel_fft(const std::vector<float>& kernel_L,
                        const std::vector<float>& kernel_R,
                        const float& kernel_rate);

  void clear_chart_data();

  void update_ir_width_and_autogain();
};
