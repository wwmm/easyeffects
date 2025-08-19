/*
 *  Copyright © 2017-2025 Wellington Wallace
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

#include "command_line_parser.hpp"
#include <qcommandlineparser.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <KLocalizedString>
#include <QApplication>
#include <QString>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include "easyeffects_db.h"
#include "pipeline_type.hpp"
#include "presets_manager.hpp"
#include "util.hpp"

CommandLineParser::CommandLineParser(QObject* parent)
    : QObject(parent), parser(std::make_unique<QCommandLineParser>()) {
  parser->setApplicationDescription("EasyEffects");

  parser->addHelpOption();
  parser->addVersionOption();

  parser->addOptions({{{"q", "quit"}, i18n("Quit Easy Effects. Useful when running in service mode.")},
                      {{"r", "reset"}, i18n("Reset Easy Effects.")},
                      {{"w", "hide-window"}, i18n("Hide the Window.")},
                      {{"b", "bypass"}, i18n("Global bypass. 1 to enable, 2 to disable and 3 to get status")},
                      {{"l", "load-preset"}, i18n("Load a preset. Example: easyeffects -l music"), i18n("preset-name")},
                      {{"p", "presets"}, i18n("Show available presets.")},
                      {{"a", "active-preset"}, i18n("Get the active input/output preset."), i18n("preset-type")},
                      {{"s", "active-presets"}, i18n("Get the active input and output presets.")}});
}

void CommandLineParser::process(QApplication* app) {
  auto* pm = &presets::Manager::self();

  parser->process(*app);

  if (parser->isSet("quit")) {
    Q_EMIT onQuit();
  }

  if (parser->isSet("reset")) {
    Q_EMIT onReset();
  }

  if (parser->isSet("hide-window")) {
    Q_EMIT onHideWindow();
  }

  if (parser->isSet("active-preset")) {
    auto value = parser->value("active-preset");

    if (value == "input") {
      auto preset = db::Main::lastLoadedInputPreset();

      if (preset.length() < 1) {
        preset = QString("None");
      }

      std::cout << preset.toStdString() << "\n";
    } else if (value == "output") {
      auto preset = db::Main::lastLoadedOutputPreset();

      if (preset.length() < 1) {
        preset = QString("None");
      }

      std::cout << preset.toStdString() << "\n";
    } else {
      util::fatal("Must specify preset type: input/output.");
    }

    std::exit(EXIT_SUCCESS);
  }

  if (parser->isSet("active-presets")) {
    auto input = db::Main::lastLoadedInputPreset();
    auto output = db::Main::lastLoadedOutputPreset();

    if (input.length() < 1) {
      input = QString("None");
    }
    if (output.length() < 1) {
      output = QString("None");
    }

    std::cout << "Input: " << input.toStdString() << '\n';
    std::cout << "Output: " << output.toStdString() << '\n';

    std::exit(EXIT_SUCCESS);
  }

  if (parser->isSet("presets")) {
    std::string list;

    for (const auto& p : pm->get_local_presets_paths(PipelineType::output)) {
      list += p.stem().string() + ",";
    }

    std::cout << i18n("Output Presets").toStdString() + ": " + list << '\n';

    list = "";

    for (const auto& p : pm->get_local_presets_paths(PipelineType::input)) {
      list += p.stem().string() + ",";
    }

    std::cout << i18n("Input Presets").toStdString() + ": " + list << '\n';

    std::exit(EXIT_SUCCESS);
  }

  if (parser->isSet("load-preset")) {
    auto name = parser->value("load-preset");

    if (pm->preset_file_exists(PipelineType::input, name.toStdString())) {
      Q_EMIT onLoadPreset(PipelineType::input, name);

      std::exit(EXIT_SUCCESS);
    }

    if (pm->preset_file_exists(PipelineType::output, name.toStdString())) {
      Q_EMIT onLoadPreset(PipelineType::output, name);

      std::exit(EXIT_SUCCESS);
    }

    std::exit(EXIT_FAILURE);
  }
}
