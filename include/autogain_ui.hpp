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
    Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, target, weight_m,
        weight_s, weight_i;
    Gtk::LevelBar *m_level, *s_level, *i_level, *r_level, *g_level, *l_level;
    Gtk::Label *m_label, *s_label, *i_label, *r_label, *g_label, *l_label;
};

#endif
