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
  std::string preset_name;

  static auto create(const std::string& device, const std::string& preset_name)
      -> Glib::RefPtr<PresetsAutoloadingHolder>;

 protected:
  PresetsAutoloadingHolder(std::string device, std::string preset_name);
};

#endif