/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef WEBRTC_HPP
#define WEBRTC_HPP

#include "plugin_base.hpp"

class Webrtc : public PluginBase {
 public:
  Webrtc(const std::string& tag,
         const std::string& schema,
         const std::string& schema_path,
         const int& sampling_rate,
         PipeManager* pipe_manager);
  Webrtc(const Webrtc&) = delete;
  auto operator=(const Webrtc&) -> Webrtc& = delete;
  Webrtc(const Webrtc&&) = delete;
  auto operator=(const Webrtc&&) -> Webrtc& = delete;
  ~Webrtc() override;

  int rate;

  void set_probe_input_node_id(const uint& id) const;

 private:
  void build_probe_bin();
  void build_dsp_bin();
  void bind_to_gsettings();
};

#endif
