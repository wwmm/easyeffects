#ifndef AUTOGAIN_UI_HPP
#define AUTOGAIN_UI_HPP

#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class AutoGainUi : public Gtk::Grid, public PluginUiBase {
   public:
    AutoGainUi(BaseObjectType* cobject,
               const Glib::RefPtr<Gtk::Builder>& builder,
               const std::string& settings_name);
    virtual ~AutoGainUi();

    void reset();

   private:
    Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, window, target;
};

#endif
