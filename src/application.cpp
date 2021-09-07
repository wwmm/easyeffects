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

Application::Application()
    : Gtk::Application("com.github.wwmm.easyeffects", Gio::Application::Flags::HANDLES_COMMAND_LINE) {
  Glib::set_application_name("EasyEffects");

  signal_handle_local_options().connect(sigc::mem_fun(*this, &Application::on_handle_local_options), false);

  add_main_option_entry(Gio::Application::OptionType::BOOL, "quit", 'q',
                        _("Quit EasyEffects. Useful when running in service mode."));

  add_main_option_entry(Gio::Application::OptionType::BOOL, "presets", 'p', _("Show available presets."));

  add_main_option_entry(Gio::Application::OptionType::STRING, "load-preset", 'l',
                        _("Load a preset. Example: easyeffects -l music"));

  add_main_option_entry(Gio::Application::OptionType::BOOL, "reset", 'r', _("Reset EasyEffects."));

  add_main_option_entry(Gio::Application::OptionType::INT, "bypass", 'b',
                        _("Global bypass. 1 to enable, 2 to disable and 3 to get status"));

  add_main_option_entry(Gio::Application::OptionType::BOOL, "hide-window", 'w', _("Hide the Window."));
}

Application::~Application() {
  util::debug(log_tag + " destroyed");
}

auto Application::create() -> Glib::RefPtr<Application> {
  return Glib::RefPtr<Application>(new Application());
}

auto Application::on_command_line(const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) -> int {
  const auto& options = command_line->get_options_dict();

  if (options->contains("quit")) {
    for (const auto& w : get_windows()) {
      w->hide();
    }

    quit();
  } else if (options->contains("load-preset")) {
    Glib::ustring name;

    if (!options->lookup_value("load-preset", name)) {
      util::debug(log_tag + "failed to load preset: " + name);
    } else {
      if (presets_manager->preset_file_exists(PresetType::input, name)) {
        presets_manager->load_preset_file(PresetType::input, name);
      }

      if (presets_manager->preset_file_exists(PresetType::output, name)) {
        presets_manager->load_preset_file(PresetType::output, name);
      }
    }
  } else if (options->contains("reset")) {
    settings->reset("");

    util::info(log_tag + "All settings were reset");
  } else if (options->contains("hide-window")) {
    util::info(log_tag + "Hiding the window...");

    for (const auto& w : get_windows()) {
      w->hide();
    }
  } else if (options->contains("bypass")) {
    if (int bypass_arg = 2; options->lookup_value("bypass", bypass_arg)) {
      if (bypass_arg == 1) {
        settings->set_boolean("bypass", true);
      } else if (bypass_arg == 2) {
        settings->set_boolean("bypass", false);
      }
    }

  } else {
    activate();
  }

  return Gtk::Application::on_command_line(command_line);
}

void Application::on_startup() {
  Gtk::Application::on_startup();

  util::debug(log_tag + "easyeffects version: " + std::string(VERSION));

  settings = Gio::Settings::create("com.github.wwmm.easyeffects");
  soe_settings = Gio::Settings::create("com.github.wwmm.easyeffects.streamoutputs");
  sie_settings = Gio::Settings::create("com.github.wwmm.easyeffects.streaminputs");

  if (static_cast<int>(get_flags() & Gio::Application::Flags::IS_SERVICE) != 0) {
    running_as_service = true;
  }

  create_actions();

  pm = std::make_unique<PipeManager>();
  soe = std::make_unique<StreamOutputEffects>(pm.get());
  sie = std::make_unique<StreamInputEffects>(pm.get());

  if (presets_manager == nullptr) {
    presets_manager = std::make_unique<PresetsManager>();
  }

  pm->new_default_sink.connect([&](NodeInfo node) {
    util::debug("new default output device: " + node.name);

    if (soe_settings->get_boolean("use-default-output-device")) {
      /*
        Depending on the hardware headphones can cause a node recreation hwere the id and the name are kept.
        So we clear the key to force the callbacks to be called
      */

      soe_settings->set_string("output-device", "");
      soe_settings->set_string("output-device", node.name);
    }
  });

  pm->new_default_source.connect([&](NodeInfo node) {
    util::debug("new default input device: " + node.name);

    if (sie_settings->get_boolean("use-default-input-device")) {
      /*
        Depending on the hardware microphones can cause a node recreation hwere the id and the name are kept.
        So we clear the key to force the callbacks to be called
      */

      sie_settings->set_string("input-device", "");
      sie_settings->set_string("input-device", node.name);
    }
  });

  pm->device_input_route_changed.connect([&](DeviceInfo device) {
    if (device.input_route_available == SPA_PARAM_AVAILABILITY_no) {
      return;
    }

    util::debug(log_tag + "device " + device.name + " has changed its input route to: " + device.input_route_name);

    NodeInfo target_node;

    for (const auto& [id, node] : pm->node_map) {
      if (node.device_id == device.id && node.media_class == "Audio/Source") {
        target_node = node;

        break;
      }
    }

    if (target_node.id != SPA_ID_INVALID) {
      if (target_node.name.c_str() == sie_settings->get_string("input-device")) {
        presets_manager->autoload(PresetType::input, target_node.name, device.input_route_name);
      }
    }
  });

  pm->device_output_route_changed.connect([&](DeviceInfo device) {
    if (device.output_route_available == SPA_PARAM_AVAILABILITY_no) {
      return;
    }

    util::debug(log_tag + "device " + device.name + " has changed its output route to: " + device.output_route_name);

    NodeInfo target_node;

    for (const auto& [id, node] : pm->node_map) {
      if (node.device_id == device.id && node.media_class == "Audio/Sink") {
        target_node = node;

        break;
      }
    }

    if (target_node.id != SPA_ID_INVALID) {
      if (target_node.name.c_str() == soe_settings->get_string("output-device")) {
        presets_manager->autoload(PresetType::output, target_node.name, device.output_route_name);
      }
    }
  });

  soe_settings->signal_changed("output-device").connect([&, this](const auto& key) {
    const auto& name = soe_settings->get_string(key).raw();

    if (name.empty()) {
      return;
    }

    uint device_id = SPA_ID_INVALID;

    for (const auto& [id, node] : pm->node_map) {
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
    const auto& name = sie_settings->get_string(key).raw();

    if (name.empty()) {
      return;
    }

    uint device_id = SPA_ID_INVALID;

    for (const auto& [id, node] : pm->node_map) {
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
    /*
      Note to myself: do not wrap this pointer in a smart pointer. Causes memory leaks when closing the window because
      GTK reference counting system will see that there is still someone with an object reference and it won't free the
      widgets.
    */

    auto* const window = ApplicationUi::create(this);

    add_window(*window);

    window->signal_close_request().connect(
        [&, window]() {
          int width = 0;
          int height = 0;

          window->get_default_size(width, height);

          settings->set_int("window-width", width);
          settings->set_int("window-height", height);

          // util::warning(std::to_string(width) + " x " + std::to_string(height));

          delete window;

          if (settings->get_boolean("shutdown-on-window-close")) {
            release();
          }

          return false;
        },
        false);

    window->show();
  }
}

auto Application::on_handle_local_options(const Glib::RefPtr<Glib::VariantDict>& options) -> int {
  if (!options) {
    std::cerr << G_STRFUNC << ": options is null." << std::endl;
  }

  // Remove some options to show that we have handled them in the local
  // instance, so they won't be passed to the primary (remote) instance:
  options->remove("preset");

  if (presets_manager == nullptr) {
    presets_manager = std::make_unique<PresetsManager>();
  }

  if (options->contains("presets")) {
    std::string list;

    for (const auto& name : presets_manager->get_names(PresetType::output)) {
      list += name + ",";
    }

    std::clog << _("Output Presets: ") + list << std::endl;

    list = "";

    for (const auto& name : presets_manager->get_names(PresetType::input)) {
      list += name + ",";
    }

    std::clog << _("Input Presets: ") + list << std::endl;

    return EXIT_SUCCESS;
  }

  if (options->contains("bypass")) {
    if (int bypass_arg = 2; options->lookup_value("bypass", bypass_arg)) {
      if (bypass_arg == 3) {
        const auto& cfg = Gio::Settings::create("com.github.wwmm.easyeffects");

        std::clog << cfg->get_boolean("bypass") << std::endl;

        return EXIT_SUCCESS;
      }
    }
  }

  return -1;
}

void Application::create_actions() {
  add_action("about", [&]() {
    auto* const dialog = new Gtk::AboutDialog();

    dialog->set_program_name("EasyEffects");
    dialog->set_version(VERSION);
    dialog->set_comments(_("Audio effects for PipeWire applications"));
    dialog->set_authors({"Wellington Wallace"});
    dialog->set_logo_icon_name("easyeffects");
    dialog->set_license_type(Gtk::License::GPL_3_0);
    dialog->set_website("https://github.com/wwmm/pulseeffects");

    dialog->set_modal(true);

    dialog->set_transient_for(*get_active_window());

    dialog->set_hide_on_close(true);

    dialog->present();
  });

  add_action("help", [&] {
    auto* const window = get_active_window();

    // show_uri has not been wrapped by GTKMM yet :-(

    gtk_show_uri(window->gobj(), "help:easyeffects", GDK_CURRENT_TIME);
  });

  add_action("quit", [&] {
    auto* const window = get_active_window();

    window->hide();
  });

  set_accel_for_action("app.help", "F1");
  set_accel_for_action("app.quit", "<Ctrl>Q");
}

void Application::update_bypass_state(const Glib::ustring& key) {
  const auto& state = settings->get_boolean(key);

  soe->set_bypass(state);
  sie->set_bypass(state);

  util::info(log_tag + ((state) ? "enabling" : "disabling") + " global bypass");
}
