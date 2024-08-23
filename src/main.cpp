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

#include <kaboutdata.h>
#include <klocalizedcontext.h>
#include <qobject.h>
#include <qqml.h>
#include <qqmlapplicationengine.h>
#include <qqmlcontext.h>
#include <qquickstyle.h>
#include <qstandardpaths.h>
#include <qstringliteral.h>
#include <qtenvironmentvariables.h>
#include <qurl.h>
#include <KAboutData>
#include <KLocalizedString>
#include <QApplication>
#include <QLocalServer>
#include <QSystemTrayIcon>
#include <cstdlib>
#include <cstring>
#include <memory>
#include "config.h"
#include "easyeffects_db.h"
#include "easyeffects_db_spectrum.h"
#include "easyeffects_db_streaminputs.h"
#include "easyeffects_db_streamoutputs.h"
#include "local_client.hpp"
#include "local_server.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

void construct_about_window() {
  KAboutData aboutData(QStringLiteral(COMPONENT_NAME), i18nc("@title", APPLICATION_NAME),
                       QStringLiteral(PROJECT_VERSION), i18n("Audio effects for PipeWire applications"),
                       KAboutLicense::GPL_V3, i18n("(c) 2024"), QStringLiteral(""),
                       QStringLiteral("https://github.com/wwmm/easyeffects"),
                       QStringLiteral("https://github.com/wwmm/easyeffects/issues"));

  aboutData.addAuthor(i18nc("@info:credit", "Wellington Wallace"), i18nc("@info:credit", "Developer"),
                      QStringLiteral("wellingtonwallace@gmail.com"));

  // Set aboutData as information about the app
  KAboutData::setApplicationData(aboutData);

  qmlRegisterSingletonType("AboutEE",  // How the import statement should look like
                           VERSION_MAJOR, VERSION_MINOR, "AboutEE", [](QQmlEngine* engine, QJSEngine*) -> QJSValue {
                             return engine->toScriptValue(KAboutData::applicationData());
                           });
}

int main(int argc, char* argv[]) {
  bool can_use_sys_tray = true;
  auto local_server = std::make_unique<LocalServer>();
  auto lockFile = util::get_lock_file();

  if (!lockFile->isLocked()) {
    auto local_client = std::make_unique<LocalClient>();

    local_client->show_main_window();
    local_client->show_version();

    return 0;
  }

  QApplication app(argc, argv);

  QApplication::setQuitOnLastWindowClosed(false);

  local_server->startServer();  // it has to be done after "QApplication app(argc, argv)"

  QObject::connect(local_server.get(), &LocalServer::onOpenWindow,
                   [&]() { util::debug("another instance as ked to open Window"); });

  KLocalizedString::setApplicationDomain(APPLICATION_DOMAIN);
  QCoreApplication::setOrganizationName(QStringLiteral(ORGANIZATION_NAME));
  QCoreApplication::setOrganizationDomain(QStringLiteral(ORGANIZATION_DOMAIN));
  QCoreApplication::setApplicationName(QStringLiteral("EasyEffects"));

  if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
  }

  construct_about_window();

  // creating our database directory
  {
    auto db_dir_path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).append("/easyeffects/db");
    util::create_user_directory(db_dir_path.toStdString());
  }

  // Verifying if we can use the system tray

  can_use_sys_tray = QSystemTrayIcon::isSystemTrayAvailable();

  // Registering kcfg settings

  auto ee_db = db::Main::self();
  auto ee_db_spectrum = db::Spectrum::self();
  auto ee_db_streamoutputs = db::StreamOutputs::self();
  auto ee_db_streaminputs = db::StreamInputs::self();

  // QObject::connect(ee_db, &db::Main::widthChanged, [=]() { util::warning(util::to_string(db::Main::width())); });

  // Making sure these singleton classes are initialized before qml
  tags::plugin_name::Model::self();
  pw::Manager::self();

  // QObject::connect(&pw::Manager::self(), &pw::Manager::sink_changed, [](auto info) { util::warning(info.name); });

  QQmlApplicationEngine engine;

  engine.rootContext()->setContextProperty("EEdb", ee_db);
  engine.rootContext()->setContextProperty("EEdbSpectrum", ee_db_spectrum);
  engine.rootContext()->setContextProperty("EEdbStreamOutputs", ee_db_streamoutputs);
  engine.rootContext()->setContextProperty("EEdbStreamInputs", ee_db_streaminputs);
  engine.rootContext()->setContextProperty("canUseSysTray", can_use_sys_tray);
  engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
  engine.load(QUrl(QStringLiteral("qrc:/ui/main.qml")));

  if (engine.rootObjects().isEmpty()) {
    return -1;
  }

  QObject::connect(&app, &QApplication::aboutToQuit, [=]() {
    ee_db->save();
    ee_db_spectrum->save();
    ee_db_streamoutputs->save();
    ee_db_streaminputs->save();
  });

  return QApplication::exec();
}