#ifndef REVERB_UI_HPP
#define REVERB_UI_HPP

#include <gtkmm/adjustment.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class ReverbUi : public Gtk::Grid, public PluginUiBase {
   public:
    ReverbUi(BaseObjectType* cobject,
             const Glib::RefPtr<Gtk::Builder>& refBuilder,
             std::string settings_name);
    ~ReverbUi();

    static std::shared_ptr<ReverbUi> create(std::string settings_name);

    void reset();

   private:
    Gtk::Adjustment *input_gain, *output_gain;
    Gtk::ComboBoxText* room_size;
};

#endif
