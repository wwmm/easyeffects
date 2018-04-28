#include <iostream>
#include "application.hpp"

Application::Application()
    : Gtk::Application("com.github.wwmm.pulseeffects",
                       Gio::APPLICATION_HANDLES_COMMAND_LINE) {
    Glib::set_application_name("PulseEffects");

    std::cout << "oi1" << std::endl;
}

Application::~Application() {}

Glib::RefPtr<Application> Application::create() {
    return Glib::RefPtr<Application>(new Application());
}

void Application::on_activate() {
    std::cout << "oi" << std::endl;
}
