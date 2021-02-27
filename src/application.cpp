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

#include "application.hpp"
#include "application_ui.hpp"

Application::Application()
    : Gtk::Application("com.github.wwmm.pulseeffects", Gio::Application::Flags::HANDLES_COMMAND_LINE) {
  Glib::set_application_name("PulseEffects");

  signal_handle_local_options().connect(sigc::mem_fun(*this, &Application::on_handle_local_options), false);

  add_main_option_entry(Gio::Application::OptionType::BOOL, "quit", 'q',
                        _("Quit PulseEffects. Useful when running in service mode."));

  add_main_option_entry(Gio::Application::OptionType::BOOL, "presets", 'p', _("Show available presets."));

  add_main_option_entry(Gio::Application::OptionType::STRING, "load-preset", 'l',
                        _("Load a preset. Example: pulseeffects -l music"));

  add_main_option_entry(Gio::Application::OptionType::BOOL, "reset", 'r', _("Reset PulseEffects."));

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
  auto options = command_line->get_options_dict();

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
        presets_manager->load(PresetType::input, name);
      }

      if (presets_manager->preset_file_exists(PresetType::output, name)) {
        presets_manager->load(PresetType::output, name);
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
    int bypass_arg = 2;

    if (options->lookup_value("bypass", bypass_arg)) {
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

  util::debug(log_tag + "PE version: " + std::string(VERSION));

  settings = Gio::Settings::create("com.github.wwmm.pulseeffects");

  if (static_cast<int>(get_flags() & Gio::Application::Flags::IS_SERVICE) != 0U) {
    running_as_service = true;
  }

  create_actions();

  pm = std::make_unique<PipeManager>();
  soe = std::make_unique<StreamOutputEffects>(pm.get());
  sie = std::make_unique<StreamInputEffects>(pm.get());

  if (presets_manager == nullptr) {
    presets_manager = std::make_unique<PresetsManager>();
  }

  pm->blocklist_in = settings->get_string_array("blocklist-in");
  pm->blocklist_out = settings->get_string_array("blocklist-out");

  pm->new_default_sink.connect([&](const NodeInfo& node) {
    util::debug("new default sink: " + node.name);

    if (soe->get_output_node_id() != node.id && settings->get_boolean("use-default-sink")) {
      soe->set_null_pipeline();

      soe->set_output_node_id(node.id);

      soe->update_pipeline_state();

      sie->webrtc->set_probe_input_node_id(node.id);
    }

    Glib::signal_timeout().connect_seconds_once(
        [=]() {
          auto defaul_sink_name = pm->default_sink.name;

          // checking if after 2 seconds this sink still is the default sink
          if (node.name == defaul_sink_name) {
            if (node.name != last_sink_dev_name) {
              last_sink_dev_name = node.name;

              presets_manager->autoload(PresetType::output, node.name);
            }
          }
        },
        2);
  });

  pm->new_default_source.connect([&](const NodeInfo& node) {
    util::debug("new default source: " + node.name);

    if (sie->get_input_node_id() != node.id && settings->get_boolean("use-default-source")) {
      sie->set_null_pipeline();

      sie->change_input_device(node);

      sie->update_pipeline_state();
    }

    Glib::signal_timeout().connect_seconds_once(
        [=]() {
          auto defaul_source_name = pm->default_source.name;

          // checking if after 2 seconds this source still is the default source
          if (node.name == defaul_source_name) {
            if (node.name != last_source_dev_name) {
              last_source_dev_name = node.name;

              presets_manager->autoload(PresetType::input, node.name);
            }
          }
        },
        3);
  });

  settings->signal_changed("blocklist-in").connect([=](auto key) {
    pm->blocklist_in = settings->get_string_array("blocklist-in");
  });

  settings->signal_changed("blocklist-out").connect([=](auto key) {
    pm->blocklist_out = settings->get_string_array("blocklist-out");
  });

  settings->signal_changed("bypass").connect([=](auto key) { update_bypass_state(key); });

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

    auto* window = ApplicationUi::create(this);

    add_window(*window);

    window->signal_close_request().connect(
        [&, window]() {
          int width = window->get_width();
          int height = window->get_height();

          settings->set_int("window-width", width);
          settings->set_int("window-height", height);

          delete window;

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
    int bypass_arg = 2;

    if (options->lookup_value("bypass", bypass_arg)) {
      if (bypass_arg == 3) {
        auto cfg = Gio::Settings::create("com.github.wwmm.pulseeffects");

        std::clog << cfg->get_boolean("bypass") << std::endl;

        return EXIT_SUCCESS;
      }
    }
  }

  return -1;
}

void Application::create_actions() {
  add_action("about", [&]() {
    auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/about.glade");

    auto* dialog = (Gtk::Dialog*)builder->get_object("about_dialog").get();

    dialog->signal_response().connect([=](auto response_id) {
      switch (response_id) {
        case Gtk::ResponseType::CLOSE:
        case Gtk::ResponseType::CANCEL:
        case Gtk::ResponseType::DELETE_EVENT: {
          dialog->hide();
          util::debug(log_tag + "hiding the about dialog window");
          break;
        }
        default:
          util::debug(log_tag + "unexpected about dialog response!");
          break;
      }
    });

    dialog->set_transient_for(*get_active_window());

    dialog->show();

    // Bring it to the front, in case it was already shown:
    dialog->present();
  });

  add_action("help", [&] {
    auto* window = get_active_window();

    // show_uri has not been wrapped by GTKMM yet :-(

    gtk_show_uri(window->gobj(), "help:pulseeffects", GDK_CURRENT_TIME);
  });

  add_action("quit", [&] {
    auto* window = get_active_window();

    window->hide();
  });

  set_accel_for_action("app.help", "F1");
  set_accel_for_action("app.quit", "<Ctrl>Q");
}

void Application::update_bypass_state(const std::string& key) {
  auto state = settings->get_boolean(key);

  if (state) {
    util::info(log_tag + "enabling global bypass");

    soe->do_bypass(true);
    sie->do_bypass(true);
  } else {
    util::info(log_tag + "disabling global bypass");

    soe->do_bypass(false);
    sie->do_bypass(false);
  }
}
