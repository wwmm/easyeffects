#ifndef CRYSTALIZER_UI_HPP
#define CRYSTALIZER_UI_HPP

#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class CrystalizerUi : public Gtk::Grid, public PluginUiBase {
   public:
    CrystalizerUi(BaseObjectType* cobject,
                  const Glib::RefPtr<Gtk::Builder>& builder,
                  const std::string& settings_name);
    virtual ~CrystalizerUi();

   private:
    Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, intensity;
};

#endif
