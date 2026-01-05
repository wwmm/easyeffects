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

#include "global_shortcuts.hpp"
#include <qcontainerfwd.h>
#include <qdbusextratypes.h>
#include <qdbuspendingcall.h>
#include <qdbuspendingreply.h>
#include <qlocalsocket.h>
#include <qlogging.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDBusReply>
#include <format>
#include <utility>
#include "easyeffects_db.h"
#include "easyeffects_db_streaminputs.h"
#include "util.hpp"

// Based on https://github.com/SourceReviver/qt_wayland_globalshortcut_via_portal/blob/main/wayland_shortcut.cpp
// Documentation: https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.GlobalShortcuts.html

GlobalShortcuts::GlobalShortcuts(QObject* parent) : QObject(parent) {
  qDBusRegisterMetaType<std::pair<QString, QVariantMap>>();
  qDBusRegisterMetaType<QList<QPair<QString, QVariantMap>>>();

  QMap<QString, QVariant> options;
  options["handle_token"] = QString("easyeffects%1").arg(util::random_string(32));
  options["session_handle_token"] = session_handle_token;

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
    util::warning(
        std::format("D-Bus CreateSession for GlobalShortcuts was denied or failed. Response code: {}", responseCode));

    return;
  }

  if (!results.contains("session_handle")) {
    util::warning("Missing session_handle on GlobalShortcuts CreateSession response.");

    return;
  }

  session_obj_path = QDBusObjectPath(results.value("session_handle").value<QString>());

  QDBusConnection::sessionBus().disconnect("org.freedesktop.portal.Desktop", response_handle.path(),
                                           "org.freedesktop.portal.Request", "Response", this,
                                           SLOT(onSessionCreatedResponse(uint, QVariantMap)));

  util::debug("D-Bus session for GlobalShortcuts created.");

  Q_EMIT onBindShortcuts();

  QDBusConnection::sessionBus().connect(
      "org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", "org.freedesktop.portal.GlobalShortcuts",
      "Activated", this, SLOT(process_activated_signal(QDBusObjectPath, QString, qulonglong, QVariantMap)));
}

void GlobalShortcuts::process_activated_signal([[maybe_unused]] const QDBusObjectPath& session_handle,
                                               const QString& shortcut_id,
                                               [[maybe_unused]] qulonglong timestamp,
                                               [[maybe_unused]] const QVariantMap& options) {
  // qDebug() << "Got GlobalShortcuts Activated Signal ->" << session_handle.path() << shortcut_id << timestamp <<
  // options;

  if (shortcut_id == "global bypass") {
    DbMain::setBypass(!DbMain::bypass());

    return;
  }

  if (shortcut_id == "microphone monitoring") {
    DbStreamInputs::setListenToMic(!DbStreamInputs::listenToMic());

    return;
  }
}

void GlobalShortcuts::bind_shortcuts() {
  // For security reasons, it's better to show the session handle only in development/debug mode.
  // util::info("Session handle object response:" + session_obj_path.path().toStdString());

  // a(sa{sv})
  QList<QPair<QString, QVariantMap>> shortcuts;

  for (const auto& gsd : ee_global_shortcuts_array) {
    QPair<QString, QVariantMap> shortcut;

    QVariantMap shortcut_options;
    shortcut.first = gsd.shortcut_id;
    shortcut_options.insert("description", gsd.shortcut_id);
    shortcut_options.insert("preferred_trigger", gsd.preferred_trigger);
    shortcut.second = shortcut_options;

    shortcuts.append(shortcut);
  }

  QMap<QString, QVariant> bind_opts;

  bind_opts.insert("handle_token", QString("easyeffects%1").arg(util::random_string(32)));

  QList<QVariant> bind_shortcut_args;

  /**
   * 1. session handle object
   * 2. shortcuts list
   * 3. window identifier (https://flatpak.github.io/xdg-desktop-portal/docs/window-identifiers.html)
   * 4. options (contains request handle token)
   */
  bind_shortcut_args.append(session_obj_path);
  bind_shortcut_args.append(QVariant::fromValue(shortcuts));
  bind_shortcut_args.append(QString());  // can be empty
  bind_shortcut_args.append(bind_opts);

  QDBusMessage bind_shortcut =
      QDBusMessage::createMethodCall("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop",
                                     "org.freedesktop.portal.GlobalShortcuts", "BindShortcuts");

  bind_shortcut.setArguments(bind_shortcut_args);

  // qDebug() << "input of bind->" << bind_shortcut.arguments();

  QDBusMessage bind_ret = QDBusConnection::sessionBus().call(bind_shortcut);

  // qDebug() << "GlobalShortcuts BindShortcuts response ->" << bind_ret;

  if (bind_ret.type() == QDBusMessage::ErrorMessage) {
    util::warning(std::format("Failed to bind shortcuts without session: {}", bind_ret.errorMessage().toStdString()));

    DbMain::setXdgGlobalShortcutsBound(false);
  } else {
    util::debug("The bind global shortcuts call returned without errors");

    DbMain::setXdgGlobalShortcutsBound(true);
  }
}
