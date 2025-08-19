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

#include "local_server.hpp"
#include <qobject.h>
#include <qtmetamacros.h>
#include <QLocalServer>
#include <cstring>
#include <memory>
#include <regex>
#include <string>
#include "pipeline_type.hpp"
#include "presets_manager.hpp"
#include "tags_local_server.hpp"
#include "util.hpp"

LocalServer::LocalServer(QObject* parent) : QObject(parent), server(std::make_unique<QLocalServer>(this)) {
  connect(server.get(), &QLocalServer::newConnection, [&]() {
    clientSocket = server->nextPendingConnection();

    connect(clientSocket, &QLocalSocket::readyRead, this, &LocalServer::onReadyRead);
    connect(clientSocket, &QLocalSocket::disconnected, this, &LocalServer::onDisconnected);

    util::debug("Client connected");
  });
}

void LocalServer::startServer() {
  QLocalServer::removeServer(tags::local_server::server_name);

  if (server->listen(tags::local_server::server_name)) {
    util::debug("Local socket server started. Listening on the name: " + std::string(tags::local_server::server_name));
  } else {
    util::debug("Failed to start the server");
  }
}

void LocalServer::onReadyRead() {
  while (!clientSocket->atEnd()) {
    char buf[1024];

    auto lineLength = clientSocket->readLine(buf, sizeof(buf));

    if (lineLength != -1) {
      if (std::strcmp(buf, tags::local_server::quit_app) == 0) {
        Q_EMIT onQuitApp();
      } else if (std::strcmp(buf, tags::local_server::show_window) == 0) {
        Q_EMIT onShowWindow();
      } else if (std::strcmp(buf, tags::local_server::hide_window) == 0) {
        Q_EMIT onHideWindow();
      } else if (std::strncmp(buf, tags::local_server::load_preset, strlen(tags::local_server::load_preset)) == 0) {
        std::string msg = buf;

        std::smatch matches;

        static const auto re_gain = std::regex("^load_preset:([0-9]+):([a-zA-Z]+)\n$");

        std::regex_search(msg, matches, re_gain);

        if (matches.size() == 3U) {
          int pipeline_type = 0;

          util::str_to_num(std::string(matches[1]), pipeline_type);

          std::string preset_name = matches[2];

          presets::Manager::self().loadLocalPresetFile(static_cast<PipelineType>(pipeline_type),
                                                       QString::fromStdString(preset_name));
        }
      }
    }
  }

  // Echo the data back to the client
  // clientSocket->write("Server received: " + data);
}

void LocalServer::onDisconnected() {
  util::debug("Client disconnected");

  clientSocket->deleteLater();
  clientSocket = nullptr;
}
