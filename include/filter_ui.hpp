#ifndef FILTER_UI_HPP
#define FILTER_UI_HPP

#include <gtkmm/button.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class FilterUi : public Gtk::Grid, public PluginUiBase {
   public:
    FilterUi(BaseObjectType* cobject,
             const Glib::RefPtr<Gtk::Builder>& refBuilder,
             const std::string& settings_name);
    ~FilterUi();

    static std::shared_ptr<FilterUi> create(std::string settings_name);

    void reset();

   private:
    Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, frequency, resonance,
        inertia;

    Gtk::ComboBoxText* mode;
    Gtk::Button *preset_muted, *preset_disco, *preset_distant_headphones,
        *preset_default;

    void init_presets_buttons();
};

#endif
