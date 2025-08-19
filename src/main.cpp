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

#include <kicontheme.h>
#include <klocalizedcontext.h>
#include <qcontainerfwd.h>
#include <qobject.h>
#include <qqml.h>
#include <qqmlapplicationengine.h>
#include <qqmlcontext.h>
#include <qquickstyle.h>
#include <qstringliteral.h>
#include <qtenvironmentvariables.h>
#include <qurl.h>
#include <KAboutData>
#include <KIconTheme>
#include <KLocalizedString>
#include <QApplication>
#include <QLocalServer>
#include <QLoggingCategory>
#include <QSystemTrayIcon>
#include <QWindow>
#include <csignal>
#include <memory>
#include "autostart.hpp"
#include "command_line_parser.hpp"
#include "config.h"
#include "db_manager.hpp"
#include "global_shortcuts.hpp"
#include "local_client.hpp"
#include "local_server.hpp"
#include "pipeline_type.hpp"
#include "presets_manager.hpp"
#include "pw_manager.hpp"
#include "stream_input_effects.hpp"
#include "stream_output_effects.hpp"
#include "tags_plugin_name.hpp"
#include "test_signals.hpp"
#include "util.hpp"

static void csignalHandler(int s) {
  std::signal(s, SIG_DFL);
  qApp->quit();
}

int main(int argc, char* argv[]) {
  QLoggingCategory::setFilterRules("easyeffects.debug=false");

  KIconTheme::initTheme();

  QApplication app(argc, argv);

  std::signal(SIGINT, csignalHandler);
  std::signal(SIGTERM, csignalHandler);

  bool show_window = true;

  KLocalizedString::setApplicationDomain(APPLICATION_DOMAIN);
  QCoreApplication::setOrganizationDomain(QStringLiteral(ORGANIZATION_DOMAIN));
  QCoreApplication::setApplicationName(QStringLiteral(APPLICATION_DOMAIN));
  QCoreApplication::setApplicationVersion(QStringLiteral(PROJECT_VERSION));

  QApplication::setStyle(QStringLiteral("breeze"));
  if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
  }

  // loading our database

  auto* dbm = &db::Manager::self();

  // Parsing command line options

  auto cmd_parser = std::make_unique<CommandLineParser>();

  QObject::connect(cmd_parser.get(), &CommandLineParser::onReset, [&]() { dbm->resetAll(); });

  // Checking if there is already an instance running

  auto lockFile = util::get_lock_file();

  if (!lockFile->isLocked()) {
    // If we do not have the lock we are probably a secondary instance that will try to communicate with the service

    auto local_client = std::make_unique<LocalClient>();

    QObject::connect(cmd_parser.get(), &CommandLineParser::onQuit, [&]() {
      local_client->quit_app();
      show_window = false;
    });

    QObject::connect(cmd_parser.get(), &CommandLineParser::onHideWindow, [&]() {
      local_client->hide_window();
      show_window = false;
    });

    QObject::connect(cmd_parser.get(), &CommandLineParser::onLoadPreset,
                     [&](PipelineType pipeline_type, QString preset_name) {
                       local_client->load_preset(pipeline_type, preset_name.toStdString());
                     });

    cmd_parser->process(&app);

    if (show_window) {
      local_client->show_window();
    }

    return 0;
  }

  QObject::connect(cmd_parser.get(), &CommandLineParser::onHideWindow, [&]() { show_window = false; });

  cmd_parser->process(&app);

  // Starting the local socket server

  auto local_server = std::make_unique<LocalServer>();

  local_server->startServer();  // it has to be done after "QApplication app(argc, argv)"

  QObject::connect(local_server.get(), &LocalServer::onQuitApp, [&]() { QApplication::quit(); });

  // Making sure these singleton classes are initialized before qml

  tags::plugin_name::Model::self();

  presets::Manager::self();

  auto* pm = &pw::Manager::self();

  auto sie = std::make_unique<StreamInputEffects>(pm);
  auto soe = std::make_unique<StreamOutputEffects>(pm);

  TestSignals::self(pm);

  {
    pw::Manager::exclude_monitor_stream = db::Main::excludeMonitorStreams();

    QObject::connect(db::Main::self(), &db::Main::excludeMonitorStreamsChanged,
                     []() { pw::Manager::exclude_monitor_stream = db::Main::excludeMonitorStreams(); });
  }

  // initializing the global bypass
  {
    auto update_bypass_state = [&]() {
      soe->set_bypass(db::Main::bypass());
      sie->set_bypass(db::Main::bypass());

      util::info((db::Main::bypass() ? "enabling global bypass" : "disabling global bypass"));
    };

    update_bypass_state();

    QObject::connect(db::Main::self(), &db::Main::bypassChanged, update_bypass_state);
  }

  // Global shortcuts

  std::unique_ptr<GlobalShortcuts> global_shortcuts = std::make_unique<GlobalShortcuts>();

  auto bind_global_shortcuts = [&]() {
    util::info("XDG Global Shortcuts experimental feature is enabled for this session.");

    if (qEnvironmentVariable("XDG_SESSION_DESKTOP") == "KDE" || qEnvironmentVariable("XDG_CURRENT_DESKTOP") == "KDE") {
      // Do the binding call only if really necessary
      if (!db::Main::xdgGlobalShortcutsBound()) {
        global_shortcuts->bind_shortcuts();
      }
    } else {
      // Some desktops like gnome and hyprland seem to need the call to always be made
      // https://github.com/wwmm/easyeffects/issues/3834#issuecomment-2940756992
      // https://github.com/wwmm/easyeffects/issues/3834#issuecomment-2941713432
      global_shortcuts->bind_shortcuts();
    }
  };

  QObject::connect(global_shortcuts.get(), &GlobalShortcuts::onBindShortcuts, [&]() {
    if (db::Main::xdgGlobalShortcuts()) {
      bind_global_shortcuts();
    }
  });

  QObject::connect(db::Main::self(), &db::Main::xdgGlobalShortcutsChanged, [&]() {
    if (db::Main::xdgGlobalShortcuts()) {
      bind_global_shortcuts();
    } else {
      db::Main::setXdgGlobalShortcutsBound(false);
    }
  });

  // autostart

  auto autostart = std::make_unique<Autostart>();

  // Initializing QML

  QQmlApplicationEngine engine;

  engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
  engine.rootContext()->setContextProperty("canUseSysTray", QSystemTrayIcon::isSystemTrayAvailable());
  engine.rootContext()->setContextProperty("projectVersion", PROJECT_VERSION);

  QWindow* window = nullptr;

  QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, [&](QObject* object, const QUrl& url) {
    if (url.toString() == "qrc:/ui/main.qml") {
      window = qobject_cast<QWindow*>(object);

      window->show();
      window->raise();
      window->requestActivate();
    }
  });

  QObject::connect(local_server.get(), &LocalServer::onShowWindow, [&]() {
    if (window != nullptr) {
      window->show();
      window->raise();
      window->requestActivate();
    }
  });

  QObject::connect(local_server.get(), &LocalServer::onHideWindow, [&]() {
    if (window != nullptr) {
      window->hide();
    }
  });

  if (show_window) {
    engine.load(QUrl(QStringLiteral("qrc:/ui/main.qml")));
  }

  if (engine.rootObjects().isEmpty()) {
    return -1;
  }

  QObject::connect(&app, &QApplication::aboutToQuit, [&]() { dbm->saveAll(); });

  return QApplication::exec();
}
