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

#include "db_manager.hpp"
#include <kconfigskeleton.h>
#include <qapplication.h>
#include <qqml.h>
#include <qstandardpaths.h>
#include <qtmetamacros.h>
#include <qvariant.h>
#include <QMap>
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
#include "easyeffects_db_crusher.h"
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
#include "easyeffects_db_voice_suppressor.h"
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
      ensureExists(makeKey(tags::plugin_name::BaseName::autogain, id),
                   [&] { return new db::Autogain(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::bassEnhancer)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::bassEnhancer, id),
                   [&] { return new db::BassEnhancer(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::bassLoudness)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::bassLoudness, id),
                   [&] { return new db::BassLoudness(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::compressor)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::compressor, id),
                   [&] { return new db::Compressor(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::convolver)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::convolver, id),
                   [&] { return new db::Convolver(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::crossfeed)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::crossfeed, id),
                   [&] { return new db::Crossfeed(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::crusher)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::crusher, id), [&] { return new db::Crusher(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::crystalizer)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::crystalizer, id),
                   [&] { return new db::Crystalizer(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::delay)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::delay, id), [&] { return new db::Delay(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::deepfilternet)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::deepfilternet, id),
                   [&] { return new db::DeepFilterNet(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::deesser)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::deesser, id), [&] { return new db::Deesser(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::echoCanceller)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::echoCanceller, id),
                   [&] { return new db::EchoCanceller(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::equalizer)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::equalizer, id),
                   [&] { return new db::Equalizer(parentGroup, id); });
      ensureExists(makeKey(tags::plugin_name::BaseName::equalizer, id, "left"),
                   [&] { return new db::EqualizerChannel(parentGroup, id, "left"); });
      ensureExists(makeKey(tags::plugin_name::BaseName::equalizer, id, "right"),
                   [&] { return new db::EqualizerChannel(parentGroup, id, "right"); });

    } else if (name.startsWith(tags::plugin_name::BaseName::exciter)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::exciter, id), [&] { return new db::Exciter(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::expander)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::expander, id),
                   [&] { return new db::Expander(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::filter)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::filter, id), [&] { return new db::Filter(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::gate)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::gate, id), [&] { return new db::Gate(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::voiceSuppressor)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::voiceSuppressor, id),
                   [&] { return new db::VoiceSuppressor(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::levelMeter)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::levelMeter, id),
                   [&] { return new db::LevelMeter(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::limiter)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::limiter, id), [&] { return new db::Limiter(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::loudness)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::loudness, id),
                   [&] { return new db::Loudness(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::maximizer)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::maximizer, id),
                   [&] { return new db::Maximizer(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::multibandCompressor)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::multibandCompressor, id),
                   [&] { return new db::MultibandCompressor(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::multibandGate)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::multibandGate, id),
                   [&] { return new db::MultibandGate(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::pitch)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::pitch, id), [&] { return new db::Pitch(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::reverb)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::reverb, id), [&] { return new db::Reverb(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::rnnoise)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::rnnoise, id), [&] { return new db::RNNoise(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::speex)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::speex, id), [&] { return new db::Speex(parentGroup, id); });

    } else if (name.startsWith(tags::plugin_name::BaseName::stereoTools)) {
      ensureExists(makeKey(tags::plugin_name::BaseName::stereoTools, id),
                   [&] { return new db::StereoTools(parentGroup, id); });
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
