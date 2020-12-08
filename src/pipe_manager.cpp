/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "pipe_manager.hpp"
#include <glibmm.h>
#include <memory>
#include <string>
#include "pipewire/device.h"
#include "pipewire/keys.h"
#include "util.hpp"

namespace {

struct proxy_data {
  pw_proxy* proxy = nullptr;

  spa_hook proxy_listener{};

  spa_hook object_listener{};

  PipeManager* pm = nullptr;

  int id;

  std::string type;

  std::string name;

  std::string description;

  std::string media_class;

  int priority = -1;
};

void removed_proxy(void* data) {
  auto* pd = static_cast<proxy_data*>(data);

  pw_proxy_destroy(pd->proxy);

  pd->pm->list_nodes.erase(
      std::remove_if(pd->pm->list_nodes.begin(), pd->pm->list_nodes.end(), [=](auto& n) { return n.id == pd->id; }),
      pd->pm->list_nodes.end());

  util::debug(pd->pm->log_tag + pd->type + " " + pd->name + " was removed");
}

void destroy_proxy(void* data) {
  auto* pd = static_cast<proxy_data*>(data);

  spa_hook_remove(&pd->proxy_listener);
  spa_hook_remove(&pd->object_listener);
}

const struct pw_proxy_events proxy_events = {PW_VERSION_PROXY_EVENTS, .destroy = destroy_proxy,
                                             .removed = removed_proxy};

void on_node_info(void* object, const struct pw_node_info* info) {
  auto* pd = static_cast<proxy_data*>(object);

  for (auto& n : pd->pm->list_nodes) {
    if (n.id == info->id) {
      n.state = info->state;
      n.n_input_ports = info->n_input_ports;
      n.n_output_ports = info->n_output_ports;

      std::cout << "updating node: " << pd->name << std::endl;

      break;
    }
  }

  // const struct spa_dict_item* item = nullptr;
  // spa_dict_for_each(item, info->props) printf("\t\t%s: \"%s\"\n", item->key, item->value);
}

const struct pw_node_events node_events = {
    PW_VERSION_NODE_EVENTS,
    .info = on_node_info,
};

void on_registry_global(void* data,
                        uint32_t id,
                        uint32_t permissions,
                        const char* type,
                        uint32_t version,
                        const struct spa_dict* props) {
  auto* pm = static_cast<PipeManager*>(data);
  const void* events = nullptr;
  uint32_t client_version = 0;
  bool listen = false;
  std::string name = "none";
  std::string description = "none";
  std::string media_class = "none";
  int priority = -1;

  if (strcmp(type, PW_TYPE_INTERFACE_Node) == 0) {
    const auto* node_media_class = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
    const auto* node_name = spa_dict_lookup(props, PW_KEY_NODE_NAME);
    const auto* node_description = spa_dict_lookup(props, PW_KEY_NODE_DESCRIPTION);
    const auto* prio_session = spa_dict_lookup(props, PW_KEY_PRIORITY_SESSION);

    if (node_media_class != nullptr) {
      media_class = node_media_class;

      if (media_class == "Audio/Sink" || media_class == "Audio/Source" || media_class == "Stream/Output/Audio" ||
          media_class == "Stream/Input/Audio") {
        listen = true;

        if (node_name != nullptr) {
          name = node_name;
        }

        if (node_description != nullptr) {
          description = node_description;
        }

        if (prio_session != nullptr) {
          priority = std::atoi(prio_session);
        }

        client_version = PW_VERSION_NODE;
        events = &node_events;
      }
    }
  }

  if (strcmp(type, PW_TYPE_INTERFACE_Port) == 0) {
  }

  if (listen) {
    proxy_data* pd = nullptr;
    pw_proxy* proxy = nullptr;

    proxy = static_cast<pw_proxy*>(pw_registry_bind(pm->registry, id, type, client_version, sizeof(struct proxy_data)));

    pd = static_cast<proxy_data*>(pw_proxy_get_user_data(proxy));

    pd->proxy = proxy;
    pd->pm = pm;
    pd->id = id;
    pd->type = type;
    pd->name = name;
    pd->description = description;
    pd->media_class = media_class;
    pd->priority = priority;

    pw_proxy_add_object_listener(proxy, &pd->object_listener, events, pd);
    pw_proxy_add_listener(proxy, &pd->proxy_listener, &proxy_events, pd);

    for (const auto& n : pm->list_nodes) {
      if (n.id == id) {
        return;  // do not add the same node two times
      }
    }

    NodeInfo nd_info;

    nd_info.id = id;
    nd_info.type = type;
    nd_info.media_class = media_class;
    nd_info.name = name;
    nd_info.description = description;
    nd_info.priority = priority;

    pm->list_nodes.emplace_back(nd_info);

    util::debug(pm->log_tag + media_class + " " + name + " was added");
  }
}

void on_core_error(void* data, uint32_t id, int seq, int res, const char* message) {
  auto* pm = static_cast<PipeManager*>(data);

  util::warning(pm->log_tag + "Remote error on id:" + std::to_string(id));
  util::warning(pm->log_tag + "Remote error message:" + message);
}

void on_core_info(void* data, const struct pw_core_info* info) {
  auto* pm = static_cast<PipeManager*>(data);

  util::debug(pm->log_tag + "core version: " + info->version);
  util::debug(pm->log_tag + "core name: " + info->name);
}

void on_core_done(void* data, uint32_t id, int seq) {
  auto* pm = static_cast<PipeManager*>(data);

  util::debug(pm->log_tag + "connected to the core");

  pw_thread_loop_signal(pm->thread_loop, false);
}

const struct pw_core_events core_events = {.version = PW_VERSION_CORE_EVENTS,
                                           .info = on_core_info,
                                           .done = on_core_done,
                                           .error = on_core_error};

const struct pw_registry_events registry_events = {
    PW_VERSION_REGISTRY_EVENTS,
    .global = on_registry_global,
};

}  // namespace

PipeManager::PipeManager() {
  pw_init(nullptr, nullptr);

  spa_zero(core_listener);
  spa_zero(registry_listener);

  util::debug(log_tag + "compiled with pipewire: " + pw_get_headers_version());
  util::debug(log_tag + "linked to pipewire: " + pw_get_library_version());

  thread_loop = pw_thread_loop_new("pipewire-thread", nullptr);

  if (thread_loop == nullptr) {
    util::error(log_tag + "could not create pipewire loop");
  }

  if (pw_thread_loop_start(thread_loop) != 0) {
    util::error(log_tag + "could not start the loop");
  }

  pw_thread_loop_lock(thread_loop);

  context = pw_context_new(pw_thread_loop_get_loop(thread_loop), nullptr, 0);

  if (context == nullptr) {
    util::error(log_tag + "could not create pipewire context");
  }

  core = pw_context_connect(context, nullptr, 0);

  if (core == nullptr) {
    util::error(log_tag + "context connection failed");
  }

  registry = pw_core_get_registry(core, PW_VERSION_REGISTRY, 0);

  if (registry == nullptr) {
    util::error(log_tag + "could not get the registry");
  }

  pw_registry_add_listener(registry, &registry_listener, &registry_events, this);

  pw_core_add_listener(core, &core_listener, &core_events, this);

  pw_core_sync(core, PW_ID_CORE, 0);

  pw_thread_loop_wait(thread_loop);

  //   get_server_info();
  //   load_apps_sink();
  //   load_mic_sink();

  pw_thread_loop_unlock(thread_loop);
}

PipeManager::~PipeManager() {
  pw_thread_loop_lock(thread_loop);

  spa_hook_remove(&registry_listener);
  spa_hook_remove(&core_listener);

  util::debug(log_tag + "Destroying Pipewire registry...");
  pw_proxy_destroy((struct pw_proxy*)registry);

  util::debug(log_tag + "Disconnecting Pipewire core...");
  pw_core_disconnect(core);

  util::debug(log_tag + "Destroying Pipewire context...");
  pw_context_destroy(context);

  pw_thread_loop_unlock(thread_loop);

  util::debug(log_tag + "Destroying Pipewire loop...");
  pw_thread_loop_destroy(thread_loop);
}

auto PipeManager::get_sink_info(const std::string& name) -> std::shared_ptr<mySinkInfo> {
  auto si = std::make_shared<mySinkInfo>();

  // struct Data {
  //   bool failed;
  //   PipeManager* pm;
  //   std::shared_ptr<mySinkInfo> si;
  // };

  // Data data = {false, this, si};

  // pa_threaded_mainloop_lock(thread_loop);

  // auto* o = pw_context_get_sink_info_by_name(
  //     context, name.c_str(),
  //     [](auto c, auto info, auto eol, auto data) {
  //       auto* d = static_cast<Data*>(data);

  //       if (eol < 0) {
  //         d->failed = true;
  //         pa_threaded_mainloop_signal(d->pm->thread_loop, false);
  //       } else if (eol > 0) {
  //         pa_threaded_mainloop_signal(d->pm->thread_loop, false);
  //       } else if (info != nullptr) {
  //         d->si->name = info->name;
  //         d->si->index = info->index;
  //         d->si->description = info->description;
  //         d->si->owner_module = info->owner_module;
  //         d->si->monitor_source = info->monitor_source;
  //         d->si->monitor_source_name = info->monitor_source_name;
  //         d->si->rate = info->sample_spec.rate;
  //         d->si->format = pa_sample_format_to_string(info->sample_spec.format);

  //         if (info->active_port != nullptr) {
  //           d->si->active_port = info->active_port->name;
  //         } else {
  //           d->si->active_port = "null";
  //         }
  //       }
  //     },
  //     &data);

  // if (o != nullptr) {
  //   while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
  //     pa_threaded_mainloop_wait(thread_loop);
  //   }

  //   pa_operation_unref(o);
  // } else {
  //   util::critical(log_tag + " failed to get sink info: " + name);
  // }

  // pa_threaded_mainloop_unlock(thread_loop);

  // if (!data.failed) {
  //   return si;
  // }

  // util::debug(log_tag + " failed to get sink info: " + name);

  return nullptr;
}

auto PipeManager::get_source_info(const std::string& name) -> std::shared_ptr<mySourceInfo> {
  // auto si = std::make_shared<mySourceInfo>();

  // struct Data {
  //   bool failed;
  //   PipeManager* pm;
  //   std::shared_ptr<mySourceInfo> si;
  // };

  // Data data = {false, this, si};

  // pa_threaded_mainloop_lock(thread_loop);

  // auto* o = pw_context_get_source_info_by_name(
  //     context, name.c_str(),
  //     [](auto c, auto info, auto eol, auto data) {
  //       auto* d = static_cast<Data*>(data);

  //       if (eol < 0) {
  //         d->failed = true;
  //         pa_threaded_mainloop_signal(d->pm->thread_loop, false);
  //       } else if (eol > 0) {
  //         pa_threaded_mainloop_signal(d->pm->thread_loop, false);
  //       } else if (info != nullptr) {
  //         d->si->name = info->name;
  //         d->si->index = info->index;
  //         d->si->description = info->description;
  //         d->si->rate = info->sample_spec.rate;
  //         d->si->format = pa_sample_format_to_string(info->sample_spec.format);

  //         if (info->active_port != nullptr) {
  //           d->si->active_port = info->active_port->name;
  //         } else {
  //           d->si->active_port = "null";
  //         }
  //       }
  //     },
  //     &data);

  // if (o != nullptr) {
  //   while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
  //     pa_threaded_mainloop_wait(thread_loop);
  //   }

  //   pa_operation_unref(o);
  // } else {
  //   util::critical(log_tag + " failed to get source info:" + name);
  // }

  // pa_threaded_mainloop_unlock(thread_loop);

  // if (!data.failed) {
  //   return si;
  // }

  // util::debug(log_tag + " failed to get source info:" + name);

  return nullptr;
}

void PipeManager::find_sink_inputs() {
  // pa_threaded_mainloop_lock(thread_loop);

  // auto* o = pw_context_get_sink_input_info_list(
  //     context,
  //     [](auto c, auto info, auto eol, auto d) {
  //       auto* pm = static_cast<PipeManager*>(d);

  //       if (info != nullptr) {
  //         pm->new_app(info);
  //       } else {
  //         pa_threaded_mainloop_signal(pm->thread_loop, false);
  //       }
  //     },
  //     this);

  // if (o != nullptr) {
  //   while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
  //     pa_threaded_mainloop_wait(thread_loop);
  //   }

  //   pa_operation_unref(o);
  // } else {
  //   util::warning(log_tag + " failed to find sink inputs");
  // }

  // pa_threaded_mainloop_unlock(thread_loop);
}

void PipeManager::find_source_outputs() {
  // pa_threaded_mainloop_lock(thread_loop);

  // auto* o = pw_context_get_source_output_info_list(
  //     context,
  //     [](auto c, auto info, auto eol, auto d) {
  //       auto* pm = static_cast<PipeManager*>(d);

  //       if (info != nullptr) {
  //         pm->new_app(info);
  //       } else {
  //         pa_threaded_mainloop_signal(pm->thread_loop, false);
  //       }
  //     },
  //     this);

  // if (o != nullptr) {
  //   while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
  //     pa_threaded_mainloop_wait(thread_loop);
  //   }

  //   pa_operation_unref(o);
  // } else {
  //   util::warning(log_tag + " failed to find source outputs");
  // }

  // pa_threaded_mainloop_unlock(thread_loop);
}

void PipeManager::find_sinks() {
  // pa_threaded_mainloop_lock(thread_loop);

  // auto* o = pw_context_get_sink_info_list(
  //     context,
  //     [](auto c, auto info, auto eol, auto d) {
  //       auto* pm = static_cast<PipeManager*>(d);

  //       if (info != nullptr) {
  //         std::string s1 = "PulseEffects_apps";
  //         std::string s2 = "PulseEffects_mic";

  //         if (info->name != s1 && info->name != s2) {
  //           auto si = std::make_shared<mySinkInfo>();

  //           si->name = info->name;
  //           si->index = info->index;
  //           si->description = info->description;
  //           si->rate = info->sample_spec.rate;
  //           si->format = pa_sample_format_to_string(info->sample_spec.format);

  //           Glib::signal_idle().connect_once([pm, si = move(si)] { pm->sink_added.emit(si); });
  //         }
  //       } else {
  //         pa_threaded_mainloop_signal(pm->thread_loop, false);
  //       }
  //     },
  //     this);

  // if (o != nullptr) {
  //   while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
  //     pa_threaded_mainloop_wait(thread_loop);
  //   }

  //   pa_operation_unref(o);
  // } else {
  //   util::warning(log_tag + " failed to find sinks");
  // }

  // pa_threaded_mainloop_unlock(thread_loop);
}

void PipeManager::find_sources() {
  // pa_threaded_mainloop_lock(thread_loop);

  // auto* o = pw_context_get_source_info_list(
  //     context,
  //     [](auto c, auto info, auto eol, auto d) {
  //       auto* pm = static_cast<PipeManager*>(d);

  //       if (info != nullptr) {
  //         std::string s1 = "PulseEffects_apps.monitor";
  //         std::string s2 = "PulseEffects_mic.monitor";

  //         if (info->name != s1 && info->name != s2) {
  //           auto si = std::make_shared<mySourceInfo>();

  //           si->name = info->name;
  //           si->index = info->index;
  //           si->description = info->description;
  //           si->rate = info->sample_spec.rate;
  //           si->format = pa_sample_format_to_string(info->sample_spec.format);

  //           Glib::signal_idle().connect_once([pm, si = move(si)] { pm->source_added.emit(si); });
  //         }
  //       } else {
  //         pa_threaded_mainloop_signal(pm->thread_loop, false);
  //       }
  //     },
  //     this);

  // if (o != nullptr) {
  //   while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
  //     pa_threaded_mainloop_wait(thread_loop);
  //   }

  //   pa_operation_unref(o);
  // } else {
  //   util::warning(log_tag + " failed to find sources");
  // }

  // pa_threaded_mainloop_unlock(thread_loop);
}

auto PipeManager::move_sink_input_to_pulseeffects(const std::string& name, uint idx) -> bool {
  // struct Data {
  //   std::string name;
  //   uint idx;
  //   PipeManager* pm;
  // };

  // Data data = {name, idx, this};

  // bool added_successfully = false;

  // pa_threaded_mainloop_lock(thread_loop);

  // auto* o = pw_context_move_sink_input_by_index(
  //     context, idx, apps_sink_info->index,
  //     [](auto c, auto success, auto data) {
  //       auto* d = static_cast<Data*>(data);

  //       if (success) {
  //         util::debug(d->pm->log_tag + "sink input: " + d->name + ", idx = " + std::to_string(d->idx) + " moved to
  //         PE");
  //       } else {
  //         util::critical(d->pm->log_tag + "failed to move sink input: " + d->name +
  //                        ", idx = " + std::to_string(d->idx) + " to PE");
  //       }

  //       pa_threaded_mainloop_signal(d->pm->thread_loop, false);
  //     },
  //     &data);

  // if (o != nullptr) {
  //   while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
  //     pa_threaded_mainloop_wait(thread_loop);
  //   }

  //   pa_operation_unref(o);

  //   added_successfully = true;
  // } else {
  //   util::critical(log_tag + "failed to move sink input: " + name + ", idx = " + std::to_string(idx) + " to PE");
  // }

  // pa_threaded_mainloop_unlock(thread_loop);

  // return added_successfully;
}

auto PipeManager::remove_sink_input_from_pulseeffects(const std::string& name, uint idx) -> bool {
  // struct Data {
  //   std::string name;
  //   uint idx;
  //   PipeManager* pm;
  // };

  // Data data = {name, idx, this};

  // bool removed_successfully = false;

  // pa_threaded_mainloop_lock(thread_loop);

  // auto* o = pw_context_move_sink_input_by_name(
  //     context, idx, server_info.default_sink_name.c_str(),
  //     [](auto c, auto success, auto data) {
  //       auto* d = static_cast<Data*>(data);

  //       if (success) {
  //         util::debug(d->pm->log_tag + "sink input: " + d->name + ", idx = " + std::to_string(d->idx) +
  //                     " removed from PE");
  //       } else {
  //         util::critical(d->pm->log_tag + "failed to remove sink input: " + d->name +
  //                        ", idx = " + std::to_string(d->idx) + " from PE");
  //       }

  //       pa_threaded_mainloop_signal(d->pm->thread_loop, false);
  //     },
  //     &data);

  // if (o != nullptr) {
  //   while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
  //     pa_threaded_mainloop_wait(thread_loop);
  //   }

  //   pa_operation_unref(o);

  //   removed_successfully = true;
  // } else {
  //   util::critical(log_tag + "failed to remove sink input: " + name + ", idx = " + std::to_string(idx) + " from PE");
  // }

  // pa_threaded_mainloop_unlock(thread_loop);

  // return removed_successfully;
}

auto PipeManager::move_source_output_to_pulseeffects(const std::string& name, uint idx) -> bool {
  // struct Data {
  //   std::string name;
  //   uint idx;
  //   PipeManager* pm;
  // };

  // Data data = {name, idx, this};

  // bool added_successfully = false;

  // pa_threaded_mainloop_lock(thread_loop);

  // auto* o = pw_context_move_source_output_by_index(
  //     context, idx, mic_sink_info->monitor_source,
  //     [](auto c, auto success, auto data) {
  //       auto* d = static_cast<Data*>(data);

  //       if (success) {
  //         util::debug(d->pm->log_tag + "source output: " + d->name + ", idx = " + std::to_string(d->idx) +
  //                     " moved to PE");
  //       } else {
  //         util::critical(d->pm->log_tag + "failed to move source output " + d->name + ", idx = " + " to PE");
  //       }

  //       pa_threaded_mainloop_signal(d->pm->thread_loop, false);
  //     },
  //     &data);

  // if (o != nullptr) {
  //   while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
  //     pa_threaded_mainloop_wait(thread_loop);
  //   }

  //   pa_operation_unref(o);

  //   added_successfully = true;
  // } else {
  //   util::critical(log_tag + "failed to move source output: " + name + ", idx = " + std::to_string(idx) + " to PE");
  // }

  // pa_threaded_mainloop_unlock(thread_loop);

  // return added_successfully;
}

auto PipeManager::remove_source_output_from_pulseeffects(const std::string& name, uint idx) -> bool {
  // struct Data {
  //   std::string name;
  //   uint idx;
  //   PipeManager* pm;
  // };

  // Data data = {name, idx, this};

  // bool removed_successfully = false;

  // pa_threaded_mainloop_lock(thread_loop);

  // auto* o = pw_context_move_source_output_by_name(
  //     context, idx, server_info.default_source_name.c_str(),
  //     [](auto c, auto success, auto data) {
  //       auto* d = static_cast<Data*>(data);

  //       if (success) {
  //         util::debug(d->pm->log_tag + "source output: " + d->name + ", idx = " + " removed from PE");
  //       } else {
  //         util::critical(d->pm->log_tag + "failed to remove source output: " + d->name + ", idx = " + " from PE");
  //       }

  //       pa_threaded_mainloop_signal(d->pm->thread_loop, false);
  //     },
  //     &data);

  // if (o != nullptr) {
  //   while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
  //     pa_threaded_mainloop_wait(thread_loop);
  //   }

  //   pa_operation_unref(o);

  //   removed_successfully = true;
  // } else {
  //   util::critical(log_tag + "failed to remove source output: " + name + ", idx = " + std::to_string(idx) + " from
  //   PE");
  // }

  // pa_threaded_mainloop_unlock(thread_loop);

  // return removed_successfully;
}

void PipeManager::set_sink_input_volume(const std::string& name, uint idx, uint8_t channels, uint value) {
  // pa_volume_t raw_value = PA_VOLUME_NORM * value / 100.0;

  // auto cvol = pa_cvolume();

  // auto* cvol_ptr = pa_cvolume_set(&cvol, channels, raw_value);

  // if (cvol_ptr != nullptr) {
  //   struct Data {
  //     std::string name;
  //     uint idx;
  //     PipeManager* pm;
  //   };

  //   Data data = {name, idx, this};

  //   pa_threaded_mainloop_lock(thread_loop);

  //   auto* o = pw_context_set_sink_input_volume(
  //       context, idx, cvol_ptr,
  //       [](auto c, auto success, auto data) {
  //         auto* d = static_cast<Data*>(data);

  //         if (success) {
  //           util::debug(d->pm->log_tag + "changed volume of sink input: " + d->name +
  //                       ", idx = " + std::to_string(d->idx));
  //         } else {
  //           util::critical(d->pm->log_tag + "failed to change volume of sink input: " + d->name +
  //                          ", idx = " + std::to_string(d->idx));
  //         }

  //         pa_threaded_mainloop_signal(d->pm->thread_loop, false);
  //       },
  //       &data);

  //   if (o != nullptr) {
  //     while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
  //       pa_threaded_mainloop_wait(thread_loop);
  //     }

  //     pa_operation_unref(o);

  //     pa_threaded_mainloop_unlock(thread_loop);
  //   } else {
  //     util::warning(log_tag + "failed to change volume of sink input: " + name + ", idx = " + std::to_string(idx));

  //     pa_threaded_mainloop_unlock(thread_loop);
  //   }
  // }
}

void PipeManager::set_sink_input_mute(const std::string& name, uint idx, bool state) {
  // struct Data {
  //   std::string name;
  //   uint idx;
  //   PipeManager* pm;
  // };

  // Data data = {name, idx, this};

  // pa_threaded_mainloop_lock(thread_loop);

  // auto* o = pw_context_set_sink_input_mute(
  //     context, idx, static_cast<int>(state),
  //     [](auto c, auto success, auto data) {
  //       auto* d = static_cast<Data*>(data);

  //       if (success) {
  //         util::debug(d->pm->log_tag + "sink input: " + d->name + ", idx = " + std::to_string(d->idx) + " is muted");
  //       } else {
  //         util::critical(d->pm->log_tag + "failed to mute sink input: " + d->name +
  //                        ", idx = " + std::to_string(d->idx));
  //       }

  //       pa_threaded_mainloop_signal(d->pm->thread_loop, false);
  //     },
  //     &data);

  // if (o != nullptr) {
  //   while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
  //     pa_threaded_mainloop_wait(thread_loop);
  //   }

  //   pa_operation_unref(o);
  // } else {
  //   util::warning(log_tag + "failed to mute set sink input: " + name + ", idx = " + std::to_string(idx) + " to PE");
  // }

  // pa_threaded_mainloop_unlock(thread_loop);
}

void PipeManager::set_source_output_volume(const std::string& name, uint idx, uint8_t channels, uint value) {
  // pa_volume_t raw_value = PA_VOLUME_NORM * value / 100.0;

  // auto cvol = pa_cvolume();

  // auto* cvol_ptr = pa_cvolume_set(&cvol, channels, raw_value);

  // if (cvol_ptr != nullptr) {
  //   struct Data {
  //     std::string name;
  //     uint idx;
  //     PipeManager* pm;
  //   };

  //   Data data = {name, idx, this};

  //   pa_threaded_mainloop_lock(thread_loop);

  //   auto* o = pw_context_set_source_output_volume(
  //       context, idx, cvol_ptr,
  //       [](auto c, auto success, auto data) {
  //         auto* d = static_cast<Data*>(data);

  //         if (success) {
  //           util::debug(d->pm->log_tag + "changed volume of source output: " + d->name +
  //                       ", idx = " + std::to_string(d->idx));
  //         } else {
  //           util::debug(d->pm->log_tag + "failed to change volume of source output: " + d->name +
  //                       ", idx = " + std::to_string(d->idx));
  //         }

  //         pa_threaded_mainloop_signal(d->pm->thread_loop, false);
  //       },
  //       &data);

  //   if (o != nullptr) {
  //     while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
  //       pa_threaded_mainloop_wait(thread_loop);
  //     }

  //     pa_operation_unref(o);

  //     pa_threaded_mainloop_unlock(thread_loop);
  //   } else {
  //     util::warning(log_tag + "failed to change volume of source output: " + name + ", idx = " +
  //     std::to_string(idx));

  //     pa_threaded_mainloop_unlock(thread_loop);
  //   }
  // }
}

void PipeManager::set_sink_volume_by_name(const std::string& name, uint8_t channels, uint value) {
  // pa_volume_t raw_value = PA_VOLUME_NORM * value / 100.0;

  // auto cvol = pa_cvolume();

  // auto* cvol_ptr = pa_cvolume_set(&cvol, channels, raw_value);

  // if (cvol_ptr != nullptr) {
  //   struct Data {
  //     std::string name;
  //     PipeManager* pm;
  //   };

  //   Data data = {name, this};

  //   pa_threaded_mainloop_lock(thread_loop);

  //   auto* o = pw_context_set_sink_volume_by_name(
  //       context, name.c_str(), cvol_ptr,
  //       [](auto c, auto success, auto data) {
  //         auto* d = static_cast<Data*>(data);

  //         if (success) {
  //           util::debug(d->pm->log_tag + "changed volume of the sink: " + d->name);
  //         } else {
  //           util::debug(d->pm->log_tag + "failed to change volume of the sink: " + d->name);
  //         }

  //         pa_threaded_mainloop_signal(d->pm->thread_loop, false);
  //       },
  //       &data);

  //   if (o != nullptr) {
  //     while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
  //       pa_threaded_mainloop_wait(thread_loop);
  //     }

  //     pa_operation_unref(o);

  //     pa_threaded_mainloop_unlock(thread_loop);
  //   } else {
  //     util::warning(log_tag + "failed to change volume of the sink: " + name);

  //     pa_threaded_mainloop_unlock(thread_loop);
  //   }
  // }
}

void PipeManager::set_source_output_mute(const std::string& name, uint idx, bool state) {
  // struct Data {
  //   std::string name;
  //   uint idx;
  //   PipeManager* pm;
  // };

  // Data data = {name, idx, this};

  // pa_threaded_mainloop_lock(thread_loop);

  // auto* o = pw_context_set_source_output_mute(
  //     context, idx, static_cast<int>(state),
  //     [](auto c, auto success, auto data) {
  //       auto* d = static_cast<Data*>(data);

  //       if (success) {
  //         util::debug(d->pm->log_tag + "source output: " + d->name + ", idx = " + std::to_string(d->idx) + " is
  //         muted");
  //       } else {
  //         util::critical(d->pm->log_tag + "failed to mute source output: " + d->name +
  //                        ", idx = " + std::to_string(d->idx));
  //       }

  //       pa_threaded_mainloop_signal(d->pm->thread_loop, false);
  //     },
  //     &data);

  // if (o != nullptr) {
  //   while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
  //     pa_threaded_mainloop_wait(thread_loop);
  //   }

  //   pa_operation_unref(o);
  // } else {
  //   util::warning(log_tag + "failed to mute source output: " + name + ", idx = " + std::to_string(idx) + " to PE");
  // }

  // pa_threaded_mainloop_unlock(thread_loop);
}

void PipeManager::get_sink_input_info(uint idx) {
  // pa_threaded_mainloop_lock(thread_loop);

  // auto* o = pw_context_get_sink_input_info(
  //     context, idx,
  //     [](auto c, auto info, auto eol, auto d) {
  //       auto* pm = static_cast<PipeManager*>(d);

  //       if (info != nullptr) {
  //         pm->changed_app(info);
  //       } else {
  //         pa_threaded_mainloop_signal(pm->thread_loop, false);
  //       }
  //     },
  //     this);

  // if (o != nullptr) {
  //   while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
  //     pa_threaded_mainloop_wait(thread_loop);
  //   }

  //   pa_operation_unref(o);
  // } else {
  //   util::critical(log_tag + "failed to get sink input info: " + std::to_string(idx));
  // }

  // pa_threaded_mainloop_unlock(thread_loop);
}

void PipeManager::get_modules_info() {
  // pa_threaded_mainloop_lock(thread_loop);

  // auto* o = pw_context_get_module_info_list(
  //     context,
  //     [](auto c, auto info, auto eol, auto d) {
  //       auto* pm = static_cast<PipeManager*>(d);

  //       if (info != nullptr) {
  //         auto mi = std::make_shared<myModuleInfo>();

  //         if (info->name) {
  //           mi->name = info->name;
  //           mi->index = info->index;

  //           if (info->argument) {
  //             mi->argument = info->argument;
  //           } else {
  //             mi->argument = "";
  //           }

  //           Glib::signal_idle().connect_once([pm, mi = move(mi)] { pm->module_info.emit(mi); });
  //         }
  //       } else {
  //         pa_threaded_mainloop_signal(pm->thread_loop, false);
  //       }
  //     },
  //     this);

  // if (o != nullptr) {
  //   while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
  //     pa_threaded_mainloop_wait(thread_loop);
  //   }

  //   pa_operation_unref(o);
  // } else {
  //   util::critical(log_tag + "failed to get modules info");
  // }

  // pa_threaded_mainloop_unlock(thread_loop);
}

void PipeManager::get_clients_info() {
  // pa_threaded_mainloop_lock(thread_loop);

  // auto* o = pw_context_get_client_info_list(
  //     context,
  //     [](auto c, auto info, auto eol, auto d) {
  //       auto* pm = static_cast<PipeManager*>(d);

  //       if (info != nullptr) {
  //         auto mi = std::make_shared<myClientInfo>();

  //         if (info->name) {
  //           mi->name = info->name;
  //           mi->index = info->index;

  //           if (pa_proplist_contains(info->proplist, "application.process.binary") == 1) {
  //             mi->binary = pa_proplist_gets(info->proplist, "application.process.binary");
  //           } else {
  //             mi->binary = "";
  //           }

  //           Glib::signal_idle().connect_once([pm, mi = move(mi)] { pm->client_info.emit(mi); });
  //         }
  //       } else {
  //         pa_threaded_mainloop_signal(pm->thread_loop, false);
  //       }
  //     },
  //     this);

  // if (o != nullptr) {
  //   while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
  //     pa_threaded_mainloop_wait(thread_loop);
  //   }

  //   pa_operation_unref(o);
  // } else {
  //   util::critical(log_tag + "failed to get clients info");
  // }

  // pa_threaded_mainloop_unlock(thread_loop);
}

// void PipeManager::new_app(const pa_sink_input_info* info) {
//   auto app_info = parse_app_info(info);

//   if (app_info != nullptr) {
//     app_info->app_type = "sink_input";

//     Glib::signal_idle().connect_once([&, app_info = move(app_info)]() { sink_input_added.emit(app_info); });
//   }
// }

// void PipeManager::new_app(const pa_source_output_info* info) {
//   auto app_info = parse_app_info(info);

//   if (app_info != nullptr) {
//     app_info->app_type = "source_output";

//     Glib::signal_idle().connect_once([&, app_info = move(app_info)]() { source_output_added.emit(app_info); });
//   }
// }

// void PipeManager::changed_app(const pa_sink_input_info* info) {
//   auto app_info = parse_app_info(info);

//   if (app_info != nullptr) {
//     // checking if the user blocklisted this app

//     auto forbidden_app =
//         std::find(std::begin(blocklist_out), std::end(blocklist_out), app_info->name) != std::end(blocklist_out);

//     if (!forbidden_app) {
//       app_info->app_type = "sink_input";

//       Glib::signal_idle().connect_once([&, app_info = move(app_info)]() { sink_input_changed.emit(app_info); });
//     }
//   }
// }

// void PipeManager::changed_app(const pa_source_output_info* info) {
//   auto app_info = parse_app_info(info);

//   if (app_info != nullptr) {
//     // checking if the user blocklisted this app

//     auto forbidden_app =
//         std::find(std::begin(blocklist_in), std::end(blocklist_in), app_info->name) != std::end(blocklist_in);

//     if (!forbidden_app) {
//       app_info->app_type = "source_output";

//       Glib::signal_idle().connect_once([&, app_info = move(app_info)]() { source_output_changed.emit(app_info); });
//     }
//   }
// }

void PipeManager::print_app_info(const std::shared_ptr<AppInfo>& info) {
  std::cout << "index: " << info->index << std::endl;
  std::cout << "name: " << info->name << std::endl;
  std::cout << "icon name: " << info->icon_name << std::endl;
  std::cout << "channels: " << info->channels << std::endl;
  std::cout << "volume: " << info->volume << std::endl;
  std::cout << "rate: " << info->rate << std::endl;
  std::cout << "resampler: " << info->resampler << std::endl;
  std::cout << "format: " << info->format << std::endl;
  std::cout << "wants to play: " << info->wants_to_play << std::endl;
}

// auto PipeManager::app_is_connected(const pa_sink_input_info* info) -> bool {
//   return info->sink == apps_sink_info->index;
// }

// auto PipeManager::app_is_connected(const pa_source_output_info* info) -> bool {
//   return info->source == mic_sink_info->monitor_source;
// }
