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

#include "pw_metadata_manager.hpp"

#include <pipewire/context.h>
#include <pipewire/core.h>
#include <pipewire/extensions/metadata.h>
#include <pipewire/keys.h>
#include <pipewire/properties.h>
#include <pipewire/proxy.h>
#include <pipewire/thread-loop.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <spa/utils/dict.h>
#include <spa/utils/hook.h>
#include <cstdint>
#include <cstring>
#include <format>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "tags_pipewire.hpp"
#include "util.hpp"

namespace pw {

auto MetadataManager::register_metadata(pw_registry* registry, uint32_t id, const char* type, const spa_dict* props)
    -> bool {
  if (const auto* name = spa_dict_lookup(props, PW_KEY_METADATA_NAME)) {
    util::debug(std::format("Found metadata: {}", name));

    if (std::strcmp(name, "default") == 0) {
      if (metadata != nullptr) {
        util::debug("A new default metadata is available. We will use it");

        spa_hook_remove(&metadata_listener);
      }

      metadata = static_cast<pw_metadata*>(pw_registry_bind(registry, id, type, PW_VERSION_METADATA, 0));

      if (metadata != nullptr) {
        pw_metadata_add_listener(metadata, &metadata_listener, &metadata_events, this);  // NOLINT

        util::debug("Successfully registered default metadata");

        return true;
      } else {
        util::warning("pw_registry_bind returned a null metadata object");

        return false;
      }
    }
  }

  return false;
}

auto MetadataManager::on_metadata_property(void* data,
                                           uint32_t id,
                                           const char* key,
                                           const char* type,
                                           const char* value) -> int {
  auto* const mm = static_cast<MetadataManager*>(data);

  const std::string str_key = (key != nullptr) ? key : "";
  const std::string str_value = (value != nullptr) ? value : "";
  const std::string str_type = (type != nullptr) ? type : "";

  util::debug(std::format("New metadata property: {}, {}, {}, {}", id, str_key, str_type, str_value));

  if (str_value.empty()) {
    return 0;
  }

  // Handle specific metadata properties
  if (str_key == "default.audio.sink") {
    auto v = nlohmann::json::parse(str_value).value("name", "");

    if (v == tags::pipewire::ee_sink_name) {
      return 0;
    }

    util::debug(std::format("New default output device: {}", v));

    Q_EMIT mm->defaultSinkChanged(QString::fromStdString(v));
  }

  if (str_key == "default.audio.source") {
    auto v = nlohmann::json::parse(str_value).value("name", "");

    if (v == tags::pipewire::ee_source_name) {
      return 0;
    }

    util::debug(std::format("New default input device: {}", v));

    Q_EMIT mm->defaultSourceChanged(QString::fromStdString(v));
  }

  return 0;
}

void MetadataManager::set_property(uint32_t id, const char* key, const char* type, const char* value) const {
  if (metadata == nullptr) {
    return;
  }

  pw_metadata_set_property(metadata, id, key, type, value);  // NOLINT
}

void MetadataManager::clear_property(uint32_t id, const char* key) const {
  if (metadata == nullptr) {
    return;
  }

  pw_metadata_set_property(metadata, id, key, nullptr, nullptr);  // NOLINT
}

void MetadataManager::destroy_metadata() {
  if (metadata != nullptr) {
    util::debug("Destroying PipeWire metadata...");

    pw_proxy_destroy((struct pw_proxy*)metadata);
  }
}

}  // namespace pw
