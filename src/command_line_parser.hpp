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

#include <qtmetamacros.h>
#include <QApplication>
#include <QCommandLineParser>
#include <QObject>
#include <memory>
#include "pipeline_type.hpp"

class KAboutData;

class CommandLineParser : public QObject {
  Q_OBJECT

 public:
  explicit CommandLineParser(KAboutData& about, QObject* parent = nullptr);

  void process(KAboutData& about, QApplication* app);

  void process_debug_option();

  void process_hide_window(bool& show_window);

  void process_events();

  void set_is_primary(const bool& state);

 Q_SIGNALS:
  void onReset();
  void onQuit();
  void onHideWindow();

  void onLoadPreset(PipelineType pipeline_type, QString preset_name);

  void onGetLastLoadedPreset(PipelineType pipeline_type);

  void onGetLastLoadedInputOutputPreset();

  void onSetGlobalBypass(bool state);

  void onGetGlobalBypass();

  void onInitQML();

 private:
  std::unique_ptr<QCommandLineParser> parser;

  bool is_primary = true;
};
