#ifndef MAXIMIZER_UI_HPP
#define MAXIMIZER_UI_HPP

#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class MaximizerUi : public Gtk::Grid, public PluginUiBase {
   public:
    MaximizerUi(BaseObjectType* cobject,
                const Glib::RefPtr<Gtk::Builder>& refBuilder,
                const std::string& settings_name);
    virtual ~MaximizerUi();

    static std::shared_ptr<MaximizerUi> create(std::string settings_name);

    void reset();

    void on_new_reduction(double value);

   private:
    Glib::RefPtr<Gtk::Adjustment> release, threshold, ceiling;
    Gtk::LevelBar* reduction;
    Gtk::Label* reduction_label;
};

#endif
