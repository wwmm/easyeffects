/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef INFO_HOLDERS_HPP
#define INFO_HOLDERS_HPP

#include <glibmm.h>
#include "pipe_manager.hpp"

class NodeInfoHolder : public Glib::Object {
 public:
  NodeInfo info;

  static auto create(const NodeInfo& info) -> Glib::RefPtr<NodeInfoHolder>;

  sigc::signal<void(NodeInfo)> info_updated;

 protected:
  NodeInfoHolder(NodeInfo info);
};

class ModuleInfoHolder : public Glib::Object {
 public:
  ModuleInfo info;

  static auto create(const ModuleInfo& info) -> Glib::RefPtr<ModuleInfoHolder>;

  sigc::signal<void(ModuleInfo)> info_updated;

 protected:
  ModuleInfoHolder(ModuleInfo info);
};

class ClientInfoHolder : public Glib::Object {
 public:
  ClientInfo info;

  static auto create(const ClientInfo& info) -> Glib::RefPtr<ClientInfoHolder>;

  sigc::signal<void(ClientInfo)> info_updated;

 protected:
  ClientInfoHolder(ClientInfo info);
};

class PresetsAutoloadingHolder : public Glib::Object {
 public:
  std::string device;
  std::string device_profile;
  std::string preset_name;

  static auto create(const std::string& device, const std::string& device_profile, const std::string& preset_name)
      -> Glib::RefPtr<PresetsAutoloadingHolder>;

 protected:
  PresetsAutoloadingHolder(std::string device, std::string device_profile, std::string preset_name);
};

#endif
