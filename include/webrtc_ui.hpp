#ifndef WEBRTC_UI_HPP
#define WEBRTC_UI_HPP

#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include <gtkmm/togglebutton.h>
#include "plugin_ui_base.hpp"

class WebrtcUi : public Gtk::Grid, public PluginUiBase {
 public:
  WebrtcUi(BaseObjectType* cobject,
           const Glib::RefPtr<Gtk::Builder>& builder,
           const std::string& schema,
           const std::string& schema_path);
  WebrtcUi(const WebrtcUi&) = delete;
  auto operator=(const WebrtcUi&) -> WebrtcUi& = delete;
  WebrtcUi(const WebrtcUi&&) = delete;
  auto operator=(const WebrtcUi &&) -> WebrtcUi& = delete;
  ~WebrtcUi() override;

  void reset() override;

 private:
  Glib::RefPtr<Gtk::Adjustment> compression_gain_db, target_level_dbfs, voice_detection_frame_size;

  Gtk::ToggleButton *echo_cancel = nullptr, *extended_filter = nullptr, *high_pass_filter = nullptr,
                    *delay_agnostic = nullptr, *noise_suppression = nullptr, *gain_control = nullptr,
                    *limiter = nullptr, *voice_detection = nullptr;

  Gtk::ComboBoxText *echo_suppression_level = nullptr, *noise_suppression_level = nullptr, *gain_control_mode = nullptr,
                    *voice_detection_likelihood = nullptr;
};

#endif
