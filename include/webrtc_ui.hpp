/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef WEBRTC_UI_HPP
#define WEBRTC_UI_HPP

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
