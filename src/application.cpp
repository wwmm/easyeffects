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

auto constexpr log_tag = "application: ";

struct _Application {
  AdwApplication parent_instance{};

  bool running_as_service = false;

  GSettings* settings = nullptr;
  GSettings* soe_settings = nullptr;
  GSettings* sie_settings = nullptr;

  std::unique_ptr<PipeManager> pm;
  std::unique_ptr<StreamOutputEffects> soe;
  std::unique_ptr<StreamInputEffects> sie;
  std::unique_ptr<PresetsManager> presets_manager;
};

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

void application_class_init(ApplicationClass* klass) {
  G_APPLICATION_CLASS(klass)->command_line = [](GApplication* gapp, GApplicationCommandLine* cmdline) {
    auto* app = EE_APP(gapp);
    auto* options = g_application_command_line_get_options_dict(cmdline);

    if (g_variant_dict_contains(options, "quit") != 0) {
      hide_all_windows(gapp);

      g_application_quit(G_APPLICATION(gapp));
    } else if (g_variant_dict_contains(options, "load-preset") != 0) {
      const char* name = nullptr;

      g_variant_dict_lookup(options, "load-preset", "&s", &name);

      util::warning(name + std::string(" Not implemented yet..."));
    } else if (g_variant_dict_contains(options, "reset") != 0) {
      g_settings_reset(app->settings, "");

      util::info(std::string(log_tag) + "All settings were reset");
    } else if (g_variant_dict_contains(options, "hide-window") != 0) {
      hide_all_windows(gapp);

      util::info(std::string(log_tag) + "Hiding the window...");
    } else if (g_variant_dict_contains(options, "bypass") != 0) {
      if (int bypass_arg = 2; g_variant_dict_lookup(options, "bypass", "i", &bypass_arg)) {
        if (bypass_arg == 1) {
          g_settings_set_boolean(app->settings, "bypass", 1);
        } else if (bypass_arg == 2) {
          g_settings_set_boolean(app->settings, "bypass", 0);
        }
      }
    } else {
      g_application_activate(gapp);
    }

    return G_APPLICATION_CLASS(application_parent_class)->command_line(gapp, cmdline);
  };

  G_APPLICATION_CLASS(klass)->handle_local_options = [](GApplication* gapp, GVariantDict* options) {
    if (options == nullptr) {
      return -1;
    }

    auto* self = EE_APP(gapp);

    if (self->presets_manager == nullptr) {
      self->presets_manager = std::make_unique<PresetsManager>();
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

  G_APPLICATION_CLASS(klass)->startup = [](GApplication* gapp) {
    G_APPLICATION_CLASS(application_parent_class)->startup(gapp);

    auto* self = EE_APP(gapp);

    if ((g_application_get_flags(gapp) & G_APPLICATION_IS_SERVICE) != 0) {
      self->running_as_service = true;
    }

    std::array<GActionEntry, 3> entries{};

    entries[0] = {
        "quit",
        [](GSimpleAction* action, GVariant* parameter, gpointer app) { g_application_quit(G_APPLICATION(app)); },
        nullptr, nullptr, nullptr};

    entries[1] = {"help",
                  [](GSimpleAction* action, GVariant* parameter, gpointer gapp) {
                    gtk_show_uri(gtk_application_get_active_window(GTK_APPLICATION(gapp)), "help:easyeffects",
                                 GDK_CURRENT_TIME);
                  },
                  nullptr, nullptr, nullptr};

    entries[2] = {"about",
                  [](GSimpleAction* action, GVariant* parameter, gpointer gapp) {
                    gtk_show_about_dialog(gtk_application_get_active_window(GTK_APPLICATION(gapp)), "program-name",
                                          "EasyEffects", "version", VERSION, "comments",
                                          _("Audio effects for PipeWire applications"), "authors", "Wellington Wallace",
                                          "logo-icon-name", "easyeffects", "license-type", GTK_LICENSE_GPL_3_0,
                                          "website", "https://github.com/wwmm/easyeffects", nullptr);
                  },
                  nullptr, nullptr, nullptr};

    g_action_map_add_action_entries(G_ACTION_MAP(gapp), entries.data(), entries.size(), gapp);

    std::array<const char*, 2> quit_accels = {"<Ctrl>Q", nullptr};
    std::array<const char*, 2> help_accels = {"F1", nullptr};

    gtk_application_set_accels_for_action(GTK_APPLICATION(gapp), "app.quit", quit_accels.data());
    gtk_application_set_accels_for_action(GTK_APPLICATION(gapp), "app.help", help_accels.data());
  };

  G_APPLICATION_CLASS(klass)->activate = [](GApplication* gapp) {
    if (gtk_application_get_active_window(GTK_APPLICATION(gapp)) == nullptr) {
      G_APPLICATION_CLASS(application_parent_class)->activate(gapp);

      auto* window = ui::application_window::application_window_new();

      gtk_application_add_window(GTK_APPLICATION(gapp), GTK_WINDOW(window));

      gtk_window_present(GTK_WINDOW(window));
    }
  };

  G_APPLICATION_CLASS(klass)->shutdown = [](GApplication* gapp) {
    G_APPLICATION_CLASS(application_parent_class)->shutdown(gapp);

    auto* self = EE_APP(gapp);

    g_object_unref(self->settings);

    // Making sure some destructors are called. I have no idea why this is not happening automatically

    self->pm = nullptr;
    self->presets_manager = nullptr;

    util::debug(std::string(log_tag) + "shutting down...");
  };
}

void application_init(Application* self) {
  self->settings = g_settings_new("com.github.wwmm.easyeffects");
  self->sie_settings = g_settings_new("com.github.wwmm.easyeffects.streaminputs");
  self->soe_settings = g_settings_new("com.github.wwmm.easyeffects.streamoutputs");

  self->pm = std::make_unique<PipeManager>();
  self->soe = std::make_unique<StreamOutputEffects>(self->pm.get());
  self->sie = std::make_unique<StreamInputEffects>(self->pm.get());

  if (self->presets_manager == nullptr) {
    self->presets_manager = std::make_unique<PresetsManager>();
  }

  self->pm->new_default_sink.connect([=](const NodeInfo node) {
    util::debug("new default output device: " + node.name);

    if (g_settings_get_boolean(self->soe_settings, "use-default-output-device") != 0) {
      /*
        Depending on the hardware headphones can cause a node recreation here the id and the name are kept.
        So we clear the key to force the callbacks to be called
      */

      g_settings_set_string(self->soe_settings, "output-device", "");
      g_settings_set_string(self->soe_settings, "output-device", node.name.c_str());
    }
  });

  self->pm->new_default_source.connect([=](const NodeInfo node) {
    util::debug("new default input device: " + node.name);

    if (g_settings_get_boolean(self->sie_settings, "use-default-input-device") != 0) {
      /*
        Depending on the hardware microphones can cause a node recreation hwere the id and the name are kept.
        So we clear the key to force the callbacks to be called
      */

      g_settings_set_string(self->sie_settings, "input-device", "");
      g_settings_set_string(self->sie_settings, "input-device", node.name.c_str());
    }
  });

  self->pm->device_input_route_changed.connect([=](const DeviceInfo device) {
    if (device.input_route_available == SPA_PARAM_AVAILABILITY_no) {
      return;
    }

    util::debug(std::string(log_tag) + "device " + device.name +
                " has changed its input route to: " + device.input_route_name);

    NodeInfo target_node;

    for (const auto& [ts, node] : self->pm->node_map) {
      if (node.device_id == device.id && node.media_class == self->pm->media_class_source) {
        target_node = node;

        break;
      }
    }

    if (target_node.id != SPA_ID_INVALID) {
      if (target_node.name.c_str() == g_settings_get_string(self->sie_settings, "input-device")) {
        self->presets_manager->autoload(PresetType::input, target_node.name, device.input_route_name);
      } else {
        util::debug(std::string(log_tag) +
                    "input autoloading: the target node name does not match the input device name");
      }
    } else {
      util::debug(std::string(log_tag) + "input autoloading: could not find the target node");
    }
  });

  self->pm->device_output_route_changed.connect([=](const DeviceInfo device) {
    if (device.output_route_available == SPA_PARAM_AVAILABILITY_no) {
      return;
    }

    util::debug(std::string(log_tag) + "device " + device.name +
                " has changed its output route to: " + device.output_route_name);

    NodeInfo target_node;

    for (const auto& [ts, node] : self->pm->node_map) {
      target_node = node;

      if (node.device_id == device.id && node.media_class == self->pm->media_class_sink) {
        target_node = node;

        break;
      }
    }

    if (target_node.id != SPA_ID_INVALID) {
      if (target_node.name.c_str() == g_settings_get_string(self->soe_settings, "output-device")) {
        self->presets_manager->autoload(PresetType::output, target_node.name, device.output_route_name);
      } else {
        util::debug(std::string(log_tag) +
                    "output autoloading: the target node name does not match the output device name");
      }
    } else {
      util::debug(std::string(log_tag) + "output autoloading: could not find the target node");
    }
  });
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

  // g_signal_connect(app, "handle-local-options", G_CALLBACK(local_options), nullptr);

  return G_APPLICATION(app);
}

}  // namespace app

Application::Application()
    : Gtk::Application("com.github.wwmm.easyeffects", Gio::Application::Flags::HANDLES_COMMAND_LINE) {}

Application::~Application() {
  util::debug(log_tag + " destroyed");
}

auto Application::create() -> Glib::RefPtr<Application> {
  return Glib::RefPtr<Application>(new Application());
}

void Application::on_startup() {
  soe_settings->signal_changed("output-device").connect([&, this](const auto& key) {
    const auto name = soe_settings->get_string(key).raw();

    if (name.empty()) {
      return;
    }

    uint device_id = SPA_ID_INVALID;

    for (const auto& [ts, node] : pm->node_map) {
      if (node.name == name) {
        device_id = node.device_id;

        break;
      }
    }

    if (device_id != SPA_ID_INVALID) {
      for (const auto& device : pm->list_devices) {
        if (device.id == device_id) {
          presets_manager->autoload(PresetType::output, name, device.output_route_name);

          break;
        }
      }
    }
  });

  sie_settings->signal_changed("input-device").connect([&, this](const auto& key) {
    const auto name = sie_settings->get_string(key).raw();

    if (name.empty()) {
      return;
    }

    uint device_id = SPA_ID_INVALID;

    for (const auto& [ts, node] : pm->node_map) {
      if (node.name == name) {
        device_id = node.device_id;

        break;
      }
    }

    if (device_id != SPA_ID_INVALID) {
      for (const auto& device : pm->list_devices) {
        if (device.id == device_id) {
          presets_manager->autoload(PresetType::input, name, device.input_route_name);

          break;
        }
      }
    }
  });

  settings->signal_changed("bypass").connect([=, this](const auto& key) { update_bypass_state(key); });

  update_bypass_state("bypass");

  if (running_as_service) {
    util::debug(log_tag + "Running in Background");

    hold();
  }
}

void Application::on_activate() {
  if (get_active_window() == nullptr) {
    // auto* const window = ApplicationUi::create(this);

    // add_window(*window);

    // window->signal_close_request().connect(
    //     [&, window]() {
    //       int width = 0;
    //       int height = 0;

    //       window->get_default_size(width, height);

    //       settings->set_int("window-width", width);
    //       settings->set_int("window-height", height);
    //       settings->set_boolean("window-maximized", window->is_maximized());

    //       // util::warning(std::to_string(width) + " x " + std::to_string(height));

    //       delete window;

    //       if (settings->get_boolean("shutdown-on-window-close")) {
    //         release();
    //       }

    //       return false;
    //     },
    //     false);

    // window->show();
  }
}

void Application::update_bypass_state(const Glib::ustring& key) {
  const auto state = settings->get_boolean(key);

  soe->set_bypass(state);
  sie->set_bypass(state);

  util::info(log_tag + ((state) ? "enabling" : "disabling") + " global bypass");
}
