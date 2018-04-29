#include <glibmm/i18n.h>
#include <cstddef>
#include <iostream>
#include "application.hpp"
#include "application_window.hpp"

Application::Application()
    : Gtk::Application("com.github.wwmm.pulseeffects",
                       Gio::APPLICATION_HANDLES_COMMAND_LINE),
      settings(Gio::Settings::create("com.github.wwmm.pulseeffects")) {
    Glib::set_application_name("PulseEffects");
    Glib::setenv("PULSE_PROP_application.id", "com.github.wwmm.pulseeffects");
    Glib::setenv("PULSE_PROP_application.icon_name", "pulseeffects");

    this->add_main_option_entry(
        Gio::Application::OPTION_TYPE_BOOL, "quit", 'q',
        _("Quit PulseEffects. Useful when running in service mode."));

    this->add_main_option_entry(Gio::Application::OPTION_TYPE_BOOL, "presets",
                                'p', _("Show available presets."));

    this->add_main_option_entry(
        Gio::Application::OPTION_TYPE_STRING, "load-preset", 'l',
        _("Load a preset. Example: pulseeffects -l music"));
}

Application::~Application() {}

Glib::RefPtr<Application> Application::create() {
    return Glib::RefPtr<Application>(new Application());
}

void Application::on_activate() {
    if (this->get_active_window() == nullptr) {
        ApplicationWindow(this);
    }
}

int Application::on_command_line(
    const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) {
    auto options = command_line->get_options_dict();

    if (options->contains("quit")) {
        this->quit();
    } else if (options->contains("presets")) {
    } else if (options->contains("load-presets")) {
        // var value = options.lookup_value("load-preset",
        // new VariantType("s"));
    } else {
        this->activate();
    }

    return Gtk::Application::on_command_line(command_line);
}

void Application::on_startup() {
    Gtk::Application::on_startup();

    running_as_service = false;

    if (this->get_flags() & Gio::ApplicationFlags::APPLICATION_IS_SERVICE) {
        running_as_service = true;

        this->hold();
    }

    g_debug("test");
}
