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

#include <utility>

NodeInfoHolder::NodeInfoHolder(NodeInfo info)
    : Glib::ObjectBase(typeid(NodeInfoHolder)), Glib::Object(), info(std::move(info)) {}

auto NodeInfoHolder::create(const NodeInfo& info) -> Glib::RefPtr<NodeInfoHolder> {
  return Glib::make_refptr_for_instance<NodeInfoHolder>(new NodeInfoHolder(info));
}

EffectsBaseUi::EffectsBaseUi(const Glib::RefPtr<Gtk::Builder>& builder,
                             Glib::RefPtr<Gio::Settings> refSettings,
                             PipeManager* pipe_manager)
    : settings(std::move(refSettings)), pm(pipe_manager), players_model(Gio::ListStore<NodeInfoHolder>::create()) {
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
  listview_players = builder->get_widget<Gtk::ListView>("listview_players");

  // configuring widgets

  setup_listview_players();

  // stack = builder->get_widget<Gtk::Stack>("stack");
  // placeholder_spectrum = builder->get_widget<Gtk::Box>("placeholder_spectrum");

  // spectrum

  // spectrum_ui = SpectrumUi::add_to_box(placeholder_spectrum);
}

EffectsBaseUi::~EffectsBaseUi() {
  for (auto& c : connections) {
    c.disconnect();
  }
}

void EffectsBaseUi::setup_listview_players() {
  // setting the listview model and factory

  listview_players->set_model(Gtk::NoSelection::create(players_model));

  auto factory = Gtk::SignalListItemFactory::create();

  listview_players->set_factory(factory);

  // setting the factory callbacks

  factory->signal_setup().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/app_info.ui");

    auto* top_box = b->get_widget<Gtk::Box>("top_box");

    list_item->set_data("enable", b->get_widget<Gtk::Switch>("enable"));
    list_item->set_data("app_name", b->get_widget<Gtk::Label>("app_name"));
    list_item->set_data("media_name", b->get_widget<Gtk::Label>("media_name"));
    list_item->set_data("blocklist", b->get_widget<Gtk::CheckButton>("blocklist"));
    list_item->set_data("format", b->get_widget<Gtk::Label>("format"));
    list_item->set_data("rate", b->get_widget<Gtk::Label>("rate"));
    list_item->set_data("channels", b->get_widget<Gtk::Label>("channels"));
    list_item->set_data("latency", b->get_widget<Gtk::Label>("latency"));
    list_item->set_data("state", b->get_widget<Gtk::Label>("state"));
    list_item->set_data("mute", b->get_widget<Gtk::ToggleButton>("mute"));
    list_item->set_data("scale_volume", b->get_widget<Gtk::Scale>("scale_volume"));
    list_item->set_data("volume", b->get_object<Gtk::Adjustment>("volume").get());

    list_item->set_child(*top_box);
  });

  factory->signal_bind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* app_name = static_cast<Gtk::Label*>(list_item->get_data("app_name"));
    auto* media_name = static_cast<Gtk::Label*>(list_item->get_data("media_name"));
    auto* format = static_cast<Gtk::Label*>(list_item->get_data("format"));
    auto* rate = static_cast<Gtk::Label*>(list_item->get_data("rate"));
    auto* channels = static_cast<Gtk::Label*>(list_item->get_data("channels"));
    auto* latency = static_cast<Gtk::Label*>(list_item->get_data("latency"));
    auto* state = static_cast<Gtk::Label*>(list_item->get_data("state"));
    auto* enable = static_cast<Gtk::Switch*>(list_item->get_data("enable"));
    auto* scale_volume = static_cast<Gtk::Scale*>(list_item->get_data("scale_volume"));

    auto holder = std::dynamic_pointer_cast<NodeInfoHolder>(list_item->get_item());

    auto connection_enable = enable->signal_state_set().connect(
        [=](bool state) {
          if (state) {
            if (holder->info.media_class == "Stream/Output/Audio") {
              pm->connect_stream_output(holder->info);
            } else if (holder->info.media_class == "Stream/Input/Audio") {
              pm->connect_stream_input(holder->info);
            }
          } else {
            if (holder->info.media_class == "Stream/Output/Audio") {
              pm->disconnect_stream_output(holder->info);
            } else if (holder->info.media_class == "Stream/Input/Audio") {
              pm->disconnect_stream_input(holder->info);
            }
          }
          return false;
        },
        false);

    auto* pointer_connection_enable = new sigc::connection(connection_enable);

    auto connection_info = holder->info_updated.connect([=](const NodeInfo& i) {
      app_name->set_text(i.name);
      media_name->set_text(i.media_name);
      format->set_text(i.format);
      rate->set_text(std::to_string(i.rate) + " Hz");
      channels->set_text(std::to_string(i.n_volume_channels));
      latency->set_text(float_to_localized_string(i.latency, 2) + " s");

      switch (i.state) {
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

      pointer_connection_enable->block();

      bool is_enabled = false;

      if (holder->info.media_class == "Stream/Output/Audio") {
        for (const auto& link : pm->list_links) {
          if (link.output_node_id == holder->info.id && link.input_node_id == pm->pe_sink_node.id) {
            is_enabled = true;

            break;
          }
        }
      } else if (holder->info.media_class == "Stream/Input/Audio") {
        for (const auto& link : pm->list_links) {
          if (link.output_node_id == pm->pe_source_node.id && link.input_node_id == holder->info.id) {
            is_enabled = true;

            break;
          }
        }
      }

      enable->set_active(is_enabled);
      // enable->set_active(is_enabled && !is_blocklisted);
      // enable->set_sensitive(!is_blocklisted);

      // blocklist->set_active(is_blocklisted);

      pointer_connection_enable->unblock();
    });

    scale_volume->set_format_value_func([=](double v) { return std::to_string(static_cast<int>(v)) + " %"; });

    holder->info_updated.emit(holder->info);

    list_item->set_data("connection_enable", pointer_connection_enable, Glib::destroy_notify_delete<sigc::connection>);

    list_item->set_data("connection_info", new sigc::connection(connection_info),
                        Glib::destroy_notify_delete<sigc::connection>);
  });

  factory->signal_unbind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    for (const auto* conn : {"connection_enable", "connection_info"}) {
      if (auto* connection = static_cast<sigc::connection*>(list_item->get_data(conn))) {
        connection->disconnect();

        list_item->set_data(conn, nullptr);
      }
    }
  });
}

void EffectsBaseUi::on_app_changed(NodeInfo node_info) {
  for (guint n = 0; n < players_model->get_n_items(); n++) {
    auto* item = players_model->get_item(n).get();

    if (item->info.id == node_info.id) {
      item->info = node_info;
      item->info_updated.emit(node_info);

      break;
    }
  }
}

void EffectsBaseUi::on_app_removed(NodeInfo node_info) {
  for (guint n = 0; n < players_model->get_n_items(); n++) {
    auto item = players_model->get_item(n);

    if (item->info.id == node_info.id) {
      players_model->remove(n);

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

auto EffectsBaseUi::float_to_localized_string(const float& value, const int& places) -> std::string {
  std::ostringstream msg;

  msg.imbue(global_locale);
  msg.precision(places);

  msg << std::fixed << value;

  return msg.str();
}
