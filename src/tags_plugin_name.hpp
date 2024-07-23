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
#include <string>

namespace tags {

// NOLINTNEXTLINE
#define CREATE_PROPERTY(type, name, value) \
  static inline const type name = value;   \
  Q_PROPERTY(type name MEMBER name CONSTANT)

class PluginName : public QObject {
  Q_OBJECT
  QML_UNCREATABLE("Constants with the plugins name")
  QML_ELEMENT
  QML_SINGLETON

 public:
  CREATE_PROPERTY(QString, autogain, QStringLiteral("autogain"));
  CREATE_PROPERTY(QString, bass_enhancer, QStringLiteral("bass_enhancer"));
  CREATE_PROPERTY(QString, bass_loudness, QStringLiteral("bass_loudness"));
  CREATE_PROPERTY(QString, compressor, QStringLiteral("compressor"));
  CREATE_PROPERTY(QString, convolver, QStringLiteral("convolver"));
  CREATE_PROPERTY(QString, crossfeed, QStringLiteral("crossfeed"));
  CREATE_PROPERTY(QString, crystalizer, QStringLiteral("crystalizer"));
  CREATE_PROPERTY(QString, deepfilternet, QStringLiteral("deepfilternet"));
  CREATE_PROPERTY(QString, deesser, QStringLiteral("deesser"));
  CREATE_PROPERTY(QString, delay, QStringLiteral("delay"));
  CREATE_PROPERTY(QString, echo_canceller, QStringLiteral("echo_canceller"));
  CREATE_PROPERTY(QString, equalizer, QStringLiteral("equalizer"));
  CREATE_PROPERTY(QString, exciter, QStringLiteral("exciter"));
  CREATE_PROPERTY(QString, expander, QStringLiteral("expander"));
  CREATE_PROPERTY(QString, filter, QStringLiteral("filter"));
  CREATE_PROPERTY(QString, gate, QStringLiteral("gate"));
  CREATE_PROPERTY(QString, level_meter, QStringLiteral("level_meter"));
  CREATE_PROPERTY(QString, limiter, QStringLiteral("limiter"));
  CREATE_PROPERTY(QString, loudness, QStringLiteral("loudness"));
  CREATE_PROPERTY(QString, maximizer, QStringLiteral("maximizer"));
  CREATE_PROPERTY(QString, multiband_compressor, QStringLiteral("multiband_compressor"));
  CREATE_PROPERTY(QString, multiband_gate, QStringLiteral("multiband_gate"));
  CREATE_PROPERTY(QString, pitch, QStringLiteral("pitch"));
  CREATE_PROPERTY(QString, speex, QStringLiteral("speex"));
  CREATE_PROPERTY(QString, reverb, QStringLiteral("reverb"));
  CREATE_PROPERTY(QString, rnnoise, QStringLiteral("rnnoise"));
  CREATE_PROPERTY(QString, stereo_tools, QStringLiteral("stereo_tools"));
};

}  // namespace tags

namespace tags::plugin_package {

inline constexpr auto bs2b = "bs2b";

inline constexpr auto calf = "Calf Studio Gear";

inline constexpr auto deepfilternet = "DeepFilterNet";

inline constexpr auto ebur128 = "libebur128";

inline constexpr auto ee = "Easy Effects";

inline constexpr auto lsp = "Linux Studio Plugins";

inline constexpr auto mda = "MDA";

inline constexpr auto rnnoise = "RNNoise";

inline constexpr auto sound_touch = "SoundTouch";

inline constexpr auto speex = "SpeexDSP";

inline constexpr auto zam = "ZamAudio";

inline constexpr auto zita = "Zita";

}  // namespace tags::plugin_package

namespace tags::plugin_name {

class Model : public QAbstractListModel {
  Q_OBJECT;

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

  enum Roles { Name = Qt::UserRole, TranslatedName };

  [[nodiscard]] int rowCount(const QModelIndex& /*parent*/) const override;

  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;

  auto getMap() -> QMap<QString, QString>;

  Q_INVOKABLE QList<QString> getBaseNames();
  Q_INVOKABLE QString translate(const QString& baseName);

 private:
  QMap<QString, QString> modelMap;
};

auto get_id(const std::string& name) -> uint;

}  // namespace tags::plugin_name

namespace TagsPluginName {
Q_NAMESPACE
// QML_ELEMENT

enum class Bar {
  VAL1,
  VAL2,
};
Q_ENUM_NS(Bar)

inline constexpr auto autogain = "autogain";

inline QString exciter = "exciter";
}  // namespace TagsPluginName