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

#include <qassert.h>
#include <qjsengine.h>
#include <qmap.h>
#include <qobject.h>
#include <qqmlengine.h>
#include <qqmlintegration.h>
#include <qtmetamacros.h>
#include <qtpreprocessorsupport.h>
#include <QTimer>
#include "easyeffects_db.h"                // IWYU pragma: export
#include "easyeffects_db_graph.h"          // IWYU pragma: export
#include "easyeffects_db_spectrum.h"       // IWYU pragma: export
#include "easyeffects_db_streaminputs.h"   // IWYU pragma: export
#include "easyeffects_db_streamoutputs.h"  // IWYU pragma: export
#include "easyeffects_db_test_signals.h"   // IWYU pragma: export
#include "pipeline_type.hpp"

namespace db {

class Manager : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(DatabaseManager)
  QML_SINGLETON
  QML_UNCREATABLE("C++ singleton - use DatabaseManager.instance")

  Q_PROPERTY(QMap<QString, QVariant> soePluginsDB MEMBER soePluginsDB NOTIFY soePluginsDBChanged)
  Q_PROPERTY(QMap<QString, QVariant> siePluginsDB MEMBER siePluginsDB NOTIFY siePluginsDBChanged)

 public:
  Manager();
  Manager(const Manager&) = delete;
  auto operator=(const Manager&) -> Manager& = delete;
  Manager(const Manager&&) = delete;
  auto operator=(const Manager&&) -> Manager& = delete;
  ~Manager() override;

  static Manager& self() {
    static Manager m;
    return m;
  }

  // Singleton provider for QML
  static Manager* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    Q_UNUSED(jsEngine)

    // The engine has to have the same thread affinity as the singleton.

    Q_ASSERT(qmlEngine->thread() == self().thread());

    // Explicitly specify C++ ownership so that the engine doesn't delete the instance.

    QJSEngine::setObjectOwnership(&self(), QJSEngine::CppOwnership);

    return &self();
  }

  Q_INVOKABLE void saveAll() const;

  Q_INVOKABLE void resetAll() const;

  Q_INVOKABLE void enableAutosave(const bool& state);

  DbGraph* graph;
  DbMain* main;
  DbSpectrum* spectrum;
  DbStreamInputs* streamInputs;
  DbStreamOutputs* streamOutputs;
  DbTestSignals* testSignals;

  QMap<QString, QVariant> soePluginsDB;
  QMap<QString, QVariant> siePluginsDB;

  template <typename T>
  auto get_plugin_db(PipelineType pipeline_type, const QString& plugin_name) -> T* {
    switch (pipeline_type) {
      case PipelineType::input:
        return siePluginsDB.contains(plugin_name) ? siePluginsDB[plugin_name].value<T*>() : nullptr;
      case PipelineType::output:
        return soePluginsDB.contains(plugin_name) ? soePluginsDB[plugin_name].value<T*>() : nullptr;
    }

    return nullptr;
  }

 Q_SIGNALS:
  void soePluginsDBChanged();
  void siePluginsDBChanged();

 private:
  QTimer* timer = nullptr;

  void create_plugin_db(const QString& parentGroup, const auto& plugins_list, QMap<QString, QVariant>& plugins_map);
};

}  // namespace db
