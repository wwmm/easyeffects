#ifndef APPLICATION_WINDOW_HPP
#define APPLICATION_WINDOW_HPP

#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/listbox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/stack.h>
#include <gtkmm/switch.h>
#include <gtkmm/togglebutton.h>
#include "application.hpp"
#include "calibration_ui.hpp"
#include "sink_input_effects_ui.hpp"
#include "source_output_effects_ui.hpp"

class ApplicationUi : public Gtk::ApplicationWindow {
   public:
    ApplicationUi(BaseObjectType* cobject,
                  const Glib::RefPtr<Gtk::Builder>& refBuilder,
                  Application* application);

    virtual ~ApplicationUi();

    static ApplicationUi* create(Application* app);

   private:
    std::string log_tag = "application_ui: ";

    Application* app;

    Glib::RefPtr<Gtk::Builder> builder;
    Glib::RefPtr<Gio::Settings> settings;

    Gtk::Switch *enable_autostart, *enable_all_apps, *theme_switch,
        *show_spectrum, *use_custom_color;
    Gtk::ToggleButton *use_default_sink, *use_default_source;
    Gtk::ComboBox *input_device, *output_device;
    Gtk::Adjustment *buffer_in, *buffer_out, *latency_in, *latency_out,
        *spectrum_n_points;
    Gtk::DrawingArea* spectrum;
    Gtk::Box* spectrum_box;
    Gtk::Button *reset_settings, *add_preset, *import_preset,
        *calibration_button;
    Gtk::ListStore *sink_list, *source_list;
    Gtk::Stack* stack;
    Gtk::ListBox* presets_listbox;
    Gtk::MenuButton* presets_menu_button;
    Gtk::Label* presets_menu_label;
    Gtk::Entry* preset_name;
    Gtk::ColorButton* spectrum_color_button;
    Gdk::RGBA spectrum_color;

    sigc::connection spectrum_connection;
    std::vector<sigc::connection> connections;

    std::unique_ptr<SinkInputEffectsUi> sie_ui;
    std::unique_ptr<SourceOutputEffectsUi> soe_ui;

    bool mouse_inside;
    double mouse_intensity, mouse_freq;
    std::vector<float> spectrum_mag;

    template <typename T>
    void get_object(std::string name, T& object) {
        object = (T)builder->get_object(name).get();
    }

    void apply_css_style(std::string css_file_name);

    void init_autostart_switch();

    void clear_spectrum();

    void populate_presets_listbox();

    bool on_enable_autostart(bool state);

    void on_reset_settings();

    bool on_show_spectrum(bool state);

    bool on_use_custom_color(bool state);

    void on_new_spectrum(const std::vector<float>& magnitudes);

    bool on_spectrum_draw(const Cairo::RefPtr<Cairo::Context>& ctx);

    bool on_spectrum_enter_notify_event(GdkEventCrossing* event);

    bool on_spectrum_leave_notify_event(GdkEventCrossing* event);

    bool on_spectrum_motion_notify_event(GdkEventMotion* event);

    void on_stack_visible_child_changed();

    void on_sink_added(std::shared_ptr<mySinkInfo> info);

    void on_sink_removed(uint idx);

    void on_source_added(std::shared_ptr<mySourceInfo> info);

    void on_source_removed(uint idx);

    void on_use_default_sink_toggled();

    void on_use_default_source_toggled();

    void on_input_device_changed();

    void on_output_device_changed();

    int on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2);

    void on_presets_menu_button_clicked();

    void on_import_preset_clicked();
};

#endif
