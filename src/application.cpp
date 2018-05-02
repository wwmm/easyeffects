#include <giomm/file.h>
#include <glibmm/i18n.h>
#include <cstddef>
#include <iostream>
#include "application.hpp"
#include "application_window.hpp"
#include "pulse_manager.hpp"
#include "util.hpp"

Application::Application()
    : Gtk::Application("com.github.wwmm.pulseeffects",
                       Gio::APPLICATION_HANDLES_COMMAND_LINE),
      settings(Gio::Settings::create("com.github.wwmm.pulseeffects")),
      pm(std::unique_ptr<PulseManager>(new PulseManager())) {
    Glib::set_application_name("PulseEffects");
    Glib::setenv("PULSE_PROP_application.id", "com.github.wwmm.pulseeffects");
    Glib::setenv("PULSE_PROP_application.icon_name", "pulseeffects");

    add_main_option_entry(
        Gio::Application::OPTION_TYPE_BOOL, "quit", 'q',
        _("Quit PulseEffects. Useful when running in service mode."));

    add_main_option_entry(Gio::Application::OPTION_TYPE_BOOL, "presets", 'p',
                          _("Show available presets."));

    add_main_option_entry(Gio::Application::OPTION_TYPE_STRING, "load-preset",
                          'l',
                          _("Load a preset. Example: pulseeffects -l music"));
}

Application::~Application() {}

Glib::RefPtr<Application> Application::create() {
    return Glib::RefPtr<Application>(new Application());
}

void Application::on_activate() {
    if (get_active_window() == nullptr) {
        ApplicationWindow(this);
    }
}

int Application::on_command_line(
    const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) {
    auto options = command_line->get_options_dict();

    if (options->contains("quit")) {
        quit();
    } else if (options->contains("presets")) {
    } else if (options->contains("load-presets")) {
        // var value = options.lookup_value("load-preset",
        // new VariantType("s"));
    } else {
        activate();
    }

    return Gtk::Application::on_command_line(command_line);
}

void Application::on_startup() {
    Gtk::Application::on_startup();

    running_as_service = false;

    create_appmenu();

    create_presets_directory();

    // just for tests. It will be removed

    pm->source_added.connect(
        [](auto i) { util::debug("added source: " + i->name); });
    pm->source_removed.connect(
        [](auto i) { util::debug("removed source: " + std::to_string(i)); });
    pm->sink_added.connect(
        [](auto i) { util::debug("added sink: " + i->name); });
    pm->sink_removed.connect(
        [](auto i) { util::debug("removed sink: " + std::to_string(i)); });

    pm->new_default_sink.connect(
        [](auto i) { util::debug("new default sink: " + i); });
    pm->new_default_source.connect(
        [](auto i) { util::debug("new default source: " + i); });

    if (get_flags() & Gio::ApplicationFlags::APPLICATION_IS_SERVICE) {
        running_as_service = true;

        util::debug(log_tag + "Running in Background");

        hold();
    }
}

void Application::create_appmenu() {
    add_action("about", [&]() {
        auto builder = Gtk::Builder::create_from_resource(
            "/com/github/wwmm/pulseeffects/about.glade");

        auto dialog = (Gtk::Dialog*)builder->get_object("about_dialog").get();

        dialog->set_transient_for(*get_active_window());

        dialog->show();

        // Bring it to the front, in case it was already shown:
        dialog->present();
    });

    add_action("quit", [&] {
        auto windows = get_windows();

        for (auto w : windows) {
            w->hide();
        }

        quit();
    });

    auto menu = Gio::Menu::create();

    menu->append("About", "app.about");
    menu->append("Quit", "app.quit");

    set_app_menu(menu);
}

void Application::create_presets_directory() {
    auto path = Glib::get_user_config_dir() + "/PulseEffects";

    auto file = Gio::File::create_for_path(path);

    if (file->query_exists()) {
        util::debug(log_tag + "user preset directory already exists");
    } else {
        file->make_directory();

        util::debug(log_tag + "user presets directory created: " + path);
    }
}
