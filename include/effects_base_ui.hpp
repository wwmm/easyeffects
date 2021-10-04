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

#include <giomm.h>
#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <memory>
#include <vector>
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
#include "spectrum_ui.hpp"
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

  void set_transient_window(Gtk::Window* transient_window);

 protected:
  EffectsBase* effects_base = nullptr;

  inline static const std::string log_tag = "effects_base_ui: ";

  std::string schema;

  Glib::RefPtr<Gio::Settings> settings;

  // The Icon Theme object is needed to lookup app icon names

  Glib::RefPtr<Gtk::IconTheme> icon_theme;

  Gtk::Image *saturation_icon = nullptr, *toggle_plugins_icon = nullptr;

  Gtk::Label *global_output_level_left = nullptr, *global_output_level_right = nullptr;

  Gtk::Label *device_state = nullptr, *latency_status = nullptr;

  Gtk::ToggleButton *toggle_players = nullptr, *toggle_plugins = nullptr, *toggle_listen_mic = nullptr;

  Gtk::Stack* stack_plugins = nullptr;

  Gtk::Stack* stack_top = nullptr;

  Gtk::Window* transient_window = nullptr;

  SpectrumUi* spectrum_ui = nullptr;

  PipeManager* pm = nullptr;

  std::vector<sigc::connection> connections;

  void on_app_added(const NodeInfo);
  void on_app_changed(const util::time_point ts);
  void on_app_removed(const util::time_point ts);

  auto icon_available(const Glib::ustring& icon_name) -> bool;

  void on_new_output_level_db(const float& left, const float& right);

  static auto node_state_to_ustring(const pw_node_state& state) -> Glib::ustring;

 private:
  Gtk::ListView *listview_players = nullptr, *listview_blocklist = nullptr, *listview_plugins = nullptr,
                *listview_selected_plugins = nullptr;

  Gtk::Switch* show_blocklisted_apps = nullptr;

  Gtk::Button* button_add_to_blocklist = nullptr;

  Gtk::Text* blocklist_player_name = nullptr;

  Gtk::ScrolledWindow *blocklist_scrolled_window = nullptr, *scrolled_window_plugins = nullptr;

  Gtk::MenuButton *menubutton_blocklist = nullptr, *menubutton_select_plugin = nullptr;

  Gtk::Popover *popover_blocklist = nullptr, *popover_plugins = nullptr;

  Gtk::SearchEntry* entry_plugins_search = nullptr;

  Glib::RefPtr<NodeInfoHolder> players_holder;

  Glib::RefPtr<Gio::ListStore<NodeInfoHolder>> players_model, all_players_model;

  Glib::RefPtr<Gtk::StringList> blocklist, plugins, selected_plugins;

  std::map<Glib::ustring, Glib::ustring> plugins_names{{plugin_name::autogain, _("Autogain")},
                                                       {plugin_name::bass_enhancer, _("Bass Enhancer")},
                                                       {plugin_name::bass_loudness, _("Bass Loudness")},
                                                       {plugin_name::compressor, _("Compressor")},
                                                       {plugin_name::convolver, _("Convolver")},
                                                       {plugin_name::crossfeed, _("Crossfeed")},
                                                       {plugin_name::crystalizer, _("Crystalizer")},
                                                       {plugin_name::deesser, _("Deesser")},
                                                       {plugin_name::delay, _("Delay")},
                                                       {plugin_name::echo_canceller, _("Echo Canceller")},
                                                       {plugin_name::equalizer, _("Equalizer")},
                                                       {plugin_name::exciter, _("Exciter")},
                                                       {plugin_name::filter, _("Filter")},
                                                       {plugin_name::gate, _("Gate")},
                                                       {plugin_name::limiter, _("Limiter")},
                                                       {plugin_name::loudness, _("Loudness")},
                                                       {plugin_name::maximizer, _("Maximizer")},
                                                       {plugin_name::multiband_compressor, _("Multiband Compressor")},
                                                       {plugin_name::multiband_gate, _("Multiband Gate")},
                                                       {plugin_name::pitch, _("Pitch")},
                                                       {plugin_name::reverb, _("Reverberation")},
                                                       {plugin_name::rnnoise, _("Noise Reduction")},
                                                       {plugin_name::stereo_tools, _("Stereo Tools")}};

  /* enabled_app_list saves the "enabled state" of processed apps regardless of their presence in the blocklist,
     useful to restore the enabled state when the app is removed from the blocklist */

  std::map<uint, bool> enabled_app_list;

  void add_plugins_to_stack_plugins();

  void setup_listview_players();

  void setup_listview_blocklist();

  void setup_listview_plugins();

  void setup_listview_selected_plugins();

  auto get_app_icon_name(const NodeInfo& node_info) -> Glib::ustring;

  auto app_is_blocklisted(const Glib::ustring& name) -> bool;

  auto add_new_blocklist_entry(const Glib::ustring& name) -> bool;

  void remove_blocklist_entry(const Glib::ustring& name);

  void connect_stream(const uint& id, const std::string& media_class);

  void disconnect_stream(const uint& id, const std::string& media_class);
};

#endif
