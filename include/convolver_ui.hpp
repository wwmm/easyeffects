#ifndef CONVOLVER_UI_HPP
#define CONVOLVER_UI_HPP

#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include <gtkmm/listbox.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/scrolledwindow.h>
#include "plugin_ui_base.hpp"

class ConvolverUi : public Gtk::Grid, public PluginUiBase {
   public:
    ConvolverUi(BaseObjectType* cobject,
                const Glib::RefPtr<Gtk::Builder>& builder,
                const std::string& settings_name);
    virtual ~ConvolverUi();

    void reset();

   private:
    Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain;
    Gtk::ListBox* irs_listbox;
    Gtk::MenuButton* irs_menu_button;
    Gtk::ScrolledWindow* irs_scrolled_window;
    Gtk::Button* import_irs;

    int on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2);

    void on_irs_menu_button_clicked();

    void on_import_irs_clicked();
};

#endif
