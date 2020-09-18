#ifndef DEESSER_UI_HPP
#define DEESSER_UI_HPP

#include "plugin_ui_base.hpp"

class DeesserUi : public Gtk::Grid, public PluginUiBase {
 public:
  DeesserUi(BaseObjectType* cobject,
            const Glib::RefPtr<Gtk::Builder>& builder,
            const std::string& schema,
            const std::string& schema_path);
  DeesserUi(const DeesserUi&) = delete;
  auto operator=(const DeesserUi&) -> DeesserUi& = delete;
  DeesserUi(const DeesserUi&&) = delete;
  auto operator=(const DeesserUi &&) -> DeesserUi& = delete;
  ~DeesserUi() override;

  void on_new_compression(double value);
  void on_new_detected(double value);

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> threshold, ratio, makeup, f1_freq, f2_freq, f1_level, f2_level, f2_q, laxity;

  Gtk::LevelBar *compression = nullptr, *detected = nullptr;
  Gtk::Label *compression_label = nullptr, *detected_label = nullptr;
  Gtk::ComboBoxText *detection = nullptr, *mode = nullptr;
  Gtk::ToggleButton* sc_listen = nullptr;
};

#endif
