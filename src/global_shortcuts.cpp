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

#include "global_shortcuts.hpp"
#include <qcontainerfwd.h>
#include <qdbusextratypes.h>
#include <qdbuspendingcall.h>
#include <qdbuspendingreply.h>
#include <qlocalsocket.h>
#include <qlogging.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qtypes.h>
#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDBusReply>
#include <utility>
#include "util.hpp"

// Based on https://github.com/SourceReviver/qt_wayland_globalshortcut_via_portal/blob/main/wayland_shortcut.cpp

GlobalShortcuts::GlobalShortcuts(QObject* parent)
    : QObject(parent),
      iface("org.freedesktop.portal.Desktop",
            "/org/freedesktop/portal/desktop",
            "org.freedesktop.portal.GlobalShortcuts",
            QDBusConnection::sessionBus()) {
  qDBusRegisterMetaType<std::pair<QString, QVariantMap>>();
  qDBusRegisterMetaType<QList<QPair<QString, QVariantMap>>>();

  QMap<QString, QVariant> options;
  options["handle_token"] = handle_token;
  options["session_handle_token"] = handle_token;

  if (!iface.isValid()) {
    qWarning() << "GlobalShortcuts interface not available";
  }

  auto message = iface.call("CreateSession", options);

  auto request_path = message.arguments().first().value<QDBusObjectPath>();

  QDBusConnection::sessionBus().connect("org.freedesktop.portal.Desktop", request_path.path(),
                                        "org.freedesktop.portal.Request", "Response", this,
                                        SLOT(onSessionCreatedResponse(uint, QVariantMap)));
}

void GlobalShortcuts::onSessionCreatedResponse(uint responseCode, const QVariantMap& results) {
  if (responseCode != 0) {
    util::warning("Session creation was denied or failed. Response code: " + util::to_string(responseCode));
    return;
  }

  QDBusObjectPath sessionPath(results.value("session_handle").value<QString>());

  util::info("Session created: " + sessionPath.path().toStdString());

  // a(sa{sv})
  QList<QPair<QString, QVariantMap>> shortcuts;

  QPair<QString, QVariantMap> shortcut;
  QVariantMap shortcut_options;
  shortcut.first = "global_bypass";
  shortcut_options.insert("description", "global bypass");
  shortcut_options.insert("preferred_trigger", "CTRL+ALT+E");
  shortcut.second = shortcut_options;

  shortcuts.append(shortcut);

  QVariantMap bindOptions;

  QDBusPendingCall bindCall =
      iface.asyncCall("BindShortcuts", sessionPath, QVariant::fromValue(shortcuts), "", bindOptions);

  auto* bindWatcher = new QDBusPendingCallWatcher(bindCall);

  QObject::connect(bindWatcher, &QDBusPendingCallWatcher::finished, [](QDBusPendingCallWatcher* bindCall) {
    if (bindCall->isError()) {
      qWarning() << "BindShortcutsToSession failed:" << bindCall->error().message();
    } else {
      qDebug() << "Shortcuts bound to session successfully.";
    }
    bindCall->deleteLater();
  });
}
