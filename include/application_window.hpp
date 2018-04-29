#ifndef APPLICATION_WINDOW_HPP
#define APPLICATION_WINDOW_HPP

#include <gtkmm.h>
#include "application.hpp"

class ApplicationWindow {
   public:
    ApplicationWindow(Application* app);

   private:
    Application* app;

    Glib::RefPtr<Gtk::Builder> builder;

    Gtk::ApplicationWindow* window;
    Gtk::Switch* enable_all_apps;
    Gtk::Switch* theme_switch;
    Gtk::ToggleButton* use_default_sink;
    Gtk::ToggleButton* use_default_source;
    Gtk::ComboBox* input_device;
    Gtk::ComboBox* output_device;
};

#endif
