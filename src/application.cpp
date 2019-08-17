#include "application.hpp"
#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gtkmm/dialog.h>
#include <gtkmm/messagedialog.h>
#include "application_ui.hpp"
#include "config.h"
#include "pulse_manager.hpp"
#include "util.hpp"

Application::Application() : Gtk::Application("com.github.wwmm.pulseeffects", Gio::APPLICATION_HANDLES_COMMAND_LINE) {
  Glib::set_application_name("PulseEffects");
  Glib::setenv("PULSE_PROP_application.id", "com.github.wwmm.pulseeffects");
  Glib::setenv("PULSE_PROP_application.icon_name", "pulseeffects");

  signal_handle_local_options().connect(sigc::mem_fun(*this, &Application::on_handle_local_options), false);

  add_main_option_entry(Gio::Application::OPTION_TYPE_BOOL, "quit", 'q',
                        _("Quit PulseEffects. Useful when running in service mode."));

  add_main_option_entry(Gio::Application::OPTION_TYPE_BOOL, "presets", 'p', _("Show available presets."));

  add_main_option_entry(Gio::Application::OPTION_TYPE_STRING, "load-preset", 'l',
                        _("Load a preset. Example: pulseeffects -l music"));

  add_main_option_entry(Gio::Application::OPTION_TYPE_BOOL, "reset", 'r', _("Reset PulseEffects."));
}

Application::~Application() {
  util::debug(log_tag + " destroyed");
}

Glib::RefPtr<Application> Application::create() {
  return Glib::RefPtr<Application>(new Application());
}

int Application::on_command_line(const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) {
  auto options = command_line->get_options_dict();

  if (options->contains("quit")) {
    for (auto w : get_windows()) {
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
  } else {
    activate();
  }

  return Gtk::Application::on_command_line(command_line);
}

void Application::on_startup() {
  Gtk::Application::on_startup();

  util::debug(log_tag + "PE version: " + std::string(VERSION));

  settings = Gio::Settings::create("com.github.wwmm.pulseeffects");

  if (get_flags() & Gio::ApplicationFlags::APPLICATION_IS_SERVICE) {
    running_as_service = true;
  }

  create_actions();

  pm = std::make_unique<PulseManager>();
  sie = std::make_unique<SinkInputEffects>(pm.get());
  soe = std::make_unique<SourceOutputEffects>(pm.get());
  presets_manager = std::make_unique<PresetsManager>();

  pm->blacklist_in = settings->get_string_array("blacklist-in");
  pm->blacklist_out = settings->get_string_array("blacklist-out");

  pm->new_default_sink.connect([&](auto name) {
    util::debug("new default sink: " + name);

    if (name != "") {
      sie->set_output_sink_name(name);
      soe->webrtc->set_probe_src_device(name + ".monitor");
    }
  });

  pm->new_default_source.connect([&](auto name) {
    util::debug("new default source: " + name);

    if (name != "") {
      soe->set_source_monitor_name(name);
    }
  });

  pm->sink_changed.connect([&](std::shared_ptr<mySinkInfo> info) {
    if (info->name == pm->server_info.default_sink_name) {
      Glib::signal_timeout().connect_seconds_once(
          [=]() {
            // checking if after 3 seconds this sink still is the default sink
            if (info->name == pm->server_info.default_sink_name) {
              auto current_info = pm->get_sink_info(info->name);

              if (current_info != nullptr) {
                auto port = current_info->active_port;
                std::string dev_name;

                if (port != "null") {
                  dev_name = current_info->name + ":" + port;
                } else {
                  dev_name = current_info->name;
                }

                if (dev_name != last_sink_dev_name) {
                  last_sink_dev_name = dev_name;

                  presets_manager->autoload(PresetType::output, dev_name);
                }
              }
            }
          },
          3);
    }
  });

  pm->source_changed.connect([&](std::shared_ptr<mySourceInfo> info) {
    if (info->name == pm->server_info.default_source_name) {
      Glib::signal_timeout().connect_seconds_once(
          [=]() {
            // checking if after 3 seconds this source still is the default source
            if (info->name == pm->server_info.default_source_name) {
              auto current_info = pm->get_source_info(info->name);

              if (current_info != nullptr) {
                auto port = current_info->active_port;
                std::string dev_name;

                if (port != "null") {
                  dev_name = current_info->name + ":" + port;
                } else {
                  dev_name = current_info->name;
                }

                if (dev_name != last_source_dev_name) {
                  last_source_dev_name = dev_name;

                  presets_manager->autoload(PresetType::input, dev_name);
                }
              }
            }
          },
          3);
    }
  });

  settings->signal_changed("blacklist-in").connect([=](auto key) {
    pm->blacklist_in = settings->get_string_array("blacklist-in");
  });

  settings->signal_changed("blacklist-out").connect([=](auto key) {
    pm->blacklist_out = settings->get_string_array("blacklist-out");
  });

  if (running_as_service) {
    pm->find_sink_inputs();
    pm->find_source_outputs();
    pm->find_sinks();
    pm->find_sources();

    util::debug(log_tag + "Running in Background");

    hold();
  }
}

void Application::on_activate() {
  if (get_active_window() == nullptr) {
    auto window = ApplicationUi::create(this);

    add_window(*window);

    window->signal_hide().connect([&, window]() {
      int width, height;

      window->get_size(width, height);

      settings->set_int("window-width", width);
      settings->set_int("window-height", height);

      delete window;
    });

    window->show_all();

    pm->find_sink_inputs();
    pm->find_source_outputs();
    pm->find_sinks();
    pm->find_sources();
  }
}

int Application::on_handle_local_options(const Glib::RefPtr<Glib::VariantDict>& options) {
  if (!options) {
    std::cerr << G_STRFUNC << ": options is null." << std::endl;
  }

  // Remove some options to show that we have handled them in the local
  // instance, so they won't be passed to the primary (remote) instance:
  options->remove("preset");

  presets_manager = std::make_unique<PresetsManager>();

  if (options->contains("presets")) {
    std::string list;

    for (auto name : presets_manager->get_names(PresetType::output)) {
      list += name + ",";
    }

    std::clog << _("Output Presets: ") + list << std::endl;

    list = "";

    for (auto name : presets_manager->get_names(PresetType::input)) {
      list += name + ",";
    }

    std::clog << _("Input Presets: ") + list << std::endl;

    return EXIT_SUCCESS;
  }

  return -1;
}

void Application::create_actions() {
  add_action("about", [&]() {
    auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/about.glade");

    auto dialog = (Gtk::Dialog*)builder->get_object("about_dialog").get();

    dialog->signal_response().connect([=](auto response_id) {
      switch (response_id) {
        case Gtk::RESPONSE_CLOSE:
        case Gtk::RESPONSE_CANCEL:
        case Gtk::RESPONSE_DELETE_EVENT:
          dialog->hide();
          util::debug(log_tag + "hiding the about dialog window");
          break;
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
    auto window = get_active_window();

    /*GTKMM 3.22 does not have a wrapper for gtk_show_uri_on_window.
     *So we have to use the C api :-(
     */

    if (!gtk_show_uri_on_window(window->gobj(), "help:pulseeffects", gtk_get_current_event_time(), nullptr)) {
      util::warning("Failed to open help!");
    }
  });

  add_action("quit", [&] {
    auto window = get_active_window();

    window->hide();
  });

  set_accel_for_action("app.help", "F1");
  set_accel_for_action("app.quit", "<Ctrl>Q");
}
