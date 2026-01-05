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
#include <QLocalSocket>
#include <QObject>
#include <memory>
#include <string>
#include "pipeline_type.hpp"

class LocalClient : public QObject {
  Q_OBJECT

 public:
  explicit LocalClient(QObject* parent = nullptr);

  void show_window();
  void hide_window();
  void quit_app();

  void load_preset(PipelineType pipeline_type, std::string preset_name);

  void setGlobalBypass(const bool& state);

  auto getLastLoadedPreset(PipelineType pipeline_type) -> QString;

 private:
  std::unique_ptr<QLocalSocket> client;
};
