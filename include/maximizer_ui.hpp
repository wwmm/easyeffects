#ifndef MAXIMIZER_UI_HPP
#define MAXIMIZER_UI_HPP

#include <gtkmm/adjustment.h>
#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class MaximizerUi : public Gtk::Grid, public PluginUiBase {
   public:
    MaximizerUi(BaseObjectType* cobject,
                const Glib::RefPtr<Gtk::Builder>& refBuilder,
                const std::string& settings_name);
    ~MaximizerUi();

    static std::shared_ptr<MaximizerUi> create(std::string settings_name);

    void reset();

    void on_new_reduction(double value);

   private:
    Gtk::Adjustment *release, *threshold, *ceiling;
    Gtk::LevelBar* reduction;
    Gtk::Label* reduction_label;
};

#endif
