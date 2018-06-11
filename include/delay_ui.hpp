#ifndef DELAY_UI_HPP
#define DELAY_UI_HPP

#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class DelayUi : public Gtk::Grid, public PluginUiBase {
   public:
    DelayUi(BaseObjectType* cobject,
            const Glib::RefPtr<Gtk::Builder>& refBuilder,
            const std::string& settings_name);
    virtual ~DelayUi();

    static std::shared_ptr<DelayUi> create(std::string settings_name);

    void reset();

    void on_new_tempo(std::array<double, 2> value);

   private:
    Glib::RefPtr<Gtk::Adjustment> m_l, m_r, cm_l, cm_r, temperature;
    Gtk::Label *d_l, *d_r;
};

#endif
