#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <gtkmm.h>

class Application : public Gtk::Application {
   public:
    Application();
    virtual ~Application();

    static Glib::RefPtr<Application> create();

   protected:
    void on_activate() override;

    int on_command_line(
        const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) override;

    void on_startup() override;

   private:
    bool running_as_service;
    Glib::RefPtr<Gio::Settings> settings;
};

#endif
