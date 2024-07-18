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
#include <qstringview.h>
#include <qtmetamacros.h>
#include <qvariant.h>
#include <sys/types.h>

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

inline constexpr auto autogain = "autogain";

inline constexpr auto bass_enhancer = "bass_enhancer";

inline constexpr auto bass_loudness = "bass_loudness";

inline constexpr auto compressor = "compressor";

inline constexpr auto convolver = "convolver";

inline constexpr auto crossfeed = "crossfeed";

inline constexpr auto crystalizer = "crystalizer";

inline constexpr auto deepfilternet = "deepfilternet";

inline constexpr auto deesser = "deesser";

inline constexpr auto delay = "delay";

inline constexpr auto echo_canceller = "echo_canceller";

inline constexpr auto equalizer = "equalizer";

inline constexpr auto exciter = "exciter";

inline constexpr auto expander = "expander";

inline constexpr auto filter = "filter";

inline constexpr auto gate = "gate";

inline constexpr auto level_meter = "level_meter";

inline constexpr auto limiter = "limiter";

inline constexpr auto loudness = "loudness";

inline constexpr auto maximizer = "maximizer";

inline constexpr auto multiband_compressor = "multiband_compressor";

inline constexpr auto multiband_gate = "multiband_gate";

inline constexpr auto pitch = "pitch";

inline constexpr auto speex = "speex";

inline constexpr auto reverb = "reverb";

inline constexpr auto rnnoise = "rnnoise";

inline constexpr auto stereo_tools = "stereo_tools";

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

 private:
  QMap<QString, QString> modelMap;
};

}  // namespace tags::plugin_name