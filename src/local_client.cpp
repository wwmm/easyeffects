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

#include "local_client.hpp"
#include <qlocalsocket.h>
#include <qobject.h>
#include <memory>
#include "tags_app.hpp"
#include "util.hpp"

LocalClient::LocalClient(QObject* parent) : QObject(parent), client(std::make_unique<QLocalSocket>(this)) {
  connect(client.get(), &QLocalSocket::connected, [&]() {
    util::debug("Connected to the local server!");

    client->write("Hello, server!\n");
    client->flush();
  });

  connect(client.get(), &QLocalSocket::readyRead, [&]() {
    QString message = client->readAll();

    util::debug("Server message: " + message.toStdString());
  });

  client->connectToServer(tags::app::local_server_name);

  if (!client->waitForConnected()) {
    util::debug("Could not connect to the local server");
  }
}

void LocalClient::show_main_window() {
  client->write("show_main_window\n");
  client->flush();
}

void LocalClient::show_version() {
  client->write("show_version\n");
  client->flush();
}