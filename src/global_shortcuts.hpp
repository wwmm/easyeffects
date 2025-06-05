/*
 *  Copyright © 2017-2025 Wellington Wallace
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <qcontainerfwd.h>
#include <qdbusextratypes.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <QApplication>
#include <QDBusInterface>
#include <QObject>
#include "util.hpp"

struct GlobalShortcutData {
  QString shortcut_id;
  QString description;
  QString preferred_trigger;
};

class GlobalShortcuts : public QObject {
  Q_OBJECT

 public:
  std::array<GlobalShortcutData, 1> ee_global_shortcuts_array = {
      {"toggle_global_bypass", "Toggle Global Bypass", "CTRL+ALT+E"}};

  explicit GlobalShortcuts(QObject* parent = nullptr);

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
