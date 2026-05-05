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

#include <qcontainerfwd.h>
#include <qdbusextratypes.h>
#include <qqmlengine.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <KLocalizedString>
#include <QApplication>
#include <QDBusInterface>
#include <QObject>
#include <array>
#include "util.hpp"

struct GlobalShortcutData {
  QString shortcut_id;
  QString description;
  QString preferred_trigger;
};

class GlobalShortcuts : public QObject {
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON
  QML_UNCREATABLE("Use the c++ instance")

 public:
  explicit GlobalShortcuts(QObject* parent = nullptr);

  /**
   * Deleting the default constructor because we want Qt to call our custom create method.
   * If this is not done qml will create its own class instance.
   */
  GlobalShortcuts() = delete;

  GlobalShortcuts(const GlobalShortcuts&) = delete;
  GlobalShortcuts(GlobalShortcuts&&) = delete;
  GlobalShortcuts& operator=(const GlobalShortcuts&) = delete;
  GlobalShortcuts& operator=(GlobalShortcuts&&) = delete;

  inline static GlobalShortcuts* singletonInstance = nullptr;

  // Singleton provider for QML
  static GlobalShortcuts* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    Q_UNUSED(jsEngine)

    // The instance has to exist before it is used. We cannot replace it.
    Q_ASSERT(singletonInstance);

    // The engine has to have the same thread affinity as the singleton.

    Q_ASSERT(qmlEngine->thread() == singletonInstance->thread());

    // Explicitly specify C++ ownership so that the engine doesn't delete the instance.

    QJSEngine::setObjectOwnership(singletonInstance, QJSEngine::CppOwnership);

    return singletonInstance;
  }

  void bind_shortcuts();

  std::array<GlobalShortcutData, 2> ee_global_shortcuts_array = {
      {{.shortcut_id = "global bypass", .description = i18n("Toggle global bypass"), .preferred_trigger = "CTRL+ALT+E"},
       {.shortcut_id = "microphone monitoring",
        .description = i18n("Toggle microphone monitoring"),
        .preferred_trigger = "CTRL+ALT+I"}},
  };

 Q_SIGNALS:
  void bindShortcuts();
  void globalBypassToggled();

 public Q_SLOTS:
  void onSessionCreatedResponse(uint responseCode, const QVariantMap& results);

  void process_activated_signal(const QDBusObjectPath& session_handle,
                                const QString& shortcut_id,
                                qulonglong timestamp,
                                const QVariantMap& options);

 private:
  const QString session_handle_token = QString("easyeffects%1").arg(util::random_string(32));

  QDBusObjectPath response_handle, session_obj_path;
};
