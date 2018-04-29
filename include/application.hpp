#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <gtkmm.h>

class Application : public Gtk::Application {
   public:
    Application();
    ~Application();

    static Glib::RefPtr<Application> create();
    Glib::RefPtr<Gio::Settings> settings;

   protected:
    void on_activate() override;

    int on_command_line(
        const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) override;

    void on_startup() override;

   private:
    bool running_as_service;

    void create_appmenu();
};

#endif
