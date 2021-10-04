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

#include "info_holders.hpp"

#include <utility>

NodeInfoHolder::NodeInfoHolder(NodeInfo info)
    : Glib::ObjectBase(typeid(NodeInfoHolder)),
      Glib::Object(),
      ts(info.timestamp),
      id(info.id),
      device_id(info.device_id),
      name(info.name),
      media_class(info.media_class) {}

auto NodeInfoHolder::create(NodeInfo info) -> Glib::RefPtr<NodeInfoHolder> {
  return Glib::make_refptr_for_instance<NodeInfoHolder>(new NodeInfoHolder(std::move(info)));
}

ModuleInfoHolder::ModuleInfoHolder(ModuleInfo info)
    : Glib::ObjectBase(typeid(ModuleInfoHolder)), Glib::Object(), info(std::move(info)) {}

auto ModuleInfoHolder::create(ModuleInfo info) -> Glib::RefPtr<ModuleInfoHolder> {
  return Glib::make_refptr_for_instance<ModuleInfoHolder>(new ModuleInfoHolder(std::move(info)));
}

ClientInfoHolder::ClientInfoHolder(ClientInfo info)
    : Glib::ObjectBase(typeid(ClientInfoHolder)), Glib::Object(), info(std::move(info)) {}

auto ClientInfoHolder::create(ClientInfo info) -> Glib::RefPtr<ClientInfoHolder> {
  return Glib::make_refptr_for_instance<ClientInfoHolder>(new ClientInfoHolder(std::move(info)));
}

PresetsAutoloadingHolder::PresetsAutoloadingHolder(std::string device,
                                                   std::string device_profile,
                                                   std::string preset_name)
    : Glib::ObjectBase(typeid(PresetsAutoloadingHolder)),
      Glib::Object(),
      device(std::move(device)),
      device_profile(std::move(device_profile)),
      preset_name(std::move(preset_name)) {}

auto PresetsAutoloadingHolder::create(const std::string& device,
                                      const std::string& device_profile,
                                      const std::string& preset_name) -> Glib::RefPtr<PresetsAutoloadingHolder> {
  return Glib::make_refptr_for_instance<PresetsAutoloadingHolder>(
      new PresetsAutoloadingHolder(device, device_profile, preset_name));
}
