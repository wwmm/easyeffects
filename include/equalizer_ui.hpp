#ifndef EQUALIZER_UI_HPP
#define EQUALIZER_UI_HPP

#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include <gtkmm/listbox.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/scrolledwindow.h>
#include "plugin_ui_base.hpp"

class EqualizerUi : public Gtk::Grid, public PluginUiBase {
   public:
    EqualizerUi(BaseObjectType* cobject,
                const Glib::RefPtr<Gtk::Builder>& builder,
                const std::string& settings_name);
    virtual ~EqualizerUi();

    void reset();

   private:
    Gtk::Grid* bands_grid;
    Glib::RefPtr<Gtk::Adjustment> nbands, input_gain, output_gain;
    Gtk::Button *reset_eq, *flat_response, *calculate_freqs;
    Gtk::ScrolledWindow* presets_scrolled_window;
    Gtk::ListBox* presets_listbox;
    Gtk::MenuButton* presets_menu_button;

    std::vector<sigc::connection> connections;

    std::string presets_path = "/com/github/wwmm/pulseeffects/presets/";

    void load_preset(const std::string& file_name);

    void on_nbands_changed();
    void on_flat_response();
    void on_calculate_frequencies();

    int on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2);
    void on_presets_menu_button_clicked();
    void populate_presets_listbox();
    void on_import_preset_clicked();
};

#endif
