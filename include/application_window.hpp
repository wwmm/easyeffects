#ifndef APPLICATION_WINDOW_HPP
#define APPLICATION_WINDOW_HPP

#include <gtkmm.h>
#include "application.hpp"

class ApplicationWindow {
   public:
    ApplicationWindow(Application* app);

   private:
    Glib::RefPtr<Gtk::Builder> builder;

    Gtk::ApplicationWindow* window;
};

#endif
