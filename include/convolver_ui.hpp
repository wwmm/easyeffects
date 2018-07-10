#ifndef CONVOLVER_UI_HPP
#define CONVOLVER_UI_HPP

#include <gtkmm/button.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/grid.h>
#include <gtkmm/listbox.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/scrolledwindow.h>
#include <boost/filesystem.hpp>
#include "plugin_ui_base.hpp"

class ConvolverUi : public Gtk::Grid, public PluginUiBase {
   public:
    ConvolverUi(BaseObjectType* cobject,
                const Glib::RefPtr<Gtk::Builder>& builder,
                const std::string& settings_name);
    virtual ~ConvolverUi();

    void reset();

   private:
    std::string log_tag = "convolver_ui: ";

    Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, ir_width;
    Gtk::ListBox* irs_listbox;
    Gtk::MenuButton* irs_menu_button;
    Gtk::ScrolledWindow* irs_scrolled_window;
    Gtk::Button* import_irs;
    Gtk::ComboBoxText* buffersize;
    Gtk::DrawingArea *left_plot, *right_plot;

    std::vector<sigc::connection> connections;

    boost::filesystem::path irs_dir;

    bool mouse_inside = false;
    double mouse_intensity = 0, mouse_freq = 0;
    std::vector<float> left_mag, right_mag;

    std::vector<std::string> get_irs_names();

    void import_irs_file(const std::string& file_path);

    void remove_irs_file(const std::string& name);

    void populate_irs_listbox();

    int on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2);

    void on_irs_menu_button_clicked();

    void on_import_irs_clicked();

    void draw_channel(Gtk::DrawingArea* da,
                      const Cairo::RefPtr<Cairo::Context>& ctx,
                      const std::vector<float>& magnitudes);

    void update_mouse_info(GdkEventMotion* event,
                           const int& height,
                           const int& width);

    bool on_left_draw(const Cairo::RefPtr<Cairo::Context>& ctx);

    bool on_left_motion_notify_event(GdkEventMotion* event);

    bool on_right_draw(const Cairo::RefPtr<Cairo::Context>& ctx);

    bool on_right_motion_notify_event(GdkEventMotion* event);

    bool on_mouse_enter_notify_event(GdkEventCrossing* event);

    bool on_mouse_leave_notify_event(GdkEventCrossing* event);
};

#endif
