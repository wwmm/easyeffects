/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects
 *
 *  EasyEffectsis free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffectsis distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef EQUALIZER_HPP
#define EQUALIZER_HPP

#include "lv2_wrapper.hpp"
#include "plugin_base.hpp"

class Equalizer : public PluginBase {
 public:
  Equalizer(const std::string& tag,
            const std::string& schema,
            const std::string& schema_path,
            const std::string& schema_channel,
            const std::string& schema_channel_left_path,
            const std::string& schema_channel_right_path,
            PipeManager* pipe_manager);
  Equalizer(const Equalizer&) = delete;
  auto operator=(const Equalizer&) -> Equalizer& = delete;
  Equalizer(const Equalizer&&) = delete;
  auto operator=(const Equalizer&&) -> Equalizer& = delete;
  ~Equalizer() override;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

  sigc::signal<void(double)> latency;

 private:
  Glib::RefPtr<Gio::Settings> settings_left, settings_right;

  std::unique_ptr<lv2::Lv2Wrapper> lv2_wrapper;

  const uint max_bands = 32U;

  uint latency_n_frames = 0U;

  void bind_band(const int& index);
};

#endif
