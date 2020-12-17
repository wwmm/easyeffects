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

AppInfoUi::AppInfoUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& builder,
                     NodeInfo node_info,
                     PipeManager* pipe_manager)
    : Gtk::Grid(cobject), nd_info(std::move(node_info)), pm(pipe_manager) {
  try {
    global_locale = std::locale("");
  } catch (const std::exception& e) {
    global_locale = std::locale();
  }

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
  builder->get_widget("latency", latency);
  builder->get_widget("state", state);

  is_blocklisted = BlocklistSettingsUi::app_is_blocklisted(
      nd_info.name, (nd_info.media_class == "Stream/Output/Audio") ? PresetType::output : PresetType::input);

  init_widgets();
  connect_signals();
}

AppInfoUi::~AppInfoUi() {
  util::debug(log_tag + nd_info.name + " info ui destroyed");
}

void AppInfoUi::init_widgets() {
  is_enabled = false;

  if (nd_info.media_class == "Stream/Output/Audio") {
    for (const auto& link : pm->list_links) {
      if (link.output_node_id == nd_info.id && link.input_node_id == pm->pe_sink_node.id) {
        is_enabled = true;

        break;
      }
    }
  } else if (nd_info.media_class == "Stream/Input/Audio") {
    for (const auto& link : pm->list_links) {
      if (link.output_node_id == pm->pe_sink_node.id && link.input_node_id == nd_info.id) {
        is_enabled = true;

        break;
      }
    }
  }

  enable->set_active(is_enabled && !is_blocklisted);
  enable->set_sensitive(!is_blocklisted);

  blocklist->set_active(is_blocklisted);

  if (!nd_info.icon_name.empty()) {
    app_icon->set_from_icon_name(nd_info.icon_name, Gtk::ICON_SIZE_BUTTON);
  } else {
    app_icon->set_from_icon_name(nd_info.name, Gtk::ICON_SIZE_BUTTON);
  }

  app_name->set_text(nd_info.name);

  if (nd_info.name == nd_info.media_name || nd_info.media_name.empty()) {
    media_name->set_visible(false);
  } else {
    media_name->set_visible(true);

    media_name->set_text(nd_info.media_name);
  }

  // volume->set_value(nd_info.volume);

  mute->set_active(nd_info.mute);

  init_mute_widgets(nd_info.mute);

  // format->set_text(app_info->format);

  rate->set_text(std::to_string(nd_info.rate) + " Hz");

  channels->set_text(std::to_string(nd_info.n_output_ports));

  // resampler->set_text(app_info->resampler);

  latency->set_text(float_to_localized_string(nd_info.latency, 2) + " ms");

  switch (nd_info.state) {
    case PW_NODE_STATE_RUNNING:
      state->set_text(_("running"));

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

void AppInfoUi::init_mute_widgets(const bool& state) const {
  if (state) {
    mute_icon->set_from_icon_name("audio-volume-muted-symbolic", Gtk::ICON_SIZE_BUTTON);

    volume->set_sensitive(false);
  } else {
    mute_icon->set_from_icon_name("audio-volume-high-symbolic", Gtk::ICON_SIZE_BUTTON);

    volume->set_sensitive(true);
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
  if (state) {
    if (nd_info.media_class == "Stream/Output/Audio") {
      pm->connect_stream_output(nd_info);
    } else {
      //     success = pm->move_source_output_to_pulseeffects(app_info->name, app_info->index);
    }
  } else {
    if (nd_info.media_class == "Stream/Output/Audio") {
      pm->disconnect_stream_output(nd_info);
    } else {
      //     success = pm->remove_source_output_from_pulseeffects(app_info->name, app_info->index);
    }
  }

  return false;
}

void AppInfoUi::on_volume_changed() {
  // auto value = volume->get_value();

  // if (app_info->app_type == "sink_input") {
  //   pm->set_sink_input_volume(app_info->name, app_info->index, app_info->channels, value);
  // } else {
  //   pm->set_source_output_volume(app_info->name, app_info->index, app_info->channels, value);
  // }
}

void AppInfoUi::on_mute() {
  bool state = mute->get_active();

  init_mute_widgets(state);

  pm->set_node_mute(nd_info, state);
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

  msg.imbue(global_locale);
  msg.precision(places);

  msg << std::fixed << value;

  return msg.str();
}
