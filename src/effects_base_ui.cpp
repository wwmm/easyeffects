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

#include "effects_base_ui.hpp"

EffectsBaseUi::EffectsBaseUi(const Glib::RefPtr<Gtk::Builder>& builder,
                             Glib::RefPtr<Gio::Settings> refSettings,
                             PipeManager* pipe_manager)
    : settings(std::move(refSettings)), pm(pipe_manager) {
  // set locale (workaround for #849)

  try {
    global_locale = std::locale("");
  } catch (const std::exception& e) {
    global_locale = std::locale();
  }

  // loading builder widgets

  global_output_level_left = builder->get_widget<Gtk::Label>("global_output_level_left");
  global_output_level_right = builder->get_widget<Gtk::Label>("global_output_level_right");
  sink_state = builder->get_widget<Gtk::Label>("sink_state");
  saturation_icon = builder->get_widget<Gtk::Image>("saturation_icon");
  players_listview = builder->get_widget<Gtk::ListView>("players_listview");

  // stack = builder->get_widget<Gtk::Stack>("stack");
  // apps_box = builder->get_widget<Gtk::Box>("apps_box");
  // placeholder_spectrum = builder->get_widget<Gtk::Box>("placeholder_spectrum");

  // spectrum

  // spectrum_ui = SpectrumUi::add_to_box(placeholder_spectrum);

  // plugin rows connections

  // listbox->signal_row_activated().connect([&](auto row) { stack->set_visible_child(row->get_name()); });
}

EffectsBaseUi::~EffectsBaseUi() {
  for (auto& c : connections) {
    c.disconnect();
  }
}

void EffectsBaseUi::on_app_changed(NodeInfo node_info) {
  std::lock_guard<std::mutex> lock(apps_list_lock_guard);

  for (auto it = apps_list.begin(); it != apps_list.end(); it++) {
    auto n = it - apps_list.begin();

    if (apps_list[n]->nd_info.id == node_info.id) {
      apps_list[n]->update(node_info);

      break;
    }
  }
}

void EffectsBaseUi::on_app_removed(NodeInfo node_info) {
  std::lock_guard<std::mutex> lock(apps_list_lock_guard);

  for (auto it = apps_list.begin(); it != apps_list.end(); it++) {
    auto n = it - apps_list.begin();

    if (apps_list[n]->nd_info.id == node_info.id) {
      auto* appui = apps_list[n];

      apps_box->remove(*appui);

      apps_list.erase(it);

      break;
    }
  }
}

void EffectsBaseUi::on_new_output_level_db(const std::array<double, 2>& peak) {
  auto left = peak[0];
  auto right = peak[1];

  // global_level_meter_grid->set_visible(true);

  global_output_level_left->set_text(level_to_localized_string_showpos(left, 0));

  global_output_level_right->set_text(level_to_localized_string_showpos(right, 0));

  // saturation icon notification

  if (left > 0.0 || right > 0.0) {
    saturation_icon->set_visible(true);
  } else {
    saturation_icon->set_visible(false);
  }
}

auto EffectsBaseUi::node_state_to_string(const pw_node_state& state) -> std::string {
  switch (state) {
    case PW_NODE_STATE_RUNNING:
      return _("running");
    case PW_NODE_STATE_SUSPENDED:
      return _("suspended");
    case PW_NODE_STATE_IDLE:
      return _("idle");
    case PW_NODE_STATE_CREATING:
      return _("creating");
    case PW_NODE_STATE_ERROR:
      return _("error");
    default:
      return "";
  }
}
