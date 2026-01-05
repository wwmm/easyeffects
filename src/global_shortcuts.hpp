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

 public:
  explicit GlobalShortcuts(QObject* parent = nullptr);

  void bind_shortcuts();

  std::array<GlobalShortcutData, 2> ee_global_shortcuts_array = {
      {{.shortcut_id = "global bypass", .description = i18n("Toggle global bypass"), .preferred_trigger = "CTRL+ALT+E"},
       {.shortcut_id = "microphone monitoring",
        .description = i18n("Toggle microphone monitoring"),
        .preferred_trigger = "CTRL+ALT+I"}},
  };

 Q_SIGNALS:
  void onBindShortcuts();

 public Q_SLOTS:
  void onSessionCreatedResponse(uint responseCode, const QVariantMap& results);

  static void process_activated_signal(const QDBusObjectPath& session_handle,
                                       const QString& shortcut_id,
                                       qulonglong timestamp,
                                       const QVariantMap& options);

 private:
  const QString session_handle_token = QString("easyeffects%1").arg(util::random_string(32));

  QDBusObjectPath response_handle, session_obj_path;
};
