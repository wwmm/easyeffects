#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <giomm/settings.h>
#include <gtkmm/application.h>
#include <memory>
#include "pulse_manager.hpp"

class Application : public Gtk::Application {
   public:
    Application();
    ~Application();

    static Glib::RefPtr<Application> create();
    Glib::RefPtr<Gio::Settings> settings;

    std::unique_ptr<PulseManager> pm;

   protected:
    void on_activate() override;

    int on_command_line(
        const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) override;

    void on_startup() override;

   private:
    std::string log_tag = "application.cpp: ";

    bool running_as_service;

    void create_appmenu();

    void create_presets_directory();
};

#endif
