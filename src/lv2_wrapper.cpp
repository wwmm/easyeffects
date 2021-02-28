#include "lv2_wrapper.hpp"
#include <cmath>

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

  create_ports();
}

Lv2Wrapper::~Lv2Wrapper() {
  lilv_instance_free(instance);
  lilv_world_free(world);
}

void Lv2Wrapper::create_ports() {
  n_ports = lilv_plugin_get_num_ports(plugin);

  ports.resize(n_ports);

  //   util::warning("n ports: " + std::to_string(n_ports));

  // Get default values for all ports

  std::vector<float> values(n_ports);

  lilv_plugin_get_port_ranges_float(plugin, nullptr, nullptr, values.data());

  LilvNode* lv2_InputPort = lilv_new_uri(world, LV2_CORE__InputPort);
  LilvNode* lv2_OutputPort = lilv_new_uri(world, LV2_CORE__OutputPort);
  LilvNode* lv2_AudioPort = lilv_new_uri(world, LV2_CORE__AudioPort);
  LilvNode* lv2_ControlPort = lilv_new_uri(world, LV2_CORE__ControlPort);
  LilvNode* lv2_connectionOptional = lilv_new_uri(world, LV2_CORE__connectionOptional);

  for (uint n = 0; n < n_ports; n++) {
    auto port = ports[n];

    port.lilv_port = lilv_plugin_get_port_by_index(plugin, n);
    port.index = n;
    port.value = std::isnan(values[n]) ? 0.0F : values[n];
  }
}

}  // namespace lv2