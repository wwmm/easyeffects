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

#include "command_line_parser.hpp"
#include <qcommandlineparser.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <KLocalizedString>
#include <QApplication>
#include <memory>

CommandLineParser::CommandLineParser(QObject* parent)
    : QObject(parent), parser(std::make_unique<QCommandLineParser>()) {
  parser->setApplicationDescription("EasyEffects");

  parser->addHelpOption();
  parser->addVersionOption();

  parser->addOptions({{{"q", "quit"}, i18n("Quit Easy Effects. Useful when running in service mode.")},
                      {{"r", "reset"}, i18n("Reset Easy Effects.")},
                      {{"w", "hide-window"}, i18n("Hide the Window.")},
                      {{"b", "bypass"}, i18n("Global bypass. 1 to enable, 2 to disable and 3 to get status")},
                      {{"l", "load-preset"}, i18n("Hide the Window.")},
                      {{"p", "presets"}, i18n("Load a preset. Example: easyeffects -l music")}});
}

void CommandLineParser::process(QApplication* app) {
  parser->process(*app);

  if (parser->isSet("reset")) {
    Q_EMIT onReset();
  }

  if (parser->isSet("quit")) {
    Q_EMIT onQuit();
  }
}
