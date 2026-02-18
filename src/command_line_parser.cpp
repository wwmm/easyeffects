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

#include "command_line_parser.hpp"
#include <qcommandlineparser.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <KAboutData>
#include <KLocalizedString>
#include <QApplication>
#include <QLoggingCategory>
#include <QString>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include "easyeffects_db.h"
#include "pipeline_type.hpp"
#include "presets_manager.hpp"
#include "util.hpp"

CommandLineParser::CommandLineParser(KAboutData& about, QObject* parent)
    : QObject(parent), parser(std::make_unique<QCommandLineParser>()) {
  parser->setApplicationDescription("Easy Effects");

  about.setupCommandLine(parser.get());
  parser->addOptions(
      {{{"q", "quit"}, i18n("Quit Easy Effects. Useful when running in service mode.")},
       {{"r", "reset"}, i18n("Reset Easy Effects.")},
       {{"w", "hide-window"}, i18n("Hide the window.")},
       {{"b", "bypass"},
        i18n("Global bypass. 1 to enable, 2 to disable and 3 to get the current state."),
        i18n("bypass-state")},
       {{"l", "load-preset"}, i18n("Load a preset. Example: easyeffects -l music"), i18n("preset-name")},
       {{"p", "presets"}, i18n("Show available presets.")},
       {{"a", "last-loaded-preset"}, i18n("Get the last loaded input/output preset."), i18n("preset-type")},
       {{"s", "last-loaded-presets"}, i18n("Get the last loaded input and output presets.")},
       {"gapplication-service", i18n("Deprecated. Use --service-mode instead.")},
       {"service-mode", i18n("Start the application with service mode turned on.")},
       {"debug", i18n("Enable debug messages.")}});
}

void CommandLineParser::set_is_primary(const bool& state) {
  is_primary = state;
}

void CommandLineParser::process(KAboutData& about, QApplication* app) {
  parser->process(*app);

  about.processCommandLine(parser.get());
}

void CommandLineParser::process_debug_option() {
  if (parser->isSet("debug")) {
    QLoggingCategory::setFilterRules("easyeffects.debug=true");
  }
}

void CommandLineParser::process_hide_window(bool& show_window) {
  if (parser->isSet("hide-window")) {
    show_window = false;
  }
}

void CommandLineParser::process_events() {
  auto* pm = &presets::Manager::self();

  if (parser->isSet("quit")) {
    Q_EMIT onQuit();
  }

  if (parser->isSet("reset")) {
    Q_EMIT onReset();
  }

  if (parser->isSet("hide-window")) {
    Q_EMIT onHideWindow();
  }

  if (parser->isSet("service-mode")) {
    DbMain::setEnableServiceMode(true);
  }

  if (parser->isSet("gapplication-service")) {
    DbMain::setEnableServiceMode(true);

    Q_EMIT onHideWindow();
  }

  if (parser->isSet("last-loaded-preset")) {
    bool ok = true;

    const auto value = parser->value("last-loaded-preset");

    if (value == "input" || value == "output") {
      if (is_primary) {  // no daemon running
        auto preset = (value == "input") ? DbMain::lastLoadedInputPreset() : DbMain::lastLoadedOutputPreset();

        if (preset.length() < 1) {
          preset = QString("None");
        }

        std::cout << preset.toStdString() << '\n';
      } else {
        Q_EMIT onGetLastLoadedPreset((value == "input") ? PipelineType::input : PipelineType::output);
      }
    } else {
      ok = false;

      std::cout << i18n("Must specify preset type: input/output.").toStdString() << '\n';
    }

    Q_EMIT onHideWindow();

    if (ok) {
      QCoreApplication::exit(EXIT_SUCCESS);
    } else {
      QCoreApplication::exit(EXIT_FAILURE);
    }
  }

  if (parser->isSet("last-loaded-presets")) {
    if (is_primary) {
      auto input = DbMain::lastLoadedInputPreset();
      auto output = DbMain::lastLoadedOutputPreset();

      std::cout << "Input: " << input.toStdString() << '\n';
      std::cout << "Output: " << output.toStdString() << '\n';
    } else {
      Q_EMIT onGetLastLoadedInputOutputPreset();
    }

    QCoreApplication::exit(EXIT_SUCCESS);
  }

  if (parser->isSet("presets")) {
    std::string list;
    int i = 0;

    for (const auto& p : pm->get_local_presets_paths(PipelineType::output)) {
      list += util::to_string(++i) + '\t' + p.stem().string() + '\n';
    }

    if (i > 0) {
      std::cout << i18n("Output presets").toStdString() + ":\n" + list << '\n';
    } else {
      std::cout << i18n("No output presets.").toStdString() << '\n';
    }

    list = "";
    i = 0;

    for (const auto& p : pm->get_local_presets_paths(PipelineType::input)) {
      list += util::to_string(++i) + '\t' + p.stem().string() + '\n';
    }

    if (i > 0) {
      std::cout << i18n("Input presets").toStdString() + ":\n" + list << '\n';
    } else {
      std::cout << i18n("No input presets.").toStdString() << '\n';
    }

    Q_EMIT onHideWindow();

    QCoreApplication::exit(EXIT_SUCCESS);
  }

  if (parser->isSet("load-preset")) {
    bool ok = false;

    const auto name = parser->value("load-preset");

    if (pm->preset_file_exists(PipelineType::input, name.toStdString())) {
      Q_EMIT onLoadPreset(PipelineType::input, name);

      ok = true;
    }

    if (pm->preset_file_exists(PipelineType::output, name.toStdString())) {
      Q_EMIT onLoadPreset(PipelineType::output, name);

      ok = true;
    }

    Q_EMIT onHideWindow();

    if (ok) {
      QCoreApplication::exit(EXIT_SUCCESS);
    } else {
      std::cout << i18n("Specified preset does not exist.").toStdString() << '\n';

      QCoreApplication::exit(EXIT_FAILURE);
    }
  }

  if (parser->isSet("bypass")) {
    bool ok = true;

    const auto value = parser->value("bypass");

    if (value == "1") {
      Q_EMIT onSetGlobalBypass(true);
    } else if (value == "2") {
      Q_EMIT onSetGlobalBypass(false);
    } else if (value == "3") {
      Q_EMIT onGetGlobalBypass();
    } else {
      ok = false;

      std::cout << i18n("Provided an invalid bypass state.").toStdString() << '\n';
    }

    Q_EMIT onHideWindow();

    if (ok) {
      QCoreApplication::exit(EXIT_SUCCESS);
    } else {
      QCoreApplication::exit(EXIT_FAILURE);
    }
  }

  if (is_primary) {
    Q_EMIT onInitQML();
  }
}
