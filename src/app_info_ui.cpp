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

#include "app_info_ui.hpp"
#include "pipewire/node.h"

AppInfoUi::AppInfoUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& builder,
                     NodeInfo node_info,
                     PipeManager* pulse_manager)
    : Gtk::Grid(cobject), nd_info(std::move(node_info)), pm(pulse_manager) {
  // loading glade widgets

  builder->get_widget("enable", enable);
  builder->get_widget("app_icon", app_icon);
  builder->get_widget("app_name", app_name);
  builder->get_widget("media_name", media_name);
  builder->get_widget("volume", volume);
  builder->get_widget("mute", mute);
  builder->get_widget("blocklist", blocklist);
  builder->get_widget("mute_icon", mute_icon);
  builder->get_widget("format", format);
  builder->get_widget("rate", rate);
  builder->get_widget("channels", channels);
  builder->get_widget("resampler", resampler);
  builder->get_widget("buffer", buffer);
  builder->get_widget("latency", latency);
  builder->get_widget("state", state);

  is_blocklisted = BlocklistSettingsUi::app_is_blocklisted(
      nd_info.name, (nd_info.media_class == "Stream/Output/Audio") ? PresetType::output : PresetType::input);

  is_enabled = nd_info.connected && !is_blocklisted;

  init_widgets();
  connect_signals();

  // if (app_info->app_type == "sink_input") {
  //   timeout_connection = Glib::signal_timeout().connect_seconds(
  //       [&]() {
  //         if (app_info != nullptr) {
  //           if (app_info->wants_to_play) {
  //             pm->get_sink_input_info(app_info->index);
  //           }
  //         }

  //         return running;
  //       },
  //       5);
  // }
}

AppInfoUi::~AppInfoUi() {
  running = false;
  timeout_connection.disconnect();

  util::debug(log_tag + nd_info.name + " info ui destroyed");
}

void AppInfoUi::init_widgets() {
  const float ms_factor = 0.001F;

  enable->set_active(is_enabled && !is_blocklisted);
  enable->set_sensitive(!is_blocklisted);

  blocklist->set_active(is_blocklisted);

  app_icon->set_from_icon_name(nd_info.icon_name, Gtk::ICON_SIZE_BUTTON);

  app_name->set_text(nd_info.name);

  if (nd_info.name == nd_info.media_name || nd_info.media_name.empty()) {
    media_name->set_visible(false);
  } else {
    media_name->set_visible(true);

    media_name->set_text(nd_info.media_name);
  }

  // volume->set_value(app_info->volume);

  // mute->set_active(app_info->mute != 0);

  // format->set_text(app_info->format);

  rate->set_text(std::to_string(nd_info.rate) + " Hz");

  channels->set_text(std::to_string(nd_info.n_output_ports));

  // resampler->set_text(app_info->resampler);

  // buffer->set_text(PluginUiBase::level_to_str(app_info->buffer * ms_factor, 1) + " ms");

  latency->set_text(float_to_localized_string(nd_info.latency, 2) + " ms");

  switch (nd_info.state) {
    case PW_NODE_STATE_RUNNING:
      state->set_text(_("playing"));

      break;
    case PW_NODE_STATE_SUSPENDED:
      state->set_text(_("suspended"));

      break;
    case PW_NODE_STATE_IDLE:
      state->set_text(_("idle"));

      break;
    case PW_NODE_STATE_CREATING:
      state->set_text(_("creating"));

      break;
    case PW_NODE_STATE_ERROR:
      state->set_text(_("error"));

      break;
    default:
      break;
  }
}

void AppInfoUi::connect_signals() {
  enable_connection = enable->signal_state_set().connect(sigc::mem_fun(*this, &AppInfoUi::on_enable_app), false);

  volume_connection = volume->signal_value_changed().connect(sigc::mem_fun(*this, &AppInfoUi::on_volume_changed));

  mute_connection = mute->signal_toggled().connect(sigc::mem_fun(*this, &AppInfoUi::on_mute));

  blocklist_connection = blocklist->signal_clicked().connect([=]() {
    PresetType preset_type = (nd_info.media_class == "Stream/Output/Audio") ? PresetType::output : PresetType::input;

    if (blocklist->get_active()) {
      // Add new entry to blocklist vector

      BlocklistSettingsUi::add_new_entry(nd_info.name, preset_type);

      pre_bl_state = is_enabled;

      is_blocklisted = true;

      if (is_enabled) {
        enable->set_active(false);
      }

      enable->set_sensitive(false);
    } else {
      // Remove app name entry from blocklist vector

      BlocklistSettingsUi::remove_entry(nd_info.name, preset_type);

      is_blocklisted = false;

      enable->set_sensitive(true);

      if (pre_bl_state) {
        enable->set_active(true);
      }
    }
  });
}

auto AppInfoUi::on_enable_app(bool state) -> bool {
  bool success = false;

  // if (state) {
  //   if (app_info->app_type == "sink_input") {
  //     success = pm->move_sink_input_to_pulseeffects(app_info->name, app_info->index);
  //   } else {
  //     success = pm->move_source_output_to_pulseeffects(app_info->name, app_info->index);
  //   }
  // } else {
  //   if (app_info->app_type == "sink_input") {
  //     success = pm->remove_sink_input_from_pulseeffects(app_info->name, app_info->index);
  //   } else {
  //     success = pm->remove_source_output_from_pulseeffects(app_info->name, app_info->index);
  //   }
  // }

  if (success) {
    is_enabled = state;
  }

  return false;
}

void AppInfoUi::on_volume_changed() {
  auto value = volume->get_value();

  // if (app_info->app_type == "sink_input") {
  //   pm->set_sink_input_volume(app_info->name, app_info->index, app_info->channels, value);
  // } else {
  //   pm->set_source_output_volume(app_info->name, app_info->index, app_info->channels, value);
  // }
}

void AppInfoUi::on_mute() {
  bool state = mute->get_active();

  if (state) {
    mute_icon->set_from_icon_name("audio-volume-muted-symbolic", Gtk::ICON_SIZE_BUTTON);

    volume->set_sensitive(false);
  } else {
    mute_icon->set_from_icon_name("audio-volume-high-symbolic", Gtk::ICON_SIZE_BUTTON);

    volume->set_sensitive(true);
  }

  // if (app_info->app_type == "sink_input") {
  //   pm->set_sink_input_mute(app_info->name, app_info->index, state);
  // } else {
  //   pm->set_source_output_mute(app_info->name, app_info->index, state);
  // }
}

void AppInfoUi::update(NodeInfo node_info) {
  nd_info = std::move(node_info);

  enable_connection.disconnect();
  volume_connection.disconnect();
  mute_connection.disconnect();
  blocklist_connection.disconnect();

  init_widgets();
  connect_signals();
}

auto AppInfoUi::float_to_localized_string(const float& value, const int& places) -> std::string {
  std::ostringstream msg;
  std::locale global_locale("");

  msg.imbue(global_locale);
  msg.precision(places);

  msg << std::fixed << value;

  return msg.str();
}
