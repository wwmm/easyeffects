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

#include <kicontheme.h>
#include <klocalizedcontext.h>
#include <qcontainerfwd.h>
#include <qhashfunctions.h>
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
#include <KColorSchemeManager>
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
#include "iostream"
#include "kcolor_manager.hpp"
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

#ifdef __GLIBC__
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

    QStringList search_paths_list;

    search_paths_list << "/usr/lib64/lv2/"
                      << "/usr/local/lib64/lv2"
                      << "/usr/lib/lv2"  // for some reason ARM needs this set
                      << "/usr/local/lib/lv2"
                      << "/usr/lib/x86_64-linux-gnu/lv2/";

    if (!existing_path.isEmpty()) {
      search_paths_list.append(existing_path.split(':'));
    }

    QSet<QString> unique_paths;

    for (auto& p : search_paths_list) {
      QString clean = QDir(p).canonicalPath();

      if (!clean.isEmpty()) {
        unique_paths.insert(clean);
      }
    }

    QStringList final_paths = unique_paths.values();

    QString paths = final_paths.join(":");

    if (qputenv("LV2_PATH", paths.toLocal8Bit())) {
      util::debug(std::format("LV2 search paths: {}", paths.toStdString()));
    } else {
      util::warning("Failed to set LV2 search paths.");
    }
  }
};

struct UiState {
  QQuickWindow* window = nullptr;
};

static void initGlobalBypass(StreamInputEffects& sie, StreamOutputEffects& soe) {
  auto update_bypass_state = [&]() {
    soe.set_bypass(DbMain::bypass());
    sie.set_bypass(DbMain::bypass());

    util::debug((DbMain::bypass() ? "Enabling global bypass" : "Disabling global bypass"));
  };

  update_bypass_state();

  QObject::connect(DbMain::self(), &DbMain::bypassChanged, update_bypass_state);
}

static void initGlobalShortcuts(GlobalShortcuts* shortcuts) {
  auto bind = [shortcuts]() {
    util::debug("XDG Global Shortcuts experimental feature is enabled for this session.");

    const auto session = qEnvironmentVariable("XDG_SESSION_DESKTOP");
    const auto desktop = qEnvironmentVariable("XDG_CURRENT_DESKTOP");

    if (session == "KDE" || desktop == "KDE") {
      if (!DbMain::xdgGlobalShortcutsBound()) {
        shortcuts->bind_shortcuts();
      }
    } else {
      // Some desktops (gnome, hyprland) need binding always
      shortcuts->bind_shortcuts();
    }
  };

  QObject::connect(shortcuts, &GlobalShortcuts::onBindShortcuts, [bind]() {
    if (DbMain::xdgGlobalShortcuts()) {
      bind();
    }
  });

  QObject::connect(DbMain::self(), &DbMain::xdgGlobalShortcutsChanged, [bind]() {
    if (DbMain::xdgGlobalShortcuts()) {
      bind();
    } else {
      DbMain::setXdgGlobalShortcutsBound(false);
    }
  });
}

static void initQml(QQmlApplicationEngine& engine,
                    Autostart& autostart,
                    LocalServer& server,
                    UiState& ui,
                    bool& show_window) {
  engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
  engine.rootContext()->setContextProperty("canUseSysTray", QSystemTrayIcon::isSystemTrayAvailable());
  engine.rootContext()->setContextProperty("projectVersion", PROJECT_VERSION);
  engine.rootContext()->setContextProperty("applicationId", APPLICATION_ID);
  engine.rootContext()->setContextProperty("applicationName", APPLICATION_NAME);

  QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, [&](QObject* object, const QUrl& url) {
    if (url.toString() == "qrc:/qt/qml/ee/ui/contents/ui/Main.qml") {
      ui.window = qobject_cast<QQuickWindow*>(object);

      if (ui.window) {
        ui.window->setPersistentGraphics(false);
        ui.window->setPersistentSceneGraph(false);

        if (show_window && !DbMain::noWindowAfterStarting()) {
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

#ifdef __GLIBC__
            malloc_trim(0);
#endif
          }
        });
      }
    }
  });

  QObject::connect(&server, &LocalServer::onShowWindow, [&]() {
    show_window = true;

    if (ui.window) {
      ui.window->show();
      ui.window->raise();
      ui.window->requestActivate();
    }
  });

  QObject::connect(&server, &LocalServer::onHideWindow, [&]() {
    show_window = false;

    if (ui.window) {
      ui.window->hide();
    }
  });

  engine.loadFromModule("ee.ui", "Main");

  if (engine.rootObjects().isEmpty()) {
    throw std::runtime_error("Failed to load QML UI");
  }
}

static int runSecondaryInstance(KAboutData& about, QApplication& app, CommandLineParser& parser, bool& show_window) {
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

  QObject::connect(&parser, &CommandLineParser::onSetGlobalBypass, [&](const bool& state) {
    local_client->setGlobalBypass(state);
    show_window = false;
  });

  QObject::connect(&parser, &CommandLineParser::onGetLastLoadedPreset, [&](PipelineType type) {
    auto preset = local_client->getLastLoadedPreset(type);
    std::cout << preset.toStdString() << '\n';
    show_window = false;
  });

  QObject::connect(&parser, &CommandLineParser::onGetLastLoadedInputOutputPreset, [&]() {
    auto input_preset = local_client->getLastLoadedPreset(PipelineType::input);
    auto output_preset = local_client->getLastLoadedPreset(PipelineType::output);

    std::cout << "input: " << input_preset.toStdString() << '\n';
    std::cout << "output: " << output_preset.toStdString() << '\n';

    show_window = false;
  });

  parser.set_is_primary(false);
  parser.process(about, &app);
  parser.process_events();

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

  KAboutData about(QStringLiteral(APPLICATION_DOMAIN), QStringLiteral(APPLICATION_NAME),
                   QStringLiteral(PROJECT_VERSION), i18n("Global audio effects"), KAboutLicense::GPL_V3,
                   i18n("© 2017-2026 EasyEffects Team"));

  about.addAuthor(i18n("Wellington Wallace"), i18nc("@info:credit", "Developer"),
                  QStringLiteral("wellingtonwallace@gmail.com"));

  about.setOrganizationDomain(ORGANIZATION_DOMAIN);
  about.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));
  about.setBugAddress("https://github.com/wwmm/easyeffects/issues");
  about.setHomepage("https://github.com/wwmm/easyeffects");
  about.setDesktopFileName(APPLICATION_ID);
  about.setProgramLogo(APPLICATION_ID);

  KAboutData::setApplicationData(about);

  QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral(APPLICATION_ID)));

  KColorSchemeManager::instance();

  // Parsing command line options

  auto cmd_parser = std::make_unique<CommandLineParser>(about);

  QObject::connect(cmd_parser.get(), &CommandLineParser::onReset, [&]() { db::Manager::self().resetAll(); });

  // Checking if there is already an instance running

  auto lockFile = util::get_lock_file();

  bool show_window = true;

  if (!lockFile->isLocked()) {
    // Used only by an instance started when one is already running

    return runSecondaryInstance(about, app, *cmd_parser, show_window);
  }

  cmd_parser->process(about, &app);
  cmd_parser->process_debug_option();  // if we take too long to process this one we will miss debug messages
  cmd_parser->process_hide_window(show_window);

  UiState ui;

  // Core managers
  CoreServices core(true);

  // Main instance services
  auto local_server = std::make_unique<LocalServer>();
  auto global_shortcuts = std::make_unique<GlobalShortcuts>();
  auto autostart = std::make_unique<Autostart>();
  auto color_manager = std::make_unique<KColorManager>();

  // theme initialization

  if (DbMain::forceBreezeTheme()) {
    QApplication::setStyle(QStringLiteral("breeze"));
  }

  if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
  }

  QQmlApplicationEngine engine;

  // Starting the local socket server

  local_server->startServer();  // it has to be done after "QApplication app(argc, argv)"

  QObject::connect(local_server.get(), &LocalServer::onQuitApp, [&]() { QApplication::quit(); });

  initGlobalBypass(*core.sie, *core.soe);
  initGlobalShortcuts(global_shortcuts.get());

  QObject::connect(&app, &QApplication::aboutToQuit, [&]() { db::Manager::self().saveAll(); });

  QObject::connect(cmd_parser.get(), &CommandLineParser::onHideWindow, [&]() {
    show_window = false;

    if (ui.window) {
      ui.window->hide();
    }
  });

  QObject::connect(cmd_parser.get(), &CommandLineParser::onInitQML,
                   [&]() { initQml(engine, *autostart, *local_server, ui, show_window); });

  cmd_parser->process_events();

  return QApplication::exec();
}
