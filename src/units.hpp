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
  Q_PROPERTY(QString dBFS MEMBER dBFS CONSTANT)
  Q_PROPERTY(QString dBoct MEMBER dBoct CONSTANT)
  Q_PROPERTY(QString s MEMBER s CONSTANT)
  Q_PROPERTY(QString ms MEMBER ms CONSTANT)
  Q_PROPERTY(QString hz MEMBER hz CONSTANT)
  Q_PROPERTY(QString kHz MEMBER kHz CONSTANT)
  Q_PROPERTY(QString percent MEMBER percent CONSTANT)
  Q_PROPERTY(QString lufs MEMBER lufs CONSTANT)
  Q_PROPERTY(QString lu MEMBER lu CONSTANT)
  Q_PROPERTY(QString bits MEMBER bits CONSTANT)
  Q_PROPERTY(QString minInf MEMBER minInf CONSTANT)

 public:
  explicit Units(QObject* parent = nullptr) : QObject(parent) {
    dB = ki18nc("decibel unit abbreviation", "dB").toString();
    dBFS = ki18nc("decibel full scale unit abbreviation", "dBFS").toString();
    dBoct = ki18nc("decibel per octaves unit abbreviation", "dB/oct").toString();
    s = ki18nc("seconds unit abbreviation", "s").toString();
    ms = ki18nc("milliseconds unit abbreviation", "ms").toString();
    hz = ki18nc("Hertz unit abbreviation", "Hz").toString();
    kHz = ki18nc("kiloHertz unit abbreviation", "kHz").toString();
    percent = ki18nc("percentage symbol", "%").toString();
    lufs = ki18nc("loudness full scale unit abbreviation", "LUFS").toString();
    lu = ki18nc("loudness units abbreviation", "LU").toString();
    bits = ki18nc("bit unit (plural)", "bits").toString();
    minInf = ki18nc("minus infinity abbreviation", "-inf").toString();
  }

 private:
  QString dB;
  QString dBFS;
  QString dBoct;
  QString s;
  QString ms;
  QString hz;
  QString kHz;
  QString percent;
  QString lufs;
  QString lu;
  QString bits;
  QString minInf;
};
