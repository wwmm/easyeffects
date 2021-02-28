#include "lv2_wrapper.hpp"

namespace lv2 {

Lv2Wrapper::Lv2Wrapper(const std::string& plugin_uri) {
  world = lilv_world_new();

  if (world == nullptr) {
    util::error(log_tag + "failed to initialized the world");
  }

  auto* uri = lilv_new_uri(world, plugin_uri.c_str());

  if (uri == nullptr) {
    util::error(log_tag + "Invalid plugin URI: " + plugin_uri);
  }

  lilv_world_load_all(world);

  const LilvPlugins* plugins = lilv_world_get_all_plugins(world);

  plugin = lilv_plugins_get_by_uri(plugins, uri);

  lilv_node_free(uri);

  if (plugin == nullptr) {
    util::error(log_tag + "Could not find the plugin: " + plugin_uri);
  }
}

Lv2Wrapper::~Lv2Wrapper() {
  lilv_instance_free(instance);
  lilv_world_free(world);

  free(ports);
}

}  // namespace lv2