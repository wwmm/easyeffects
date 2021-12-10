/*
 *  Copyright © 2017-2022 Wellington Wallace
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

#ifndef EFFECTS_BASE_UI_HPP
#define EFFECTS_BASE_UI_HPP

#include <glibmm/i18n.h>
#include "autogain_ui.hpp"
#include "bass_enhancer_ui.hpp"
#include "bass_loudness_ui.hpp"
#include "compressor_ui.hpp"
#include "convolver_ui.hpp"
#include "crossfeed_ui.hpp"
#include "crystalizer_ui.hpp"
#include "deesser_ui.hpp"
#include "delay_ui.hpp"
#include "echo_canceller_ui.hpp"
#include "effects_base.hpp"
#include "equalizer_ui.hpp"
#include "exciter_ui.hpp"
#include "filter_ui.hpp"
#include "gate_ui.hpp"
#include "info_holders.hpp"
#include "limiter_ui.hpp"
#include "loudness_ui.hpp"
#include "maximizer_ui.hpp"
#include "multiband_compressor_ui.hpp"
#include "multiband_gate_ui.hpp"
#include "pipe_manager.hpp"
#include "pitch_ui.hpp"
#include "preset_type.hpp"
#include "reverb_ui.hpp"
#include "rnnoise_ui.hpp"
#include "stereo_tools_ui.hpp"

class EffectsBaseUi {
 public:
  EffectsBaseUi(const Glib::RefPtr<Gtk::Builder>& builder,
                Glib::RefPtr<Gtk::IconTheme> icon_ptr,
                EffectsBase* effects_base,
                const std::string& schema);
  EffectsBaseUi(const EffectsBaseUi&) = delete;
  auto operator=(const EffectsBaseUi&) -> EffectsBaseUi& = delete;
  EffectsBaseUi(const EffectsBaseUi&&) = delete;
  auto operator=(const EffectsBaseUi&&) -> EffectsBaseUi& = delete;
  virtual ~EffectsBaseUi();

 protected:
  EffectsBase* effects_base = nullptr;

  const std::string log_tag = "effects_base_ui: ";

  std::string schema;

  Glib::RefPtr<Gio::Settings> settings;
  Glib::RefPtr<Gio::Settings> app_settings;

  Gtk::Stack* stack_plugins = nullptr;

  Gtk::Stack* stack_top = nullptr;

  Gtk::Window* transient_window = nullptr;

  PipeManager* pm = nullptr;

  std::vector<sigc::connection> connections;

 private:
  Gtk::ListView* listview_selected_plugins = nullptr;

  Glib::RefPtr<NodeInfoHolder> players_holder;

  Glib::RefPtr<Gtk::StringList> selected_plugins;

  /*
    enabled_app_list map saves the "enabled state" of processed apps regardless of their presence in the blocklist,
     useful to restore the enabled state when the app is removed from the blocklist
  */

  std::unordered_map<uint, bool> enabled_app_list;

  void add_plugins_to_stack_plugins();
};

#endif
