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

#pragma once

#include <qtmetamacros.h>
#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <memory>

class LocalServer : public QObject {
  Q_OBJECT

 public:
  explicit LocalServer(QObject* parent = nullptr);

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
};
