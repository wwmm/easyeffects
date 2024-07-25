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
#include <memory>
#include "config.h"
#include "easyeffects_db.h"
#include "easyeffects_db_spectrum.h"
#include "easyeffects_db_streaminputs.h"
#include "easyeffects_db_streamoutputs.h"
#include "pipe_manager.hpp"
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
  auto lockFile = util::get_lock_file();

  if (!lockFile->isLocked()) {
    return -1;
  }

  QApplication app(argc, argv);

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

  // Registering kcfg settings

  auto ee_db = db::Main::self();
  auto ee_db_spectrum = db::Spectrum::self();
  auto ee_db_streamoutputs = db::StreamOutputs::self();
  auto ee_db_streaminputs = db::StreamInputs::self();

  // QObject::connect(ee_db, &db::Main::widthChanged, [=]() { util::warning(util::to_string(db::Main::width())); });

  // Making sure these singleton classes are initialized before qml
  tags::plugin_name::Model::self();
  pw::Manager::self();

  QQmlApplicationEngine engine;

  engine.rootContext()->setContextProperty("EEdb", ee_db);
  engine.rootContext()->setContextProperty("EEdbSpectrum", ee_db_spectrum);
  engine.rootContext()->setContextProperty("EEdbStreamOutputs", ee_db_streamoutputs);
  engine.rootContext()->setContextProperty("EEdbStreamInputs", ee_db_streaminputs);
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