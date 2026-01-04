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

#include <pipewire/core.h>
#include <pipewire/extensions/metadata.h>
#include <pipewire/proxy.h>
#include <pipewire/thread-loop.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <spa/utils/defs.h>
#include <spa/utils/hook.h>
#include <cstdint>

namespace pw {

class MetadataManager : public QObject {
  Q_OBJECT

 public:
  explicit MetadataManager() = default;
  ~MetadataManager() override = default;

  MetadataManager(const MetadataManager&) = delete;
  auto operator=(const MetadataManager&) -> MetadataManager& = delete;
  MetadataManager(const MetadataManager&&) = delete;
  auto operator=(const MetadataManager&&) -> MetadataManager& = delete;

  auto register_metadata(pw_registry* registry, uint32_t id, const char* type, const spa_dict* props) -> bool;

  void set_property(uint32_t id, const char* key, const char* type, const char* value) const;

  void clear_property(uint32_t id, const char* key) const;

  void destroy_metadata();

 Q_SIGNALS:
  void defaultSinkChanged(const QString& name);
  void defaultSourceChanged(const QString& name);

 private:
  pw_metadata* metadata = nullptr;

  spa_hook metadata_listener{};

  const struct pw_metadata_events metadata_events = {
      .version = PW_VERSION_METADATA_EVENTS,
      .property = on_metadata_property,
  };

  static auto on_metadata_property(void* data, uint32_t id, const char* key, const char* type, const char* value)
      -> int;
};

}  // namespace pw
