#ifndef FILTER_UI_HPP
#define FILTER_UI_HPP

#include <gtkmm/grid.h>
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
};

#endif
