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
#include "easyeffects_db_bass_loudness.h"
#include "easyeffects_db_compressor.h"
#include "easyeffects_db_convolver.h"
#include "easyeffects_db_crossfeed.h"
#include "easyeffects_db_crystalizer.h"
#include "easyeffects_db_deesser.h"
#include "easyeffects_db_delay.h"
#include "easyeffects_db_echo_canceller.h"
#include "easyeffects_db_exciter.h"
#include "easyeffects_db_expander.h"
#include "easyeffects_db_filter.h"
#include "easyeffects_db_gate.h"
#include "easyeffects_db_level_meter.h"
#include "easyeffects_db_limiter.h"
#include "easyeffects_db_loudness.h"
#include "easyeffects_db_maximizer.h"
#include "easyeffects_db_pitch.h"
#include "easyeffects_db_reverb.h"
#include "easyeffects_db_rnnoise.h"
#include "easyeffects_db_spectrum.h"
#include "easyeffects_db_speex.h"
#include "easyeffects_db_stereo_tools.h"
#include "easyeffects_db_streaminputs.h"
#include "easyeffects_db_streamoutputs.h"
#include "tags_plugin_name.hpp"
#include "util.hpp"

namespace db {

Manager::Manager()
    : main(db::Main::self()),
      spectrum(db::Spectrum::self()),
      streamInputs(db::StreamInputs::self()),
      streamOutputs(db::StreamOutputs::self()),
      testSignals(db::TestSignals::self()) {
  // creating our database directory if it does not exist

  auto db_dir_path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).append("/easyeffects/db");

  util::create_user_directory(db_dir_path.toStdString());

  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete)
  qmlRegisterSingletonInstance<db::Manager>("ee.database", VERSION_MAJOR, VERSION_MINOR, "Manager", this);

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
  testSignals->save();

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
  testSignals->setDefaults();

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
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::bassEnhancer)) {
        plugins_map[tags::plugin_name::BaseName::bassEnhancer + "#" + id] =
            QVariant::fromValue(new db::BassEnhancer(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::bassLoudness)) {
        plugins_map[tags::plugin_name::BaseName::bassLoudness + "#" + id] =
            QVariant::fromValue(new db::BassLoudness(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::compressor)) {
        plugins_map[tags::plugin_name::BaseName::compressor + "#" + id] =
            QVariant::fromValue(new db::Compressor(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::convolver)) {
        plugins_map[tags::plugin_name::BaseName::convolver + "#" + id] =
            QVariant::fromValue(new db::Convolver(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::crossfeed)) {
        plugins_map[tags::plugin_name::BaseName::crossfeed + "#" + id] =
            QVariant::fromValue(new db::Crossfeed(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::crystalizer)) {
        plugins_map[tags::plugin_name::BaseName::crystalizer + "#" + id] =
            QVariant::fromValue(new db::Crystalizer(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::delay)) {
        plugins_map[tags::plugin_name::BaseName::delay + "#" + id] =
            QVariant::fromValue(new db::Delay(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::deesser)) {
        plugins_map[tags::plugin_name::BaseName::deesser + "#" + id] =
            QVariant::fromValue(new db::Deesser(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::echoCanceller)) {
        plugins_map[tags::plugin_name::BaseName::echoCanceller + "#" + id] =
            QVariant::fromValue(new db::EchoCanceller(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::exciter)) {
        plugins_map[tags::plugin_name::BaseName::exciter + "#" + id] =
            QVariant::fromValue(new db::Exciter(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::expander)) {
        plugins_map[tags::plugin_name::BaseName::expander + "#" + id] =
            QVariant::fromValue(new db::Expander(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::filter)) {
        plugins_map[tags::plugin_name::BaseName::filter + "#" + id] =
            QVariant::fromValue(new db::Filter(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::gate)) {
        plugins_map[tags::plugin_name::BaseName::gate + "#" + id] = QVariant::fromValue(new db::Gate(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::levelMeter)) {
        plugins_map[tags::plugin_name::BaseName::levelMeter + "#" + id] =
            QVariant::fromValue(new db::LevelMeter(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::limiter)) {
        plugins_map[tags::plugin_name::BaseName::limiter + "#" + id] =
            QVariant::fromValue(new db::Limiter(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::loudness)) {
        plugins_map[tags::plugin_name::BaseName::loudness + "#" + id] =
            QVariant::fromValue(new db::Loudness(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::maximizer)) {
        plugins_map[tags::plugin_name::BaseName::maximizer + "#" + id] =
            QVariant::fromValue(new db::Maximizer(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::pitch)) {
        plugins_map[tags::plugin_name::BaseName::pitch + "#" + id] =
            QVariant::fromValue(new db::Pitch(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::reverb)) {
        plugins_map[tags::plugin_name::BaseName::reverb + "#" + id] =
            QVariant::fromValue(new db::Reverb(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::rnnoise)) {
        plugins_map[tags::plugin_name::BaseName::rnnoise + "#" + id] =
            QVariant::fromValue(new db::RNNoise(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::speex)) {
        plugins_map[tags::plugin_name::BaseName::speex + "#" + id] =
            QVariant::fromValue(new db::Speex(parentGroup, id));
        //
      } else if (name.startsWith(tags::plugin_name::BaseName::stereoTools)) {
        plugins_map[tags::plugin_name::BaseName::stereoTools + "#" + id] =
            QVariant::fromValue(new db::StereoTools(parentGroup, id));
        //
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
