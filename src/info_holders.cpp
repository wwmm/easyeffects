#include "info_holders.hpp"

#include <utility>

NodeInfoHolder::NodeInfoHolder(NodeInfo info)
    : Glib::ObjectBase(typeid(NodeInfoHolder)), Glib::Object(), info(std::move(info)) {}

auto NodeInfoHolder::create(const NodeInfo& info) -> Glib::RefPtr<NodeInfoHolder> {
  return Glib::make_refptr_for_instance<NodeInfoHolder>(new NodeInfoHolder(info));
}

ModuleInfoHolder::ModuleInfoHolder(ModuleInfo info)
    : Glib::ObjectBase(typeid(ModuleInfoHolder)), Glib::Object(), info(std::move(info)) {}

auto ModuleInfoHolder::create(const ModuleInfo& info) -> Glib::RefPtr<ModuleInfoHolder> {
  return Glib::make_refptr_for_instance<ModuleInfoHolder>(new ModuleInfoHolder(info));
}

ClientInfoHolder::ClientInfoHolder(ClientInfo info)
    : Glib::ObjectBase(typeid(ClientInfoHolder)), Glib::Object(), info(std::move(info)) {}

auto ClientInfoHolder::create(const ClientInfo& info) -> Glib::RefPtr<ClientInfoHolder> {
  return Glib::make_refptr_for_instance<ClientInfoHolder>(new ClientInfoHolder(info));
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