#ifndef CROSSFEED_UI_HPP
#define CROSSFEED_UI_HPP

#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class CrossfeedUi : public Gtk::Grid, public PluginUiBase {
 public:
  CrossfeedUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name);
  CrossfeedUi(const CrossfeedUi&) = delete;
  auto operator=(const CrossfeedUi&) -> CrossfeedUi& = delete;
  CrossfeedUi(const CrossfeedUi&&) = delete;
  auto operator=(const CrossfeedUi &&) -> CrossfeedUi& = delete;
  ~CrossfeedUi() override;

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> fcut, feed;

  Gtk::Button *preset_cmoy = nullptr, *preset_default = nullptr, *preset_jmeier = nullptr;

  void init_presets_buttons();
};

#endif
