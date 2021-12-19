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

#include "application.hpp"
#include "application_ui.hpp"

namespace app {

using namespace std::string_literals;

auto constexpr log_tag = "application: ";

G_DEFINE_TYPE(Application, application, ADW_TYPE_APPLICATION)

void hide_all_windows(GApplication* app) {
  auto* list = gtk_application_get_windows(GTK_APPLICATION(app));

  while (list != nullptr) {
    auto* window = list->data;
    auto* next = list->next;

    gtk_window_destroy(GTK_WINDOW(window));

    list = next;
  }
}

void update_bypass_state(Application* self) {
  const auto state = g_settings_get_boolean(self->settings, "bypass");

  self->soe->set_bypass(state != 0);
  self->sie->set_bypass(state != 0);

  util::info(log_tag + ((state) != 0 ? "enabling"s : "disabling"s) + " global bypass"s);
}

void on_startup(GApplication* gapp) {
  G_APPLICATION_CLASS(application_parent_class)->startup(gapp);

  auto* self = EE_APP(gapp);

  self->data = new Data();

  self->settings = g_settings_new("com.github.wwmm.easyeffects");
  self->sie_settings = g_settings_new("com.github.wwmm.easyeffects.streaminputs");
  self->soe_settings = g_settings_new("com.github.wwmm.easyeffects.streamoutputs");

  self->pm = new PipeManager();
  self->soe = new StreamOutputEffects(self->pm);
  self->sie = new StreamInputEffects(self->pm);

  if (self->presets_manager == nullptr) {
    self->presets_manager = new PresetsManager();
  }

  self->data->connections.push_back(self->pm->new_default_sink.connect([=](const NodeInfo node) {
    util::debug("new default output device: " + node.name);

    if (g_settings_get_boolean(self->soe_settings, "use-default-output-device") != 0) {
      /*
        Depending on the hardware headphones can cause a node recreation where the id and the name are kept.
        So we clear the key to force the callbacks to be called
      */

      g_settings_set_string(self->soe_settings, "output-device", "");
      g_settings_set_string(self->soe_settings, "output-device", node.name.c_str());
    }
  }));

  self->data->connections.push_back(self->pm->new_default_source.connect([=](const NodeInfo node) {
    util::debug("new default input device: " + node.name);

    if (g_settings_get_boolean(self->sie_settings, "use-default-input-device") != 0) {
      /*
        Depending on the hardware microphones can cause a node recreation where the id and the name are kept.
        So we clear the key to force the callbacks to be called
      */

      g_settings_set_string(self->sie_settings, "input-device", "");
      g_settings_set_string(self->sie_settings, "input-device", node.name.c_str());
    }
  }));

  self->data->connections.push_back(self->pm->device_input_route_changed.connect([=](const DeviceInfo device) {
    if (device.input_route_available == SPA_PARAM_AVAILABILITY_no) {
      return;
    }

    util::debug(log_tag + "device "s + device.name + " has changed its input route to: "s + device.input_route_name);

    NodeInfo target_node;

    for (const auto& [ts, node] : self->pm->node_map) {
      if (node.device_id == device.id && node.media_class == self->pm->media_class_source) {
        target_node = node;

        break;
      }
    }

    if (target_node.id != SPA_ID_INVALID) {
      auto name = util::gsettings_get_string(self->sie_settings, "input-device");

      if (target_node.name == name) {
        self->presets_manager->autoload(PresetType::input, target_node.name, device.input_route_name);
      } else {
        util::debug(log_tag + "input autoloading: the target node name does not match the input device name"s);
      }
    } else {
      util::debug(log_tag + "input autoloading: could not find the target node"s);
    }
  }));

  self->data->connections.push_back(self->pm->device_output_route_changed.connect([=](const DeviceInfo device) {
    if (device.output_route_available == SPA_PARAM_AVAILABILITY_no) {
      return;
    }

    util::debug(log_tag + "device "s + device.name + " has changed its output route to: "s + device.output_route_name);

    NodeInfo target_node;

    for (const auto& [ts, node] : self->pm->node_map) {
      target_node = node;

      if (node.device_id == device.id && node.media_class == self->pm->media_class_sink) {
        target_node = node;

        break;
      }
    }

    if (target_node.id != SPA_ID_INVALID) {
      auto name = util::gsettings_get_string(self->soe_settings, "output-device");

      if (target_node.name == name) {
        self->presets_manager->autoload(PresetType::output, target_node.name, device.output_route_name);
      } else {
        util::debug(log_tag + "output autoloading: the target node name does not match the output device name"s);
      }
    } else {
      util::debug(log_tag + "output autoloading: could not find the target node"s);
    }
  }));

  self->data->gconnections_soe.push_back(g_signal_connect(
      self->soe_settings, "changed::output-device", G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
        auto self = static_cast<Application*>(user_data);

        const auto name = util::gsettings_get_string(settings, key);

        if (name.empty()) {
          return;
        }

        uint device_id = SPA_ID_INVALID;

        for (const auto& [ts, node] : self->pm->node_map) {
          if (node.name == name) {
            device_id = node.device_id;

            break;
          }
        }

        if (device_id != SPA_ID_INVALID) {
          for (const auto& device : self->pm->list_devices) {
            if (device.id == device_id) {
              self->presets_manager->autoload(PresetType::output, name, device.output_route_name);

              break;
            }
          }
        }
      }),
      self));

  self->data->gconnections_sie.push_back(g_signal_connect(
      self->sie_settings, "changed::input-device", G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
        auto self = static_cast<Application*>(user_data);

        const auto name = util::gsettings_get_string(settings, key);

        if (name.empty()) {
          return;
        }

        uint device_id = SPA_ID_INVALID;

        for (const auto& [ts, node] : self->pm->node_map) {
          if (node.name == name) {
            device_id = node.device_id;

            break;
          }
        }

        if (device_id != SPA_ID_INVALID) {
          for (const auto& device : self->pm->list_devices) {
            if (device.id == device_id) {
              self->presets_manager->autoload(PresetType::input, name, device.input_route_name);

              break;
            }
          }
        }
      }),
      self));

  self->data->gconnections.push_back(g_signal_connect(
      self->settings, "changed::bypass", G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
        auto self = static_cast<Application*>(user_data);

        update_bypass_state(self);
      }),
      self));

  update_bypass_state(self);

  if ((g_application_get_flags(gapp) & G_APPLICATION_IS_SERVICE) != 0) {
    g_application_hold(gapp);
  }
}

void application_class_init(ApplicationClass* klass) {
  auto* application_class = G_APPLICATION_CLASS(klass);

  application_class->command_line = [](GApplication* gapp, GApplicationCommandLine* cmdline) {
    auto* self = EE_APP(gapp);
    auto* options = g_application_command_line_get_options_dict(cmdline);

    if (g_variant_dict_contains(options, "quit") != 0) {
      hide_all_windows(gapp);

      g_application_quit(G_APPLICATION(gapp));
    } else if (g_variant_dict_contains(options, "load-preset") != 0) {
      const char* name = nullptr;

      if (g_variant_dict_lookup(options, "load-preset", "&s", &name) != 0) {
        if (self->presets_manager->preset_file_exists(PresetType::input, name)) {
          self->presets_manager->load_preset_file(PresetType::input, name);
        }

        if (self->presets_manager->preset_file_exists(PresetType::output, name)) {
          self->presets_manager->load_preset_file(PresetType::output, name);
        }
      }
    } else if (g_variant_dict_contains(options, "reset") != 0) {
      g_settings_reset(self->settings, "");

      util::info(log_tag + "All settings were reset"s);
    } else if (g_variant_dict_contains(options, "hide-window") != 0) {
      hide_all_windows(gapp);

      util::info(log_tag + "Hiding the window..."s);
    } else if (g_variant_dict_contains(options, "bypass") != 0) {
      if (int bypass_arg = 2; g_variant_dict_lookup(options, "bypass", "i", &bypass_arg)) {
        if (bypass_arg == 1) {
          g_settings_set_boolean(self->settings, "bypass", 1);
        } else if (bypass_arg == 2) {
          g_settings_set_boolean(self->settings, "bypass", 0);
        }
      }
    } else {
      g_application_activate(gapp);
    }

    return G_APPLICATION_CLASS(application_parent_class)->command_line(gapp, cmdline);
  };

  application_class->handle_local_options = [](GApplication* gapp, GVariantDict* options) {
    if (options == nullptr) {
      return -1;
    }

    auto* self = EE_APP(gapp);

    if (self->presets_manager == nullptr) {
      self->presets_manager = new PresetsManager();
    }

    if (g_variant_dict_contains(options, "presets") != 0) {
      std::string list;

      for (const auto& name : self->presets_manager->get_names(PresetType::output)) {
        list += name + ",";
      }

      std::clog << _("Output Presets: ") + list << std::endl;

      list = "";

      for (const auto& name : self->presets_manager->get_names(PresetType::input)) {
        list += name + ",";
      }

      std::clog << _("Input Presets: ") + list << std::endl;

      return EXIT_SUCCESS;
    }

    if (g_variant_dict_contains(options, "bypass") != 0) {
      if (int bypass_arg = 2; g_variant_dict_lookup(options, "bypass", "i", &bypass_arg)) {
        if (bypass_arg == 3) {
          std::clog << g_settings_get_boolean(self->settings, "bypass") << std::endl;

          return EXIT_SUCCESS;
        }
      }
    }

    return -1;
  };

  application_class->startup = on_startup;

  application_class->activate = [](GApplication* gapp) {
    if (gtk_application_get_active_window(GTK_APPLICATION(gapp)) == nullptr) {
      G_APPLICATION_CLASS(application_parent_class)->activate(gapp);

      auto* window = ui::application_window::create(gapp);

      gtk_window_present(GTK_WINDOW(window));
    }
  };

  application_class->shutdown = [](GApplication* gapp) {
    G_APPLICATION_CLASS(application_parent_class)->shutdown(gapp);

    auto* self = EE_APP(gapp);

    for (auto& c : self->data->connections) {
      c.disconnect();
    }

    for (auto& handler_id : self->data->gconnections) {
      g_signal_handler_disconnect(self->settings, handler_id);
    }

    for (auto& handler_id : self->data->gconnections_sie) {
      g_signal_handler_disconnect(self->sie_settings, handler_id);
    }

    for (auto& handler_id : self->data->gconnections_soe) {
      g_signal_handler_disconnect(self->soe_settings, handler_id);
    }

    self->data->connections.clear();
    self->data->gconnections_sie.clear();
    self->data->gconnections_soe.clear();

    g_object_unref(self->settings);
    g_object_unref(self->sie_settings);
    g_object_unref(self->soe_settings);

    PipeManager::exiting = true;

    delete self->data;
    delete self->presets_manager;
    delete self->sie;
    delete self->soe;
    delete self->pm;

    self->data = nullptr;
    self->presets_manager = nullptr;
    self->sie = nullptr;
    self->soe = nullptr;
    self->pm = nullptr;

    util::debug(log_tag + "shutting down..."s);
  };
}

void application_init(Application* self) {
  std::array<GActionEntry, 6> entries{};

  entries[0] = {
      "quit", [](GSimpleAction* action, GVariant* parameter, gpointer app) { g_application_quit(G_APPLICATION(app)); },
      nullptr, nullptr, nullptr};

  entries[1] = {"help",
                [](GSimpleAction* action, GVariant* parameter, gpointer gapp) {
                  gtk_show_uri(gtk_application_get_active_window(GTK_APPLICATION(gapp)), "help:easyeffects",
                               GDK_CURRENT_TIME);
                },
                nullptr, nullptr, nullptr};

  entries[2] = {"about",
                [](GSimpleAction* action, GVariant* parameter, gpointer gapp) {
                  std::array<const char*, 2> authors = {"Wellington Wallace", nullptr};

                  gtk_show_about_dialog(gtk_application_get_active_window(GTK_APPLICATION(gapp)), "program-name",
                                        "EasyEffects", "version", VERSION, "comments",
                                        _("Audio effects for PipeWire applications"), "authors", authors.data(),
                                        "logo-icon-name", "easyeffects", "license-type", GTK_LICENSE_GPL_3_0, "website",
                                        "https://github.com/wwmm/easyeffects", nullptr);
                },
                nullptr, nullptr, nullptr};

  entries[3] = {"fullscreen",
                [](GSimpleAction* action, GVariant* parameter, gpointer gapp) {
                  auto* self = EE_APP(gapp);

                  auto state = g_settings_get_boolean(self->settings, "window-fullscreen") != 0;

                  if (state) {
                    gtk_window_unfullscreen(GTK_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(gapp))));

                    g_settings_set_boolean(self->settings, "window-fullscreen", 0);
                  } else {
                    gtk_window_fullscreen(GTK_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(gapp))));

                    g_settings_set_boolean(self->settings, "window-fullscreen", 1);
                  }
                },
                nullptr, nullptr, nullptr};

  entries[4] = {"preferences",
                [](GSimpleAction* action, GVariant* parameter, gpointer gapp) {
                  auto* preferences = ui::preferences::window::create();

                  gtk_window_set_transient_for(GTK_WINDOW(preferences),
                                               GTK_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(gapp))));

                  gtk_window_set_modal(GTK_WINDOW(preferences), 1);
                  gtk_window_present(GTK_WINDOW(preferences));
                },
                nullptr, nullptr, nullptr};

  entries[5] = {"reset",
                [](GSimpleAction* action, GVariant* parameter, gpointer gapp) {
                  auto* self = EE_APP(gapp);

                  util::reset_all_keys(self->settings);

                  self->soe->reset_settings();
                  self->sie->reset_settings();
                },
                nullptr, nullptr, nullptr};

  g_action_map_add_action_entries(G_ACTION_MAP(self), entries.data(), entries.size(), self);

  std::array<const char*, 3> quit_accels = {"<Ctrl>Q", "<Ctrl>W", nullptr};
  std::array<const char*, 2> help_accels = {"F1", nullptr};
  std::array<const char*, 2> fullscreen_accels = {"F11", nullptr};

  gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.quit", quit_accels.data());
  gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.help", help_accels.data());
  gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.fullscreen", fullscreen_accels.data());
}

auto application_new() -> GApplication* {
  g_set_application_name("EasyEffects");

  auto* app = g_object_new(EE_TYPE_APPLICATION, "application-id", "com.github.wwmm.easyeffects", "flags",
                           G_APPLICATION_HANDLES_COMMAND_LINE, nullptr);

  g_application_add_main_option(G_APPLICATION(app), "quit", 'q', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,
                                _("Quit EasyEffects. Useful when running in service mode."), nullptr);

  g_application_add_main_option(G_APPLICATION(app), "load-preset", 'l', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING,
                                _("Load a preset. Example: easyeffects -l music"), nullptr);

  g_application_add_main_option(G_APPLICATION(app), "reset", 'r', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,
                                _("Reset EasyEffects."), nullptr);

  g_application_add_main_option(G_APPLICATION(app), "hide-window", 'w', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,
                                _("Hide the Window."), nullptr);

  g_application_add_main_option(G_APPLICATION(app), "bypass", 'b', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT,
                                _("Global bypass. 1 to enable, 2 to disable and 3 to get status"), nullptr);

  g_application_add_main_option(G_APPLICATION(app), "presets", 'p', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,
                                _("Show available presets."), nullptr);

  return G_APPLICATION(app);
}

}  // namespace app
