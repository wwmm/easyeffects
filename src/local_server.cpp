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

#include "local_server.hpp"
#include <qobject.h>
#include <QLocalServer>
#include <memory>
#include "util.hpp"

LocalServer::LocalServer(QObject* parent)
    : QObject(parent), server(std::make_unique<QLocalServer>(this)), clientSocket(nullptr) {
  connect(server.get(), &QLocalServer::newConnection, this, [&]() {
    clientSocket = server->nextPendingConnection();

    connect(clientSocket, &QLocalSocket::readyRead, this, &LocalServer::onReadyRead);
    connect(clientSocket, &QLocalSocket::disconnected, this, &LocalServer::onDisconnected);

    util::debug("Client connected");
  });
}

void LocalServer::startServer() {
  if (server->listen(serverName)) {
    util::debug("Local socket server started. Listening on the name: EasyEffectsServer");
  } else {
    util::debug("Failed to start the server");
  }
}

void LocalServer::onReadyRead() {
  QByteArray data = clientSocket->readAll();

  util::debug("Received from client:" + data.toStdString());

  // Echo the data back to the client
  clientSocket->write("Server received: " + data);
}

void LocalServer::onDisconnected() {
  util::debug("Client disconnected");

  clientSocket->deleteLater();
  clientSocket = nullptr;
}