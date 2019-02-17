#ifndef COMPRESSOR_UI_HPP
#define COMPRESSOR_UI_HPP

#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class CompressorUi : public Gtk::Grid, public PluginUiBase {
 public:
  CompressorUi(BaseObjectType* cobject,
               const Glib::RefPtr<Gtk::Builder>& builder,
               const std::string& settings_name);
  virtual ~CompressorUi();

  void on_new_reduction(double value);

 private:
  Glib::RefPtr<Gtk::Adjustment> attack, release, threshold, knee, ratio, makeup,
      preamp, reactivity, lookahead;

  Gtk::LevelBar *reduction, *sidechain;
  Gtk::Label *reduction_label, *sidechain_label;
  Gtk::ComboBoxText *compression_mode, *sidechain_type, *sidechain_mode,
      *sidechain_source;
};

#endif
