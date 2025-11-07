/**
 * Copyright © 2017-2025 Wellington Wallace
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

#include <kicontheme.h>
#include <klocalizedcontext.h>
#include <qcontainerfwd.h>
#include <qnamespace.h>
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
#include <QDir>
#include <QLocalServer>
#include <QLoggingCategory>
#include <QProcessEnvironment>
#include <QQuickWindow>
#include <QSystemTrayIcon>
#include <csignal>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
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

#if defined(__GLIBC__)
#include <malloc.h>
#endif

class SignalHandler {
 public:
  SignalHandler() {
    std::signal(SIGINT, handle);
    std::signal(SIGTERM, handle);
  }

 private:
  static void handle(int s) {
    std::signal(s, SIG_DFL);

    QCoreApplication::quit();
  }
};

struct CoreServices {
  db::Manager* dbm = nullptr;
  pw::Manager* pwm = nullptr;

  std::unique_ptr<StreamInputEffects> sie;
  std::unique_ptr<StreamOutputEffects> soe;

  CoreServices(bool is_primary) {
    if (is_primary) {
      extra_lv2_paths();

      dbm = &db::Manager::self();
      pwm = &pw::Manager::self();

      sie = std::make_unique<StreamInputEffects>(pwm);
      soe = std::make_unique<StreamOutputEffects>(pwm);

      TestSignals::self(pwm);
      tags::plugin_name::Model::self();
      presets::Manager::self();
    }
  }

  static void extra_lv2_paths() {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    QString existing_path = env.value("LV2_PATH");

    QStringList extra_paths_list;

    extra_paths_list << "/usr/lib64/lv2/"
                     << "/usr/local/lib64/lv2"
                     << "/usr/lib/x86_64-linux-gnu/lv2/";

    if (!existing_path.isEmpty()) {
      extra_paths_list << existing_path;
    }

    QString extra_paths = extra_paths_list.join(":");

    if (qputenv("LV2_PATH", extra_paths.toLocal8Bit())) {
      util::debug(std::format("Extra LV2 search paths: {}", extra_paths.toStdString()));
    } else {
      util::warning("Failed to set extra LV2 search paths.");
    }
  }
};

struct UiState {
  QQuickWindow* window = nullptr;
};

static void initGlobalBypass(StreamInputEffects& sie, StreamOutputEffects& soe) {
  auto update_bypass_state = [&]() {
    soe.set_bypass(db::Main::bypass());
    sie.set_bypass(db::Main::bypass());

    util::info((db::Main::bypass() ? "Enabling global bypass" : "Disabling global bypass"));
  };

  update_bypass_state();

  QObject::connect(db::Main::self(), &db::Main::bypassChanged, update_bypass_state);
}

static void initGlobalShortcuts(GlobalShortcuts* shortcuts) {
  auto bind = [shortcuts]() {
    util::info("XDG Global Shortcuts experimental feature is enabled for this session.");

    const auto session = qEnvironmentVariable("XDG_SESSION_DESKTOP");
    const auto desktop = qEnvironmentVariable("XDG_CURRENT_DESKTOP");

    if (session == "KDE" || desktop == "KDE") {
      if (!db::Main::xdgGlobalShortcutsBound()) {
        shortcuts->bind_shortcuts();
      }
    } else {
      // Some desktops (gnome, hyprland) need binding always
      shortcuts->bind_shortcuts();
    }
  };

  QObject::connect(shortcuts, &GlobalShortcuts::onBindShortcuts, [bind]() {
    if (db::Main::xdgGlobalShortcuts()) {
      bind();
    }
  });

  QObject::connect(db::Main::self(), &db::Main::xdgGlobalShortcutsChanged, [bind]() {
    if (db::Main::xdgGlobalShortcuts()) {
      bind();
    } else {
      db::Main::setXdgGlobalShortcutsBound(false);
    }
  });
}

static void initQml(QQmlApplicationEngine& engine,
                    Autostart& autostart,
                    LocalServer& server,
                    UiState& ui,
                    bool show_window) {
  engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
  engine.rootContext()->setContextProperty("canUseSysTray", QSystemTrayIcon::isSystemTrayAvailable());
  engine.rootContext()->setContextProperty("projectVersion", PROJECT_VERSION);

  QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, [&](QObject* object, const QUrl& url) {
    if (url.toString() == "qrc:/ui/main.qml") {
      ui.window = qobject_cast<QQuickWindow*>(object);

      if (ui.window) {
        ui.window->setPersistentGraphics(false);
        ui.window->setPersistentSceneGraph(false);

        if (show_window) {
          ui.window->show();
          ui.window->raise();
          ui.window->requestActivate();
        } else {
          ui.window->hide();
        }

        autostart.set_window(ui.window);

        QObject::connect(ui.window, &QQuickWindow::visibleChanged, [&](bool visible) {
          if (!visible) {
            util::debug("Asking Qt to clear QML's engine cache");

            ui.window->releaseResources();

            engine.trimComponentCache();
            engine.collectGarbage();

#if defined(__GLIBC__)
            malloc_trim(0);
#endif
          }
        });
      }
    }
  });

  QObject::connect(&server, &LocalServer::onShowWindow, [&]() {
    if (ui.window) {
      ui.window->show();
      ui.window->raise();
      ui.window->requestActivate();
    }
  });

  QObject::connect(&server, &LocalServer::onHideWindow, [&]() {
    if (ui.window) {
      ui.window->hide();
    }
  });

  engine.load(QUrl(QStringLiteral("qrc:/ui/main.qml")));
  if (engine.rootObjects().isEmpty()) {
    throw std::runtime_error("Failed to load QML UI");
  }
}

static int runSecondaryInstance(QApplication& app, CommandLineParser& parser, bool& show_window) {
  auto local_client = std::make_unique<LocalClient>();

  QObject::connect(&parser, &CommandLineParser::onQuit, [&]() {
    local_client->quit_app();
    show_window = false;
  });

  QObject::connect(&parser, &CommandLineParser::onHideWindow, [&]() {
    local_client->hide_window();
    show_window = false;
  });

  QObject::connect(&parser, &CommandLineParser::onLoadPreset,
                   [&](PipelineType type, QString preset) { local_client->load_preset(type, preset.toStdString()); });

  parser.process(&app);

  if (show_window) {
    local_client->show_window();
  }

  return 0;
}

int main(int argc, char* argv[]) {
  QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
  QLoggingCategory::setFilterRules("easyeffects.debug=false");
  KIconTheme::initTheme();

  QApplication app(argc, argv);

  SignalHandler signalHandler;

  KLocalizedString::setApplicationDomain(APPLICATION_DOMAIN);
  QCoreApplication::setOrganizationDomain(QStringLiteral(ORGANIZATION_DOMAIN));
  QCoreApplication::setApplicationName(QStringLiteral(APPLICATION_DOMAIN));
  QCoreApplication::setApplicationVersion(QStringLiteral(PROJECT_VERSION));

  /**
   * QApplication specializes QGuiApplication and we need to use it to set
   * the application name and the desktop entry name in order to show the
   * correct icon in the title bar and desktop application menus.
   */
  QApplication::setApplicationName(APPLICATION_DOMAIN);
  QApplication::setApplicationDisplayName(APPLICATION_NAME);
  QApplication::setApplicationVersion(QStringLiteral(PROJECT_VERSION));
  QApplication::setDesktopFileName("com.github.wwmm.easyeffects");

  QApplication::setStyle(QStringLiteral("breeze"));
  if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
  }

  // Parsing command line options

  auto cmd_parser = std::make_unique<CommandLineParser>();

  QObject::connect(cmd_parser.get(), &CommandLineParser::onReset, [&]() { db::Manager::self().resetAll(); });

  // Checking if there is already an instance running

  auto lockFile = util::get_lock_file();

  bool show_window = true;

  if (!lockFile->isLocked()) {
    // Used only by an instance started when one is already running
    CoreServices core(false);

    return runSecondaryInstance(app, *cmd_parser, show_window);
  }

  QObject::connect(cmd_parser.get(), &CommandLineParser::onHideWindow, [&]() { show_window = false; });

  cmd_parser->process(&app);

  // Core managers
  CoreServices core(true);

  // Main instance services
  auto local_server = std::make_unique<LocalServer>();
  auto global_shortcuts = std::make_unique<GlobalShortcuts>();
  auto autostart = std::make_unique<Autostart>();

  UiState ui;

  QQmlApplicationEngine engine;

  // Starting the local socket server

  local_server->startServer();  // it has to be done after "QApplication app(argc, argv)"

  QObject::connect(local_server.get(), &LocalServer::onQuitApp, [&]() { QApplication::quit(); });

  initGlobalBypass(*core.sie, *core.soe);
  initGlobalShortcuts(global_shortcuts.get());
  initQml(engine, *autostart, *local_server, ui, show_window);

  QObject::connect(&app, &QApplication::aboutToQuit, [&]() { db::Manager::self().saveAll(); });

  return QApplication::exec();
}
