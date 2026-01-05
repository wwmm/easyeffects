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

#pragma once

#include <qabstractitemmodel.h>
#include <qhash.h>
#include <qhashfunctions.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qqmlintegration.h>
#include <qstringliteral.h>
#include <qstringview.h>
#include <qtmetamacros.h>
#include <qvariant.h>
#include <sys/types.h>

// NOLINTNEXTLINE
#define CREATE_PROPERTY(type, name, value) \
  static inline const type name = value;   \
  Q_PROPERTY(type name MEMBER name CONSTANT)

namespace tags::plugin_package {

class Package : public QObject {
  Q_OBJECT

 public:
  CREATE_PROPERTY(QString, bs2b, QStringLiteral("bs2b"));
  CREATE_PROPERTY(QString, calf, QStringLiteral("Calf Studio Gear"));
  CREATE_PROPERTY(QString, deepfilternet, QStringLiteral("DeepFilterNet"));
  CREATE_PROPERTY(QString, ebur128, QStringLiteral("libebur128"));
  CREATE_PROPERTY(QString, ee, QStringLiteral("Easy Effects"));
  CREATE_PROPERTY(QString, lsp, QStringLiteral("Linux Studio Plugins"));
  CREATE_PROPERTY(QString, mda, QStringLiteral("MDA"));
  CREATE_PROPERTY(QString, rnnoise, QStringLiteral("RNNoise"));
  CREATE_PROPERTY(QString, soundTouch, QStringLiteral("SoundTouch"));
  CREATE_PROPERTY(QString, speex, QStringLiteral("SpeexDSP"));
  CREATE_PROPERTY(QString, webrtc, QStringLiteral("WebRTC"));
  CREATE_PROPERTY(QString, zam, QStringLiteral("ZamAudio"));
  CREATE_PROPERTY(QString, zita, QStringLiteral("Zita"));
};

}  // namespace tags::plugin_package

namespace tags::plugin_name {

class BaseName : public QObject {
  Q_OBJECT

 public:
  CREATE_PROPERTY(QString, autogain, QStringLiteral("autogain"));
  CREATE_PROPERTY(QString, bassEnhancer, QStringLiteral("bass_enhancer"));
  CREATE_PROPERTY(QString, bassLoudness, QStringLiteral("bass_loudness"));
  CREATE_PROPERTY(QString, compressor, QStringLiteral("compressor"));
  CREATE_PROPERTY(QString, convolver, QStringLiteral("convolver"));
  CREATE_PROPERTY(QString, crossfeed, QStringLiteral("crossfeed"));
  CREATE_PROPERTY(QString, crystalizer, QStringLiteral("crystalizer"));
  CREATE_PROPERTY(QString, deepfilternet, QStringLiteral("deepfilternet"));
  CREATE_PROPERTY(QString, deesser, QStringLiteral("deesser"));
  CREATE_PROPERTY(QString, delay, QStringLiteral("delay"));
  CREATE_PROPERTY(QString, echoCanceller, QStringLiteral("echo_canceller"));
  CREATE_PROPERTY(QString, equalizer, QStringLiteral("equalizer"));
  CREATE_PROPERTY(QString, exciter, QStringLiteral("exciter"));
  CREATE_PROPERTY(QString, expander, QStringLiteral("expander"));
  CREATE_PROPERTY(QString, filter, QStringLiteral("filter"));
  CREATE_PROPERTY(QString, gate, QStringLiteral("gate"));
  CREATE_PROPERTY(QString, levelMeter, QStringLiteral("level_meter"));
  CREATE_PROPERTY(QString, limiter, QStringLiteral("limiter"));
  CREATE_PROPERTY(QString, loudness, QStringLiteral("loudness"));
  CREATE_PROPERTY(QString, maximizer, QStringLiteral("maximizer"));
  CREATE_PROPERTY(QString, multibandCompressor, QStringLiteral("multiband_compressor"));
  CREATE_PROPERTY(QString, multibandGate, QStringLiteral("multiband_gate"));
  CREATE_PROPERTY(QString, outputLevel, QStringLiteral("output_level"));
  CREATE_PROPERTY(QString, pitch, QStringLiteral("pitch"));
  CREATE_PROPERTY(QString, speex, QStringLiteral("speex"));
  CREATE_PROPERTY(QString, spectrum, QStringLiteral("spectrum"));
  CREATE_PROPERTY(QString, reverb, QStringLiteral("reverb"));
  CREATE_PROPERTY(QString, rnnoise, QStringLiteral("rnnoise"));
  CREATE_PROPERTY(QString, stereoTools, QStringLiteral("stereo_tools"));
};

class Model : public QAbstractListModel {
  Q_OBJECT

 public:
  explicit Model(QObject* parent = nullptr);

  Model(const Model&) = delete;
  Model(Model&&) = delete;
  Model& operator=(const Model&) = delete;
  Model& operator=(Model&&) = delete;

  static Model& self() {
    static Model m;
    return m;
  }

  enum class Roles { Name = Qt::UserRole, TranslatedName };

  [[nodiscard]] int rowCount(const QModelIndex& /* parent */) const override;

  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;

  auto getMap() -> QMap<QString, QString>;

  auto getBaseName(const QString& name) -> QString;

  Q_INVOKABLE QList<QString> getBaseNames();
  Q_INVOKABLE QString translate(const QString& baseName);

 private:
  const QMap<QString, QString> modelMap;

  BaseName base_name;
  plugin_package::Package package;
};

auto get_id(const QString& name) -> QString;

}  // namespace tags::plugin_name

#undef CREATE_PROPERTY
