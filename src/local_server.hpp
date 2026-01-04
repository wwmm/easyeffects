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
#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <memory>
#include <string>
#include "pipeline_type.hpp"

class LocalServer : public QObject {
  Q_OBJECT

 public:
  explicit LocalServer(QObject* parent = nullptr);
  ~LocalServer() override;

  void startServer();
  void onNewConnection();
  void onReadyRead();
  void onDisconnected();

 Q_SIGNALS:
  void onShowWindow();
  void onHideWindow();
  void onQuitApp();

 private:
  std::unique_ptr<QLocalServer> server;

  QLocalSocket* clientSocket = nullptr;

  static auto pipeline_from(const std::string& str) -> PipelineType;

  static void set_property(const std::string& pipeline,
                           const std::string& plugin_name,
                           const std::string& instance_id,
                           const std::string& property,
                           const std::string& value);

  static auto get_property(const std::string& pipeline,
                           const std::string& plugin_name,
                           const std::string& instance_id,
                           const std::string& property) -> std::string;
};
