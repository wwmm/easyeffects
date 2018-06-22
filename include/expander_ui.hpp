#ifndef EXPANDER_UI_HPP
#define EXPANDER_UI_HPP

#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include <gtkmm/togglebutton.h>
#include "plugin_ui_base.hpp"

class ExpanderUi : public Gtk::Grid, public PluginUiBase {
   public:
    ExpanderUi(BaseObjectType* cobject,
               const Glib::RefPtr<Gtk::Builder>& builder,
               const std::string& settings_name);
    virtual ~ExpanderUi();

    void reset();

   private:
    Gtk::ComboBoxText *em, *scm, *scs;
    Glib::RefPtr<Gtk::Adjustment> scr, scp, sla, cr, kn, mk, al, at, rrl, rt;
    Gtk::ToggleButton* scl;

    std::vector<sigc::connection> connections;
};

#endif
