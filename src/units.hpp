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

#include <qqmlintegration.h>
#include <qtmetamacros.h>
#include <KLocalizedString>
#include <QObject>

class Units : public QObject {
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

  Q_PROPERTY(QString dB MEMBER dB CONSTANT)
  Q_PROPERTY(QString s MEMBER s CONSTANT)
  Q_PROPERTY(QString ms MEMBER ms CONSTANT)
  Q_PROPERTY(QString kHz MEMBER kHz CONSTANT)
  Q_PROPERTY(QString percentage MEMBER percentage CONSTANT)
  Q_PROPERTY(QString lufs MEMBER lufs CONSTANT)
  Q_PROPERTY(QString lu MEMBER lu CONSTANT)

 public:
  explicit Units(QObject* parent = nullptr) : QObject(parent) {
    dB = ki18nc("decibel", "dB").toString();
    s = ki18nc("seconds", "s").toString();
    ms = ki18nc("milliseconds", "ms").toString();
    kHz = ki18nc("kiloHertz", "kHz").toString();
    percentage = ki18nc("percentage", "%").toString();
    lufs = ki18nc("loudness units full scale", "LUFS").toString();
    lu = ki18nc("loudness units", "LU").toString();
  }

 private:
  QString dB;
  QString s;
  QString ms;
  QString kHz;
  QString percentage;
  QString lufs;
  QString lu;
};
