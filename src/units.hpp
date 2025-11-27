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

#pragma once

#include <qobject.h>
#include <qqmlintegration.h>
#include <qtmetamacros.h>
#include <KLazyLocalizedString>

// NOLINTNEXTLINE
#define CREATE_PROPERTY(type, name, value)          \
  static inline const type name = value.toString(); \
  Q_PROPERTY(type name MEMBER name CONSTANT)

class Units : public QObject {
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

 public:
  CREATE_PROPERTY(QString, dB, kli18nc("decibel", "dB"));
  CREATE_PROPERTY(QString, s, kli18nc("seconds", "s"));
  CREATE_PROPERTY(QString, ms, kli18nc("milliseconds", "ms"));
  CREATE_PROPERTY(QString, kHz, kli18nc("kiloHertz", "kHz"));
  CREATE_PROPERTY(QString, percentage, kli18nc("percentage", "%"));
  CREATE_PROPERTY(QString, lufs, kli18nc("loudness units full scale", "LUFS"));
  CREATE_PROPERTY(QString, lu, kli18nc("loudness units", "LU"));
};
