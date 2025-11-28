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

#include "db_manager.hpp"
#include <kconfigskeleton.h>
#include <qapplication.h>
#include <qqml.h>
#include <qstandardpaths.h>
#include <qtmetamacros.h>
#include <qvariant.h>
#include <QMap>
#include <QSet>
#include <QString>
#include <QTimer>
#include "config.h"
#include "easyeffects_db.h"
#include "easyeffects_db_autogain.h"
#include "easyeffects_db_bass_enhancer.h"
#include "easyeffects_db_bass_loudness.h"
#include "easyeffects_db_compressor.h"
#include "easyeffects_db_convolver.h"
#include "easyeffects_db_crossfeed.h"
#include "easyeffects_db_crystalizer.h"
#include "easyeffects_db_deepfilternet.h"
#include "easyeffects_db_deesser.h"
#include "easyeffects_db_delay.h"
#include "easyeffects_db_echo_canceller.h"
#include "easyeffects_db_equalizer.h"
#include "easyeffects_db_equalizer_channel.h"
#include "easyeffects_db_exciter.h"
#include "easyeffects_db_expander.h"
#include "easyeffects_db_filter.h"
#include "easyeffects_db_gate.h"
#include "easyeffects_db_level_meter.h"
#include "easyeffects_db_limiter.h"
#include "easyeffects_db_loudness.h"
#include "easyeffects_db_maximizer.h"
#include "easyeffects_db_multiband_compressor.h"
#include "easyeffects_db_multiband_gate.h"
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
    : graph(DbGraph::self()),
      main(DbMain::self()),
      spectrum(DbSpectrum::self()),
      streamInputs(DbStreamInputs::self()),
      streamOutputs(DbStreamOutputs::self()),
      testSignals(DbTestSignals::self()),
      timer(new QTimer(this)) {
  // creating our database directory if it does not exist

  auto db_dir_path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).append("/easyeffects/db");

  util::create_user_directory(db_dir_path.toStdString());

  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete)
  qmlRegisterSingletonInstance<db::Manager>("ee.database", VERSION_MAJOR, VERSION_MINOR, "Manager", this);

  QApplication::setQuitOnLastWindowClosed(!DbMain::enableServiceMode());

  // autosave timer

  timer->setInterval(DbMain::databaseAutosaveInterval());

  // creating plugins database

  create_plugin_db("sie", DbStreamInputs::plugins(), siePluginsDB);
  create_plugin_db("soe", DbStreamOutputs::plugins(), soePluginsDB);

  // signals

  connect(main, &DbMain::enableServiceModeChanged,
          []() { QApplication::setQuitOnLastWindowClosed(!DbMain::enableServiceMode()); });

  connect(streamInputs, &DbStreamInputs::pluginsChanged,
          [&]() { create_plugin_db("sie", DbStreamInputs::plugins(), siePluginsDB); });

  connect(streamOutputs, &DbStreamOutputs::pluginsChanged,
          [&]() { create_plugin_db("soe", DbStreamOutputs::plugins(), soePluginsDB); });

  connect(main, &DbMain::databaseAutosaveIntervalChanged,
          [&]() { timer->setInterval(DbMain::databaseAutosaveInterval()); });

  connect(timer, &QTimer::timeout, [&]() { saveAll(); });
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

  graph->save();
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

  graph->setDefaults();
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
  QSet<QString> active_keys;

  auto makeKey = [&](const QString& base, const QString& id, const QString& suffix = QString()) {
    return suffix.isEmpty() ? base + "#" + id : base + "#" + id + "#" + suffix;
  };

  auto ensureExists = [&](const QString& key, auto factory) {
    if (!plugins_map.contains(key)) {
      plugins_map[key] = QVariant::fromValue(factory());
    }
  };

  for (const auto& name : plugins_list) {
    auto id = tags::plugin_name::get_id(name);

    if (name.startsWith(tags::plugin_name::BaseName::autogain)) {
      const auto key = makeKey(tags::plugin_name::BaseName::autogain, id);
      ensureExists(key, [&] { return new db::Autogain(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::bassEnhancer)) {
      const auto key = makeKey(tags::plugin_name::BaseName::bassEnhancer, id);
      ensureExists(key, [&] { return new db::BassEnhancer(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::bassLoudness)) {
      const auto key = makeKey(tags::plugin_name::BaseName::bassLoudness, id);
      ensureExists(key, [&] { return new db::BassLoudness(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::compressor)) {
      const auto key = makeKey(tags::plugin_name::BaseName::compressor, id);
      ensureExists(key, [&] { return new db::Compressor(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::convolver)) {
      const auto key = makeKey(tags::plugin_name::BaseName::convolver, id);
      ensureExists(key, [&] { return new db::Convolver(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::crossfeed)) {
      const auto key = makeKey(tags::plugin_name::BaseName::crossfeed, id);
      ensureExists(key, [&] { return new db::Crossfeed(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::crystalizer)) {
      const auto key = makeKey(tags::plugin_name::BaseName::crystalizer, id);
      ensureExists(key, [&] { return new db::Crystalizer(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::delay)) {
      const auto key = makeKey(tags::plugin_name::BaseName::delay, id);
      ensureExists(key, [&] { return new db::Delay(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::deepfilternet)) {
      const auto key = makeKey(tags::plugin_name::BaseName::deepfilternet, id);
      ensureExists(key, [&] { return new db::DeepFilterNet(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::deesser)) {
      const auto key = makeKey(tags::plugin_name::BaseName::deesser, id);
      ensureExists(key, [&] { return new db::Deesser(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::echoCanceller)) {
      const auto key = makeKey(tags::plugin_name::BaseName::echoCanceller, id);
      ensureExists(key, [&] { return new db::EchoCanceller(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::equalizer)) {
      const auto base_key = makeKey(tags::plugin_name::BaseName::equalizer, id);
      const auto left_key = makeKey(tags::plugin_name::BaseName::equalizer, id, "left");
      const auto right_key = makeKey(tags::plugin_name::BaseName::equalizer, id, "right");

      ensureExists(base_key, [&] { return new db::Equalizer(parentGroup, id); });
      ensureExists(left_key, [&] { return new db::EqualizerChannel(parentGroup, id, "left"); });
      ensureExists(right_key, [&] { return new db::EqualizerChannel(parentGroup, id, "right"); });

      active_keys.insert(base_key);
      active_keys.insert(left_key);
      active_keys.insert(right_key);

    } else if (name.startsWith(tags::plugin_name::BaseName::exciter)) {
      const auto key = makeKey(tags::plugin_name::BaseName::exciter, id);
      ensureExists(key, [&] { return new db::Exciter(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::expander)) {
      const auto key = makeKey(tags::plugin_name::BaseName::expander, id);
      ensureExists(key, [&] { return new db::Expander(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::filter)) {
      const auto key = makeKey(tags::plugin_name::BaseName::filter, id);
      ensureExists(key, [&] { return new db::Filter(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::gate)) {
      const auto key = makeKey(tags::plugin_name::BaseName::gate, id);
      ensureExists(key, [&] { return new db::Gate(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::levelMeter)) {
      const auto key = makeKey(tags::plugin_name::BaseName::levelMeter, id);
      ensureExists(key, [&] { return new db::LevelMeter(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::limiter)) {
      const auto key = makeKey(tags::plugin_name::BaseName::limiter, id);
      ensureExists(key, [&] { return new db::Limiter(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::loudness)) {
      const auto key = makeKey(tags::plugin_name::BaseName::loudness, id);
      ensureExists(key, [&] { return new db::Loudness(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::maximizer)) {
      const auto key = makeKey(tags::plugin_name::BaseName::maximizer, id);
      ensureExists(key, [&] { return new db::Maximizer(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::multibandCompressor)) {
      const auto key = makeKey(tags::plugin_name::BaseName::multibandCompressor, id);
      ensureExists(key, [&] { return new db::MultibandCompressor(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::multibandGate)) {
      const auto key = makeKey(tags::plugin_name::BaseName::multibandGate, id);
      ensureExists(key, [&] { return new db::MultibandGate(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::pitch)) {
      const auto key = makeKey(tags::plugin_name::BaseName::pitch, id);
      ensureExists(key, [&] { return new db::Pitch(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::reverb)) {
      const auto key = makeKey(tags::plugin_name::BaseName::reverb, id);
      ensureExists(key, [&] { return new db::Reverb(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::rnnoise)) {
      const auto key = makeKey(tags::plugin_name::BaseName::rnnoise, id);
      ensureExists(key, [&] { return new db::RNNoise(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::speex)) {
      const auto key = makeKey(tags::plugin_name::BaseName::speex, id);
      ensureExists(key, [&] { return new db::Speex(parentGroup, id); });
      active_keys.insert(key);

    } else if (name.startsWith(tags::plugin_name::BaseName::stereoTools)) {
      const auto key = makeKey(tags::plugin_name::BaseName::stereoTools, id);
      ensureExists(key, [&] { return new db::StereoTools(parentGroup, id); });
      active_keys.insert(key);
    }
  }

  for (auto it = plugins_map.begin(); it != plugins_map.end();) {
    if (!active_keys.contains(it.key())) {
      auto value = it.value();
      if (value.canConvert<KConfigSkeleton*>()) {
        delete value.value<KConfigSkeleton*>();
      }

      it = plugins_map.erase(it);
    } else {
      ++it;
    }
  }

  if (parentGroup == "sie") {
    Q_EMIT siePluginsDBChanged();
  } else {
    Q_EMIT soePluginsDBChanged();
  }
}

void Manager::enableAutosave(const bool& state) {
  if (state) {
    timer->start();
  } else {
    timer->stop();
  }
}

}  // namespace db
