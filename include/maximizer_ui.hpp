#ifndef MAXIMIZER_UI_HPP
#define MAXIMIZER_UI_HPP

#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class MaximizerUi : public Gtk::Grid, public PluginUiBase {
 public:
  MaximizerUi(BaseObjectType* cobject,
              const Glib::RefPtr<Gtk::Builder>& builder,
              const std::string& schema,
              const std::string& schema_path);
  MaximizerUi(const MaximizerUi&) = delete;
  auto operator=(const MaximizerUi&) -> MaximizerUi& = delete;
  MaximizerUi(const MaximizerUi&&) = delete;
  auto operator=(const MaximizerUi &&) -> MaximizerUi& = delete;
  ~MaximizerUi() override;

  void on_new_reduction(double value);

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> release, threshold, ceiling;
  Gtk::LevelBar* reduction = nullptr;
  Gtk::Label* reduction_label = nullptr;
};

#endif
