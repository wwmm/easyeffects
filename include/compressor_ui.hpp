#ifndef COMPRESSOR_UI_HPP
#define COMPRESSOR_UI_HPP

#include <gtkmm/adjustment.h>
#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class CompressorUi : public Gtk::Grid, public PluginUiBase {
   public:
    CompressorUi(BaseObjectType* cobject,
                 const Glib::RefPtr<Gtk::Builder>& refBuilder,
                 std::string settings_name);
    ~CompressorUi();

    static std::shared_ptr<CompressorUi> create(std::string settings_name);

    void reset();

    void on_new_compression(double value);

   private:
    Gtk::Adjustment *attack, *release, *threshold, *knee, *ratio, *mix, *makeup;
    Gtk::LevelBar* compression;
    Gtk::Label* compression_label;
};

#endif
