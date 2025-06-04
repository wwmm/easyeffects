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

GlobalShortcuts::GlobalShortcuts(QObject* parent) : QObject(parent) {
  qDBusRegisterMetaType<std::pair<QString, QVariantMap>>();
  qDBusRegisterMetaType<QList<QPair<QString, QVariantMap>>>();

  QMap<QString, QVariant> options;
  options["handle_token"] = handle_token;
  options["session_handle_token"] = handle_token;

  QList<QVariant> args_create_session;

  args_create_session.append(options);

  QDBusMessage create_session =
      QDBusMessage::createMethodCall("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop",
                                     "org.freedesktop.portal.GlobalShortcuts", "CreateSession");

  create_session.setArguments(args_create_session);

  auto message = QDBusConnection::sessionBus().call(create_session);

  response_handle = message.arguments().first().value<QDBusObjectPath>();

  QDBusConnection::sessionBus().connect("org.freedesktop.portal.Desktop", response_handle.path(),
                                        "org.freedesktop.portal.Request", "Response", this,
                                        SLOT(onSessionCreatedResponse(uint, QVariantMap)));
}

void GlobalShortcuts::onSessionCreatedResponse(uint responseCode, const QVariantMap& results) {
  if (responseCode != 0) {
    util::warning("Session creation was denied or failed. Response code: " + util::to_string(responseCode));
    return;
  }

  if (results.contains("session_handle")) {
    session_obj_path = QDBusObjectPath(results.value("session_handle").value<QString>());
  }

  QDBusConnection::sessionBus().disconnect("org.freedesktop.portal.Desktop", response_handle.path(),
                                           "org.freedesktop.portal.Request", "Response", this,
                                           SLOT(onSessionCreatedResponse(uint, QVariantMap)));

  util::info("Session created: " + session_obj_path.path().toStdString());

  // a(sa{sv})
  QList<QPair<QString, QVariantMap>> shortcuts;

  QPair<QString, QVariantMap> shortcut;
  QVariantMap shortcut_options;
  shortcut.first = "global_bypass";
  shortcut_options.insert("description", "global bypass");
  shortcut_options.insert("preferred_trigger", "CTRL+ALT+E");
  shortcut.second = shortcut_options;

  shortcuts.append(shortcut);

  QMap<QString, QVariant> bind_opts;

  bind_opts.insert("handle_token", handle_token);

  QList<QVariant> bind_shortcut_args;

  bind_shortcut_args.append(session_obj_path);
  bind_shortcut_args.append(QVariant::fromValue(shortcuts));
  bind_shortcut_args.append(QString());
  bind_shortcut_args.append(bind_opts);

  QDBusMessage bind_shortcut =
      QDBusMessage::createMethodCall("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop",
                                     "org.freedesktop.portal.GlobalShortcuts", "BindShortcuts");

  bind_shortcut.setArguments(bind_shortcut_args);

  // qDebug() << "input of bind->" << bind_shortcut.arguments();

  QDBusMessage bind_ret = QDBusConnection::sessionBus().call(bind_shortcut);

  qDebug() << "bind message return->" << bind_ret;

  QDBusConnection::sessionBus().connect(
      "org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", "org.freedesktop.portal.GlobalShortcuts",
      "Activated", this, SLOT(process_activated_signal(QDBusObjectPath, QString, qulonglong, QVariantMap)));
}

void GlobalShortcuts::process_activated_signal(const QDBusObjectPath& session_handle,
                                               const QString& shortcut_id,
                                               qulonglong timestamp,
                                               const QVariantMap& options) {
  qDebug() << "Got Signal ->" << session_handle.path() << shortcut_id << timestamp << options;
}
