#include "info_holders.hpp"

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