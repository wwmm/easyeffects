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
#include <qapplication.h>
#include <qqml.h>
#include <qstandardpaths.h>
#include "config.h"
#include "easyeffects_db.h"
#include "easyeffects_db_spectrum.h"
#include "easyeffects_db_streaminputs.h"
#include "easyeffects_db_streamoutputs.h"
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

  // service mdoe configuration

  QApplication::setQuitOnLastWindowClosed(!db::Main::enableServiceMode());

  connect(main, &db::Main::enableServiceModeChanged,
          []() { QApplication::setQuitOnLastWindowClosed(!db::Main::enableServiceMode()); });
}

Manager::~Manager() {
  saveAll();
}

void Manager::saveAll() const {
  util::debug("Saving settings...");

  main->save();
  spectrum->save();
  streamOutputs->save();
  streamInputs->save();
}

void Manager::resetAll() const {
  util::debug("Resetting settings...");

  main->setDefaults();
  spectrum->setDefaults();
  streamOutputs->setDefaults();
  streamInputs->setDefaults();
}

}  // namespace db