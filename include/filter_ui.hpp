#ifndef FILTER_UI_HPP
#define FILTER_UI_HPP

#include <gtkmm/button.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class FilterUi : public Gtk::Grid, public PluginUiBase {
 public:
  FilterUi(BaseObjectType* cobject,
           const Glib::RefPtr<Gtk::Builder>& builder,
           const std::string& schema,
           const std::string& schema_path);
  FilterUi(const FilterUi&) = delete;
  auto operator=(const FilterUi&) -> FilterUi& = delete;
  FilterUi(const FilterUi&&) = delete;
  auto operator=(const FilterUi &&) -> FilterUi& = delete;
  ~FilterUi() override;

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> input_gain, output_gain, frequency, resonance, inertia;

  Gtk::ComboBoxText* mode = nullptr;
  Gtk::Button *preset_muted = nullptr, *preset_disco = nullptr, *preset_distant_headphones = nullptr,
              *preset_default = nullptr;

  void init_presets_buttons();
};

#endif
