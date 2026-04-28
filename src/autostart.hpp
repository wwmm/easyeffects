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

#include <qqmlengine.h>
#include <qqmlintegration.h>
#include <qtmetamacros.h>
#include <QObject>
#include <QWindow>

class Autostart : public QObject {
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON
  QML_UNCREATABLE("Use the c++ instance")

 public:
  explicit Autostart(QObject* parent = nullptr);

  /**
   * Deleting the default constructor because we want Qt to call our custom create method.
   * If this is not done qml will create its own class instance.
   */
  Autostart() = delete;

  Autostart(const Autostart&) = delete;
  Autostart(Autostart&&) = delete;
  Autostart& operator=(const Autostart&) = delete;
  Autostart& operator=(Autostart&&) = delete;

  inline static Autostart* singletonInstance = nullptr;

  // Singleton provider for QML
  static Autostart* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    Q_UNUSED(jsEngine)

    // The instance has to exist before it is used. We cannot replace it.
    Q_ASSERT(singletonInstance);

    // The engine has to have the same thread affinity as the singleton.

    Q_ASSERT(qmlEngine->thread() == singletonInstance->thread());

    // Explicitly specify C++ ownership so that the engine doesn't delete the instance.

    QJSEngine::setObjectOwnership(singletonInstance, QJSEngine::CppOwnership);

    return singletonInstance;
  }

  void set_window(QWindow* window);

 Q_SIGNALS:
  void error(const QString& msg);

 private:
  QWindow* window = nullptr;

#ifdef ENABLE_LIBPORTAL
  void update_background_portal();
#endif

  static void fallback_enable_autostart(const bool& state);

  void update_state();
};
