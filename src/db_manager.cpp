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

#include "db_manager.hpp"
#include <kconfigskeleton.h>
#include <qapplication.h>
#include <qqml.h>
#include <qstandardpaths.h>
#include <qtmetamacros.h>
#include <qvariant.h>
#include <QMap>
#include <QString>
#include "config.h"
#include "easyeffects_db.h"
#include "easyeffects_db_autogain.h"
#include "easyeffects_db_bass_enhancer.h"
#include "easyeffects_db_spectrum.h"
#include "easyeffects_db_streaminputs.h"
#include "easyeffects_db_streamoutputs.h"
#include "tags_plugin_name.hpp"
#include "util.hpp"

namespace db {

Manager::Manager()
    : main(db::Main::self()),
      spectrum(db::Spectrum::self()),
      streamInputs(db::StreamInputs::self()),
      streamOutputs(db::StreamOutputs::self()) {
  // creating our database directory if it does not exist

  auto db_dir_path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).append("/easyeffects/db");

  util::create_user_directory(db_dir_path.toStdString());

  qmlRegisterSingletonInstance<db::Manager>("EEdbm", VERSION_MAJOR, VERSION_MINOR, "EEdbm", this);

  QApplication::setQuitOnLastWindowClosed(!db::Main::enableServiceMode());

  // creating plugins database

  create_plugin_db("sie", db::StreamInputs::plugins(), siePluginsDB);
  create_plugin_db("soe", db::StreamOutputs::plugins(), soePluginsDB);

  // signals

  connect(main, &db::Main::enableServiceModeChanged,
          []() { QApplication::setQuitOnLastWindowClosed(!db::Main::enableServiceMode()); });

  connect(streamInputs, &db::StreamInputs::pluginsChanged,
          [&]() { create_plugin_db("sie", db::StreamInputs::plugins(), siePluginsDB); });

  connect(streamOutputs, &db::StreamOutputs::pluginsChanged,
          [&]() { create_plugin_db("soe", db::StreamOutputs::plugins(), soePluginsDB); });
}

Manager::~Manager() {
  saveAll();

  for (auto& v : soePluginsDB) {
    delete v.value<KConfigSkeleton*>();
  }

  for (auto& v : siePluginsDB) {
    delete v.value<KConfigSkeleton*>();
  }
}

void Manager::saveAll() const {
  util::debug("Saving settings...");

  main->save();
  spectrum->save();
  streamOutputs->save();
  streamInputs->save();

  for (const auto& plugin_db : siePluginsDB.values()) {
    plugin_db.value<KConfigSkeleton*>()->save();
  }

  for (const auto& plugin_db : soePluginsDB.values()) {
    plugin_db.value<KConfigSkeleton*>()->save();
  }
}

void Manager::resetAll() const {
  util::debug("Resetting settings...");

  main->setDefaults();
  spectrum->setDefaults();
  streamOutputs->setDefaults();
  streamInputs->setDefaults();

  for (const auto& plugin_db : siePluginsDB.values()) {
    plugin_db.value<KConfigSkeleton*>()->setDefaults();
  }

  for (const auto& plugin_db : soePluginsDB.values()) {
    plugin_db.value<KConfigSkeleton*>()->setDefaults();
  }
}

void Manager::create_plugin_db(const QString& parentGroup,
                               const auto& plugins_list,
                               QMap<QString, QVariant>& plugins_map) {
  for (const auto& name : plugins_list) {
    if (!plugins_map.contains(name)) {
      auto id = tags::plugin_name::get_id(name);

      if (name.startsWith(tags::plugin_name::BaseName::autogain)) {
        plugins_map[tags::plugin_name::BaseName::autogain + "#" + id] =
            QVariant::fromValue(new db::Autogain(parentGroup, id));
      } else if (name.startsWith(tags::plugin_name::BaseName::bassEnhancer)) {
        plugins_map[tags::plugin_name::BaseName::bassEnhancer + "#" + id] =
            QVariant::fromValue(new db::BassEnhancer(parentGroup, id));
      }
    }
  }

  if (parentGroup == "sie") {
    Q_EMIT siePluginsDBChanged();
  } else {
    Q_EMIT soePluginsDBChanged();
  }
}

}  // namespace db