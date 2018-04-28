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
};

#endif
