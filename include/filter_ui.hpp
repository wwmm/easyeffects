#ifndef FILTER_UI_HPP
#define FILTER_UI_HPP

#include <gtkmm/adjustment.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class FilterUi : public Gtk::Grid, public PluginUiBase {
   public:
    FilterUi(BaseObjectType* cobject,
             const Glib::RefPtr<Gtk::Builder>& refBuilder,
             std::string settings_name);
    ~FilterUi();

    static std::shared_ptr<FilterUi> create(std::string settings_name);

    void reset();

   private:
    Gtk::Adjustment *input_gain, *output_gain, *frequency, *resonance, *inertia;
    Gtk::ComboBoxText* mode;
};

#endif
