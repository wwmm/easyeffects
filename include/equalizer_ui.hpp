#ifndef EQUALIZER_UI_HPP
#define EQUALIZER_UI_HPP

#include <gtkmm/adjustment.h>
#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class EqualizerUi : public Gtk::Grid, public PluginUiBase {
   public:
    EqualizerUi(BaseObjectType* cobject,
                const Glib::RefPtr<Gtk::Builder>& refBuilder,
                std::string settings_name);
    ~EqualizerUi();

    static std::shared_ptr<EqualizerUi> create(std::string settings_name);

    void reset();

   private:
    Gtk::Grid* bands_grid;
    Gtk::Adjustment* nbands;
    Gtk::Button* reset_eq;

    std::vector<sigc::connection> connections;

    void on_nbands_changed();
};

#endif
