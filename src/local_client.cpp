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

#include "local_client.hpp"
#include <qlocalsocket.h>
#include <qobject.h>
#include <format>
#include <memory>
#include <string>
#include "pipeline_type.hpp"
#include "tags_local_server.hpp"
#include "util.hpp"

LocalClient::LocalClient(QObject* parent) : QObject(parent), client(std::make_unique<QLocalSocket>(this)) {
  connect(client.get(), &QLocalSocket::connected, [&]() {
    util::debug("Connected to the local server!");

    client->write("Hello, server!\n");
    client->flush();
  });

  connect(client.get(), &QLocalSocket::readyRead, [&]() {
    QString message = client->readAll();

    util::debug(std::format("Server message: {}", message.toStdString()));
  });

  client->connectToServer(tags::local_server::server_name);

  if (!client->waitForConnected()) {
    util::debug("Could not connect to the local server");
  }
}

void LocalClient::show_window() {
  client->write(tags::local_server::show_window);
  client->flush();
}

void LocalClient::hide_window() {
  client->write(tags::local_server::hide_window);
  client->flush();
}

void LocalClient::quit_app() {
  client->write(tags::local_server::quit_app);
  client->flush();
}

void LocalClient::load_preset(PipelineType pipeline_type, std::string preset_name) {
  auto msg = std::format("{}:{}:{}\n", tags::local_server::load_preset, static_cast<int>(pipeline_type), preset_name);

  client->write(msg.c_str());
  client->flush();
}

void LocalClient::set_property(const QString& pipeline,
                               const QString& plugin_name,
                               const QString& instance_id,
                               const QString& property_name,
                               const QString& property_value) {
  auto msg = std::format("{}:{}:{}:{}:{}:{}\n", tags::local_server::set_property, pipeline.toStdString(),
                         plugin_name.toStdString(), instance_id.toStdString(), property_name.toStdString(),
                         property_value.toStdString());

  client->write(msg.c_str());
  client->flush();
}
