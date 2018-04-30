#ifndef APPLICATION_WINDOW_HPP
#define APPLICATION_WINDOW_HPP

#include <gtkmm.h>
#include "application.hpp"

class ApplicationWindow {
   public:
    ApplicationWindow(Application* app);

    Gtk::DrawingArea* spectrum;

   private:
    std::string log_tag = "application_window.cpp: ";

    Application* app;

    Glib::RefPtr<Gtk::Builder> builder;

    Glib::RefPtr<Gio::Settings> settings;

    Gtk::ApplicationWindow* window;
    Gtk::Switch* enable_autostart;
    Gtk::Switch* enable_all_apps;
    Gtk::Switch* theme_switch;
    Gtk::ToggleButton* use_default_sink;
    Gtk::ToggleButton* use_default_source;
    Gtk::ComboBox* input_device;
    Gtk::ComboBox* output_device;
    Gtk::Adjustment* buffer_in;
    Gtk::Adjustment* buffer_out;
    Gtk::Adjustment* latency_in;
    Gtk::Adjustment* latency_out;
    Gtk::Switch* show_spectrum;
    Gtk::Adjustment* spectrum_n_points;
    Gtk::Button* reset_settings;

    double mouse_intensity;

    template <typename T>
    void get_object(std::string name, T& object) {
        object = (T)builder->get_object(name).get();
    }

    void apply_css_style(std::string css_file_name);

    void init_autostart_switch();

    bool on_enable_autostart(bool state, std::string tag);

    void on_reset_settings();

    bool on_draw(const Cairo::RefPtr<Cairo::Context>& ctx);

    bool on_enter_notify_event(GdkEventCrossing* event);

    bool on_leave_notify_event(GdkEventCrossing* event);

    bool on_motion_notify_event(GdkEventMotion* event, Gtk::DrawingArea* area);
};

#endif
