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

 Q_SIGNALS:
  void onNewKernel(ConvolverKernelManager::KernelData data, bool init_zita);

  void onNewChartMag(QList<QPointF> mag_L, QList<QPointF> mag_R);

  void onNewSpectrum(QList<QPointF> linear_L, QList<QPointF> linear_R, QList<QPointF> log_L, QList<QPointF> log_R);

  void onInvalidKernel(QString name);
};

class Convolver : public PluginBase {
  Q_OBJECT

  Q_PROPERTY(bool kernelIsInitialized MEMBER kernel_is_initialized NOTIFY kernelInitializedChanged)
  Q_PROPERTY(bool kernelIsSofa MEMBER kernelIsSofa NOTIFY kernelIsSofaChanged)

  Q_PROPERTY(int kernelChannels MEMBER kernelChannels NOTIFY kernelChannelsChanged)
  Q_PROPERTY(QString kernelRate MEMBER kernelRate NOTIFY kernelRateChanged)
  Q_PROPERTY(QString kernelSamples MEMBER kernelSamples NOTIFY kernelSamplesChanged)
  Q_PROPERTY(QString kernelDuration MEMBER kernelDuration NOTIFY kernelDurationChanged)

  Q_PROPERTY(QString sofaDatabase MEMBER sofaDatabase NOTIFY sofaDatabaseChanged)
  Q_PROPERTY(float sofaMeasurements MEMBER sofaMeasurements NOTIFY sofaMeasurementsChanged)
  Q_PROPERTY(float sofaIndex MEMBER sofaIndex NOTIFY sofaIndexChanged)
  Q_PROPERTY(float sofaAzimuth MEMBER sofaAzimuth NOTIFY sofaAzimuthChanged)
  Q_PROPERTY(float sofaElevation MEMBER sofaElevation NOTIFY sofaElevationChanged)
  Q_PROPERTY(float sofaRadius MEMBER sofaRadius NOTIFY sofaRadiusChanged)
  Q_PROPERTY(float sofaMinAzimuth MEMBER sofaMinAzimuth NOTIFY sofaMinAzimuthChanged)
  Q_PROPERTY(float sofaMaxAzimuth MEMBER sofaMaxAzimuth NOTIFY sofaMaxAzimuthChanged)
  Q_PROPERTY(float sofaMinElevation MEMBER sofaMinElevation NOTIFY sofaMinElevationChanged)
  Q_PROPERTY(float sofaMaxElevation MEMBER sofaMaxElevation NOTIFY sofaMaxElevationChanged)
  Q_PROPERTY(float sofaMinRadius MEMBER sofaMinRadius NOTIFY sofaMinRadiusChanged)
  Q_PROPERTY(float sofaMaxRadius MEMBER sofaMaxRadius NOTIFY sofaMaxRadiusChanged)

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

  void clear_data() override;

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

  Q_INVOKABLE void applySofaOrientation();

 Q_SIGNALS:
  void newKernelLoaded(QString name, bool success);

  void kernelInitializedChanged();
  void kernelIsSofaChanged();
  void kernelRateChanged();
  void kernelSamplesChanged();
  void kernelDurationChanged();
  void kernelChannelsChanged();

  void sofaDatabaseChanged();
  void sofaMeasurementsChanged();
  void sofaIndexChanged();
  void sofaAzimuthChanged();
  void sofaElevationChanged();
  void sofaRadiusChanged();
  void sofaMinAzimuthChanged();
  void sofaMaxAzimuthChanged();
  void sofaMinElevationChanged();
  void sofaMaxElevationChanged();
  void sofaMinRadiusChanged();
  void sofaMaxRadiusChanged();

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
  bool kernelIsSofa = false;
  bool n_samples_is_power_of_2 = true;
  bool ready = false;
  bool destructor_called = false;
  bool notify_latency = false;

  uint blocksize = 512U;
  uint latency_n_frames = 0U;

  int interpPoints = 1000;

  float dry = 0.0F, wet = 1.0F;

  QString sofaDatabase;
  int sofaMeasurements = 1;
  int sofaIndex = 0;
  float sofaAzimuth = 0.0F;
  float sofaElevation = 0.0F;
  float sofaRadius = 0.0F;
  float sofaMinAzimuth = 0.0F;
  float sofaMaxAzimuth = 0.0F;
  float sofaMinElevation = 0.0F;
  float sofaMaxElevation = 0.0F;
  float sofaMinRadius = 0.0F;
  float sofaMaxRadius = 0.0F;

  int kernelChannels = 2;
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

  void load_kernel_file(const bool& init_zita, const uint& server_sampling_rate);

  void combine_kernels(const std::string& kernel_1_name,
                       const std::string& kernel_2_name,
                       const std::string& output_file_name);

  void clear_chart_data();
};
