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
#include <qobject.h>
#include <qqmlengine.h>
#include <qqmlintegration.h>
#include <qtmetamacros.h>
#include <qtpreprocessorsupport.h>
#include <KColorSchemeManager>

class KColorManager : public QObject {
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON
  QML_UNCREATABLE("C++ singleton - use KColorManager.instance")

  Q_PROPERTY(QAbstractItemModel* model MEMBER model CONSTANT)

 public:
  KColorManager();
  KColorManager(const KColorManager&) = delete;
  auto operator=(const KColorManager&) -> KColorManager& = delete;
  KColorManager(const KColorManager&&) = delete;
  auto operator=(const KColorManager&&) -> KColorManager& = delete;
  ~KColorManager() override = default;

  static KColorManager& self() {
    static KColorManager m;
    return m;
  }

  // Singleton provider for QML
  static KColorManager* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    Q_UNUSED(jsEngine)

    // The engine has to have the same thread affinity as the singleton.

    Q_ASSERT(qmlEngine->thread() == self().thread());

    // Explicitly specify C++ ownership so that the engine doesn't delete the instance.

    QJSEngine::setObjectOwnership(&self(), QJSEngine::CppOwnership);

    return &self();
  }

  Q_INVOKABLE void activateScheme(const int& index);

  Q_INVOKABLE int activeScheme();

 private:
  KColorSchemeManager* manager = nullptr;

  QAbstractItemModel* model = nullptr;
};
