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

#include "local_server.hpp"
#include <kconfigskeleton.h>
#include <qobject.h>
#include <qstandardpaths.h>
#include <qtmetamacros.h>
#include <QLocalServer>
#include <QMetaType>
#include <cstring>
#include <format>
#include <memory>
#include <regex>
#include <string>
#include "db_manager.hpp"
#include "pipeline_type.hpp"
#include "presets_manager.hpp"
#include "tags_local_server.hpp"
#include "util.hpp"

LocalServer::LocalServer(QObject* parent) : QObject(parent), server(std::make_unique<QLocalServer>(this)) {
  connect(server.get(), &QLocalServer::newConnection, [&]() {
    auto* newSocket = server->nextPendingConnection();

    connect(newSocket, &QLocalSocket::readyRead, this, &LocalServer::onReadyRead);
    connect(newSocket, &QLocalSocket::disconnected, this, &LocalServer::onDisconnected);

    util::debug("Client connected");
  });
}

LocalServer::~LocalServer() {
  server->close();
}

void LocalServer::startServer() {
  auto path = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation) + "/" + tags::local_server::server_name;

  QLocalServer::removeServer(path);

  if (server->listen(path)) {
    util::debug(std::format("Local socket server started. Listening on the name: {}", tags::local_server::server_name));

  } else {
    util::debug("Failed to start the server");
  }
}

auto LocalServer::pipeline_from(const std::string& str) -> PipelineType {
  if (str == "input") {
    return PipelineType::input;
  } else if (str == "output") {
    return PipelineType::output;
  }

  util::warning(std::format("LocalServer: Invalid pipeline '{}', defaulting to output", str));
  return PipelineType::output;
}

void LocalServer::onReadyRead() {
  auto* socket = qobject_cast<QLocalSocket*>(sender());

  if (!socket) {
    return;
  }

  while (!socket->atEnd()) {
    char buf[1024];

    auto lineLength = socket->readLine(buf, sizeof(buf));

    if (lineLength != -1) {
      if (std::strcmp(buf, tags::local_server::quit_app) == 0) {
        Q_EMIT onQuitApp();
      } else if (std::strcmp(buf, tags::local_server::show_window) == 0) {
        Q_EMIT onShowWindow();
      } else if (std::strcmp(buf, tags::local_server::hide_window) == 0) {
        Q_EMIT onHideWindow();
      } else if (std::strncmp(buf, tags::local_server::global_bypass, strlen(tags::local_server::global_bypass)) == 0) {
        std::string msg = buf;

        std::smatch matches;

        static const auto re = std::regex("^global_bypass:([01])\n$");

        std::regex_search(msg, matches, re);

        if (matches.size() == 2U) {
          int state = 0;

          util::str_to_num(std::string(matches[1]), state);

          DbMain::setBypass(state);
        }
      } else if (std::strncmp(buf, tags::local_server::load_preset, strlen(tags::local_server::load_preset)) == 0) {
        std::string msg = buf;

        std::smatch matches;

        static const auto re = std::regex("^load_preset:(input|output):([^\n]{1,100})\n$");

        std::regex_search(msg, matches, re);

        if (matches.size() == 3U) {
          auto pipeline_type = pipeline_from(matches[1].str());

          std::string preset_name = matches[2];

          presets::Manager::self().loadLocalPresetFile(pipeline_type, QString::fromStdString(preset_name));
        }
      } else if (std::strncmp(buf, tags::local_server::set_property, strlen(tags::local_server::set_property)) == 0) {
        std::string msg = buf;

        std::smatch matches;

        /**
         * Original regex:
         * ^set_property:(input|output):([^:]+):([0-9]+):([^:]+):(.+)\n$
         *
         * Since the dot matches any character except line terminators, there's
         * no need to search for final new line and end of line position.
         */
        static const auto re = std::regex("^set_property:(input|output):([^:]+):([0-9]+):([^:]+):([^\n]+)");

        std::regex_search(msg, matches, re);

        if (matches.size() == 6U) {
          const auto& pipeline = matches[1].str();
          const auto& plugin_name = matches[2].str();
          const auto& instance_id = matches[3].str();
          const auto& property = matches[4].str();
          const auto& value = matches[5].str();

          set_property(pipeline, plugin_name, instance_id, property, value);
        }
      } else if (std::strncmp(buf, tags::local_server::get_property, strlen(tags::local_server::get_property)) == 0) {
        /**
         * Example of client write that should be done:
         * client->write(std::format("{}:output:loudness:0:volume\n", tags::local_server::get_property).c_str());
         */

        std::string msg = buf;

        std::smatch matches;

        static const auto re = std::regex("^get_property:(input|output):([^:]+):([0-9]+):([^\n]+)");

        std::regex_search(msg, matches, re);

        if (matches.size() == 5U) {
          const auto& pipeline = matches[1].str();
          const auto& plugin_name = matches[2].str();
          const auto& instance_id = matches[3].str();
          const auto& property = matches[4].str();

          const auto value = get_property(pipeline, plugin_name, instance_id, property);

          socket->write((value + "\n").c_str());
        }
      } else if (std::strncmp(buf, tags::local_server::get_last_loaded_preset,
                              strlen(tags::local_server::get_last_loaded_preset)) == 0) {
        std::string msg = buf;

        std::smatch matches;

        static const auto re = std::regex("^get_last_loaded_preset:(input|output)\n$");

        std::regex_search(msg, matches, re);

        if (matches.size() == 2U) {
          auto pipeline_type = pipeline_from(matches[1].str());

          QString preset_name = (pipeline_type == PipelineType::input) ? DbMain::lastLoadedInputPreset() + "\n"
                                                                       : DbMain::lastLoadedOutputPreset() + "\n";

          socket->write(preset_name.toUtf8());
        }
      }
    }
  }

  socket->flush();
}

void LocalServer::onDisconnected() {
  util::debug("Client disconnected");

  auto* socket = qobject_cast<QLocalSocket*>(sender());

  if (socket) {
    socket->deleteLater();
    socket = nullptr;
  }
}

void LocalServer::set_property(const std::string& pipeline,
                               const std::string& plugin_name,
                               const std::string& instance_id,
                               const std::string& property,
                               const std::string& value) {
  auto type = pipeline_from(pipeline);
  QString key = QString::fromStdString(plugin_name + "#" + instance_id);

  auto& mgr = db::Manager::self();
  QObject* db = nullptr;

  if (type == PipelineType::input && mgr.siePluginsDB.contains(key)) {
    db = mgr.siePluginsDB.value(key).value<KConfigSkeleton*>();
  } else if (type == PipelineType::output && mgr.soePluginsDB.contains(key)) {
    db = mgr.soePluginsDB.value(key).value<KConfigSkeleton*>();
  }

  if (!db) {
    util::warning(std::format("LocalServer: Plugin DB not found: {}", key.toStdString()));
    return;
  }

  QVariant current = db->property(property.c_str());

  if (!current.isValid()) {
    util::warning(std::format("LocalServer: Property '{}' invalid on {}", property, key.toStdString()));
    return;
  }

  util::debug(std::format("LocalServer: Setting property '{}' to '{}' on {}", property, value, key.toStdString()));

  switch (current.typeId()) {
    case QMetaType::Bool: {
      db->setProperty(property.c_str(), (value == "true" || value == "1" || value == "on"));
      break;
    }
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong: {
      try {
        db->setProperty(property.c_str(), std::stoi(value));
      } catch (...) {
        util::warning(std::format("LocalServer: Invalid integer value for {}", property));
      }

      break;
    }
    case QMetaType::Double:
    case QMetaType::Float: {
      double v = 0.0;

      if (util::str_to_num(value, v)) {
        db->setProperty(property.c_str(), v);
      } else {
        util::warning(std::format("LocalServer: Invalid float value for {}", property));
      }

      break;
    }
    case QMetaType::QString:
      db->setProperty(property.c_str(), QString::fromStdString(value));
      break;
    default:
      util::warning(std::format("LocalServer: Unhandled type {} for {}", current.typeName(), property));
  }
}

auto LocalServer::get_property(const std::string& pipeline,
                               const std::string& plugin_name,
                               const std::string& instance_id,
                               const std::string& property) -> std::string {
  PipelineType type = (pipeline == "input") ? PipelineType::input : PipelineType::output;
  QString key = QString::fromStdString(plugin_name + "#" + instance_id);

  auto& mgr = db::Manager::self();
  QObject* db = nullptr;

  if (type == PipelineType::input && mgr.siePluginsDB.contains(key)) {
    db = mgr.siePluginsDB.value(key).value<KConfigSkeleton*>();
  } else if (type == PipelineType::output && mgr.soePluginsDB.contains(key)) {
    db = mgr.soePluginsDB.value(key).value<KConfigSkeleton*>();
  }

  if (!db) {
    return "error_plugin_not_found";
  }

  QVariant val = db->property(property.c_str());
  if (!val.isValid()) {
    return "error_property_not_found";
  }

  return val.toString().toStdString();
}
