#ifndef EQUALIZER_UI_HPP
#define EQUALIZER_UI_HPP

#include <gtkmm/button.h>
#include <gtkmm/grid.h>
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

    std::vector<sigc::connection> connections;

    void on_nbands_changed();
    void on_flat_response();
    void on_calculate_frequencies();
};

#endif
