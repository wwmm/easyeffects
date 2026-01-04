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

#include <kconfigskeleton.h>
#include <pipewire/proxy.h>
#include <qlist.h>
#include <qobject.h>
#include <qpoint.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <QString>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "output_level.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "spectrum.hpp"

class EffectsBaseWorker : public QObject {
  Q_OBJECT
};

class EffectsBase : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool filtersLinked MEMBER filtersLinked NOTIFY filtersLinkedChanged)

 public:
  EffectsBase(pw::Manager* pipe_manager, PipelineType pipe_type);
  EffectsBase(const EffectsBase&) = delete;
  auto operator=(const EffectsBase&) -> EffectsBase& = delete;
  EffectsBase(const EffectsBase&&) = delete;
  auto operator=(const EffectsBase&&) -> EffectsBase& = delete;
  ~EffectsBase() override;

  const std::string log_tag;

  pw::Manager* pm = nullptr;

  PipelineType pipeline_type;

  std::shared_ptr<OutputLevel> output_level;
  std::shared_ptr<Spectrum> spectrum;

  auto get_plugins_map() -> std::map<QString, std::shared_ptr<PluginBase>>;

  Q_INVOKABLE QVariant getPluginInstance(const QString& pluginName);

  Q_INVOKABLE [[nodiscard]] uint getPipeLineRate() const;

  Q_INVOKABLE [[nodiscard]] uint getPipeLineLatency();

  Q_INVOKABLE [[nodiscard]] float getOutputLevelLeft() const;

  Q_INVOKABLE [[nodiscard]] float getOutputLevelRight() const;

  Q_INVOKABLE void requestSpectrumData();

  Q_INVOKABLE void setUpdateLevelMeters(const bool& state);

  Q_INVOKABLE void setSpectrumBypass(const bool& state);

 Q_SIGNALS:
  void pipelineChanged();
  void newSpectrumData(QList<QPointF> newData);
  void filtersLinkedChanged();

 protected:
  bool filtersLinked = false;

  std::map<QString, std::shared_ptr<PluginBase>> plugins;

  std::vector<pw_proxy*> list_proxies, list_proxies_listen_mic;

  EffectsBaseWorker* baseWorker;

  QThread workerThread;

  void create_filters_if_necessary();

  void remove_unused_filters();

  void activate_filters();

  void deactivate_filters();
};
