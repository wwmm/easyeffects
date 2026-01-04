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

#include <qqmlintegration.h>
#include <qtmetamacros.h>
#include <KLocalizedString>
#include <QObject>

class Units : public QObject {
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

  Q_PROPERTY(QString leftCh MEMBER leftCh CONSTANT)
  Q_PROPERTY(QString rightCh MEMBER rightCh CONSTANT)
  Q_PROPERTY(QString sofa MEMBER sofa CONSTANT)
  Q_PROPERTY(QString wav MEMBER wav CONSTANT)
  Q_PROPERTY(QString dB MEMBER dB CONSTANT)
  Q_PROPERTY(QString dBFS MEMBER dBFS CONSTANT)
  Q_PROPERTY(QString dBoct MEMBER dBoct CONSTANT)
  Q_PROPERTY(QString s MEMBER s CONSTANT)
  Q_PROPERTY(QString ms MEMBER ms CONSTANT)
  Q_PROPERTY(QString hz MEMBER hz CONSTANT)
  Q_PROPERTY(QString kHz MEMBER kHz CONSTANT)
  Q_PROPERTY(QString lufs MEMBER lufs CONSTANT)
  Q_PROPERTY(QString lu MEMBER lu CONSTANT)
  Q_PROPERTY(QString m MEMBER m CONSTANT)
  Q_PROPERTY(QString cm MEMBER cm CONSTANT)
  Q_PROPERTY(QString degC MEMBER degC CONSTANT)
  Q_PROPERTY(QString bits MEMBER bits CONSTANT)
  Q_PROPERTY(QString frames MEMBER frames CONSTANT)
  Q_PROPERTY(QString fps MEMBER fps CONSTANT)
  Q_PROPERTY(QString k MEMBER k CONSTANT)
  Q_PROPERTY(QString percent MEMBER percent CONSTANT)
  Q_PROPERTY(QString degrees MEMBER degrees CONSTANT)
  Q_PROPERTY(QString minInf MEMBER minInf CONSTANT)

 public:
  explicit Units(QObject* parent = nullptr) : QObject(parent) {
    // Channels
    leftCh = ki18nc("left channel abbreviation (single letter)", "L").toString();
    rightCh = ki18nc("left channel abbreviation (single letter)", "R").toString();

    // Formats
    sofa = ki18nc("The SOFA audio file format", "SOFA").toString();
    wav = ki18nc("The WAV audio file format", "WAV").toString();

    // Measurement units
    dB = ki18nc("decibel unit abbreviation", "dB").toString();
    dBFS = ki18nc("decibel full scale unit abbreviation", "dBFS").toString();
    dBoct = ki18nc("decibel per octaves unit abbreviation", "dB/oct").toString();
    s = ki18nc("seconds unit abbreviation", "s").toString();
    ms = ki18nc("milliseconds unit abbreviation", "ms").toString();
    hz = ki18nc("Hertz unit abbreviation", "Hz").toString();
    kHz = ki18nc("kiloHertz unit abbreviation", "kHz").toString();
    lufs = ki18nc("loudness full scale unit abbreviation", "LUFS").toString();
    lu = ki18nc("loudness unit abbreviation", "LU").toString();
    m = ki18nc("meter unit abbreviation", "m").toString();
    cm = ki18nc("centimeter unit abbreviation", "cm").toString();
    degC = ki18nc("degrees Celsius unit abbreviation", "°C").toString();
    bits = ki18nc("bit unit (plural)", "bits").toString();
    frames = ki18nc("frame unit (plural)", "frames").toString();
    fps = ki18nc("frames per second abbreviation", "fps").toString();

    // Symbols
    k = ki18nc("thousand abbreviation (single letter)", "k").toString();
    percent = ki18nc("percentage symbol", "%").toString();
    degrees = ki18nc("degrees (angle) symbol", "°").toString();
    minInf = ki18nc("minus infinity abbreviation", "-inf").toString();
  }

 private:
  QString leftCh;
  QString rightCh;
  QString sofa;
  QString wav;
  QString dB;
  QString dBFS;
  QString dBoct;
  QString s;
  QString ms;
  QString hz;
  QString kHz;
  QString lufs;
  QString lu;
  QString m;
  QString cm;
  QString degC;
  QString bits;
  QString frames;
  QString fps;
  QString k;
  QString percent;
  QString degrees;
  QString minInf;
};
