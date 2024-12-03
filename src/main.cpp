/*
 *  Copyright © 2017-2024 Wellington Wallace
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
#include <QSystemTrayIcon>
#include <QWindow>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <memory>
#include "command_line_parser.hpp"
#include "config.h"
#include "db_manager.hpp"
#include "local_client.hpp"
#include "local_server.hpp"
#include "presets_manager.hpp"
#include "pw_manager.hpp"
#include "stream_input_effects.hpp"
#include "stream_output_effects.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

void csignalHandler(int s) {
  std::signal(s, SIG_DFL);
  qApp->quit();
}

int main(int argc, char* argv[]) {
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
    auto local_client = std::make_unique<LocalClient>();

    QObject::connect(cmd_parser.get(), &CommandLineParser::onQuit, [&]() {
      local_client->quit_app();
      show_window = false;
    });

    QObject::connect(cmd_parser.get(), &CommandLineParser::onHideWindow, [&]() {
      local_client->hide_window();
      show_window = false;
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

  if (show_window) {
    engine.load(QUrl(QStringLiteral("qrc:/ui/main.qml")));
  }

  if (engine.rootObjects().isEmpty()) {
    return -1;
  }

  QObject::connect(&app, &QApplication::aboutToQuit, [&]() { dbm->saveAll(); });

  return QApplication::exec();
}