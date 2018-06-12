#ifndef COMPRESSOR_UI_HPP
#define COMPRESSOR_UI_HPP

#include <gtkmm/button.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class CompressorUi : public Gtk::Grid, public PluginUiBase {
   public:
    CompressorUi(BaseObjectType* cobject,
                 const Glib::RefPtr<Gtk::Builder>& builder,
                 const std::string& settings_name);
    virtual ~CompressorUi();

    static std::shared_ptr<CompressorUi> create(std::string settings_name);

    void reset();

    void on_new_compression(double value);

   private:
    Glib::RefPtr<Gtk::Adjustment> attack, release, threshold, knee, ratio, mix,
        makeup;

    Gtk::LevelBar* compression;
    Gtk::Label* compression_label;
    Gtk::ComboBoxText *detection, *stereo_link;
    Gtk::Button *preset_vocal_leveller1, *preset_vocal_leveller2,
        *preset_default;

    void init_presets_buttons();
};

#endif
