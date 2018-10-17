#ifndef LOUDNESS_UI_HPP
#define LOUDNESS_UI_HPP

#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class LoudnessUi : public Gtk::Grid, public PluginUiBase {
 public:
  LoudnessUi(BaseObjectType* cobject,
             const Glib::RefPtr<Gtk::Builder>& builder,
             const std::string& settings_name);
  virtual ~LoudnessUi();

 private:
  Glib::RefPtr<Gtk::Adjustment> loudness, output, link;
};

#endif
