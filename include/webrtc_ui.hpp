#ifndef WEBRTC_UI_HPP
#define WEBRTC_UI_HPP

#include <gtkmm/adjustment.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include <gtkmm/togglebutton.h>
#include "plugin_ui_base.hpp"

class WebrtcUi : public Gtk::Grid, public PluginUiBase {
   public:
    WebrtcUi(BaseObjectType* cobject,
             const Glib::RefPtr<Gtk::Builder>& refBuilder,
             std::string settings_name);
    ~WebrtcUi();

    static std::shared_ptr<WebrtcUi> create(std::string settings_name);

    void reset();

   private:
    Gtk::Adjustment *compression_gain_db, *target_level_dbfs,
        *voice_detection_frame_size;
    Gtk::ToggleButton *echo_cancel, *extended_filter, *high_pass_filter,
        *delay_agnostic, *noise_suppression, *gain_control, *limiter,
        *voice_detection;
    Gtk::ComboBoxText *echo_suppression_level, *noise_suppression_level,
        *gain_control_mode, *voice_detection_likelihood;
};

#endif
