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

#include <qtmetamacros.h>
#include <QApplication>
#include <QCommandLineParser>
#include <QObject>
#include <memory>
#include "pipeline_type.hpp"

class CommandLineParser : public QObject {
  Q_OBJECT

 public:
  explicit CommandLineParser(QObject* parent = nullptr);

  void process(QApplication* app);

 Q_SIGNALS:
  void onReset();
  void onQuit();
  void onHideWindow();

  void onLoadPreset(PipelineType pipeline_type, QString preset_name);

  void onSetGlobalBypass(bool state);

 private:
  std::unique_ptr<QCommandLineParser> parser;
};
