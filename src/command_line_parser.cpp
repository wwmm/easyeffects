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

#include "command_line_parser.hpp"
#include <qcommandlineparser.h>
#include <qobject.h>
#include <qtmetamacros.h>
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

CommandLineParser::CommandLineParser(QObject* parent)
    : QObject(parent), parser(std::make_unique<QCommandLineParser>()) {
  parser->setApplicationDescription("Easy Effects");

  parser->addHelpOption();
  parser->addVersionOption();

  parser->addOptions(
      {{{"q", "quit"}, i18n("Quit Easy Effects. Useful when running in service mode.")},
       {{"r", "reset"}, i18n("Reset Easy Effects.")},
       {{"w", "hide-window"}, i18n("Hide the Window.")},
       {{"b", "bypass"}, i18n("Global bypass. 1 to enable, 2 to disable and 3 to get status")},
       {{"l", "load-preset"}, i18n("Load a preset. Example: easyeffects -l music"), i18n("preset-name")},
       {{"p", "presets"}, i18n("Show available presets.")},
       {{"a", "active-preset"}, i18n("Get the active input/output preset."), i18n("preset-type")},
       {{"s", "active-presets"}, i18n("Get the active input and output presets.")},
       {"set-property", i18n("Set plugin property. Format: pipeline:plugin_name:instance_id:property:value"),
        i18n("property-string")},
       {"gapplication-service", i18n("Deprecated. Use --service-mode instead.")},
       {"service-mode", i18n("Start the application with service mode turned on.")},
       {"debug", i18n("Enable debug messages.")}});
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

  if (parser->isSet("service-mode")) {
    db::Main::setEnableServiceMode(true);
  }

  if (parser->isSet("gapplication-service")) {
    db::Main::setEnableServiceMode(true);

    Q_EMIT onHideWindow();
  }

  if (parser->isSet("debug")) {
    QLoggingCategory::setFilterRules("easyeffects.debug=true");
  }

  if (parser->isSet("active-preset")) {
    bool ok = true;

    const auto value = parser->value("active-preset");

    if (value == "input") {
      auto preset = db::Main::lastLoadedInputPreset();

      if (preset.length() < 1) {
        preset = QString("None");
      }

      std::cout << preset.toStdString() << '\n';
    } else if (value == "output") {
      auto preset = db::Main::lastLoadedOutputPreset();

      if (preset.length() < 1) {
        preset = QString("None");
      }

      std::cout << preset.toStdString() << '\n';
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

    Q_EMIT onHideWindow();

    QCoreApplication::exit(EXIT_SUCCESS);
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

    Q_EMIT onHideWindow();

    QCoreApplication::exit(EXIT_SUCCESS);
  }

  if (parser->isSet("load-preset")) {
    const auto name = parser->value("load-preset");

    if (pm->preset_file_exists(PipelineType::input, name.toStdString())) {
      Q_EMIT onLoadPreset(PipelineType::input, name);

      QCoreApplication::exit(EXIT_SUCCESS);
    }

    if (pm->preset_file_exists(PipelineType::output, name.toStdString())) {
      Q_EMIT onLoadPreset(PipelineType::output, name);

      QCoreApplication::exit(EXIT_SUCCESS);
    }

    Q_EMIT onHideWindow();

    QCoreApplication::exit(EXIT_FAILURE);
  }

  if (parser->isSet("set-property")) {
    bool ok = true;

    const auto value = parser->value("set-property");

    // Parse the property string:
    // pipeline:plugin_name:instance_id:property:value
    const auto parts = value.split(':');

    if (parts.size() != 5) {
      ok = false;

      std::cout
          << i18n("Invalid property format. Expected: pipeline:plugin_name:instance_id:property:value").toStdString()
          << '\n';
    } else {
      const auto& pipeline = parts[0];
      const auto& plugin_name = parts[1];
      const auto& instance_id = parts[2];
      const auto& property_name = parts[3];
      const auto& property_value = parts[4];

      if (pipeline != "input" && pipeline != "output") {
        ok = false;

        std::cout << i18n("Invalid pipeline type. Must be 'input' or 'output'").toStdString() << '\n';
      }

      bool valid_instance = true;

      const auto instance = instance_id.toInt(&valid_instance);

      if (valid_instance && instance >= 0) {
        Q_EMIT onSetProperty(pipeline, plugin_name, instance_id, property_name, property_value);
      } else {
        ok = false;

        std::cout << i18n("Invalid instance ID. Must be a non-negative integer").toStdString() << '\n';
      }
    }

    Q_EMIT onHideWindow();

    if (ok) {
      QCoreApplication::exit(EXIT_SUCCESS);
    } else {
      QCoreApplication::exit(EXIT_FAILURE);
    }
  }
}
