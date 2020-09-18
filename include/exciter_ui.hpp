#ifndef EXCITER_UI_HPP
#define EXCITER_UI_HPP

#include "plugin_ui_base.hpp"

class ExciterUi : public Gtk::Grid, public PluginUiBase {
 public:
  ExciterUi(BaseObjectType* cobject,
            const Glib::RefPtr<Gtk::Builder>& builder,
            const std::string& schema,
            const std::string& schema_path);
  ExciterUi(const ExciterUi&) = delete;
  auto operator=(const ExciterUi&) -> ExciterUi& = delete;
  ExciterUi(const ExciterUi&&) = delete;
  auto operator=(const ExciterUi &&) -> ExciterUi& = delete;
  ~ExciterUi() override;

  void on_new_harmonics_level(double value);

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> amount, blend, ceilv, harmonics, input_gain, output_gain, scope;

  Gtk::LevelBar* harmonics_levelbar = nullptr;
  Gtk::Label* harmonics_levelbar_label = nullptr;
  Gtk::SpinButton* ceil_freq = nullptr;
  Gtk::ToggleButton *ceil_active = nullptr, *listen = nullptr;
};

#endif
