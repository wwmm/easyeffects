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

#include "stream_input_effects_ui.hpp"

StreamInputEffectsUi::StreamInputEffectsUi(BaseObjectType* cobject,
                                           const Glib::RefPtr<Gtk::Builder>& refBuilder,
                                           Glib::RefPtr<Gtk::IconTheme> icon_ptr,
                                           StreamInputEffects* sie_ptr,
                                           const std::string& schema)
    : Gtk::Box(cobject), EffectsBaseUi(refBuilder, icon_ptr, sie_ptr, schema), sie(sie_ptr) {
  auto* toggle_players_icon = dynamic_cast<Gtk::Image*>(toggle_players->get_child()->get_first_child());
  auto* toggle_players_label = dynamic_cast<Gtk::Label*>(toggle_players_icon->get_next_sibling());

  toggle_players_icon->set_from_icon_name("media-record-symbolic");
  toggle_players_label->set_text(_("Recorders"));

  stack_top->connect_property_changed("visible-child", [=, this]() {
    const auto& child_name = stack_top->get_visible_child_name();

    toggle_listen_mic->set_visible(child_name != "page_players");
  });

  toggle_listen_mic->signal_toggled().connect([&, this]() { sie->set_listen_to_mic(toggle_listen_mic->get_active()); });

  for (const auto& [id, node] : pm->node_map) {
    if (node.media_class == "Stream/Input/Audio") {
      on_app_added(node.id, node.name);
    }
  }

  connections.emplace_back(
      sie->output_level->output_level.connect(sigc::mem_fun(*this, &StreamInputEffectsUi::on_new_output_level_db)));

  connections.emplace_back(sie->spectrum->power.connect(sigc::mem_fun(*spectrum_ui, &SpectrumUi::on_new_spectrum)));

  connections.emplace_back(
      sie->pm->stream_input_added.connect(sigc::mem_fun(*this, &StreamInputEffectsUi::on_app_added)));
  connections.emplace_back(
      sie->pm->stream_input_changed.connect(sigc::mem_fun(*this, &StreamInputEffectsUi::on_app_changed)));
  connections.emplace_back(
      sie->pm->stream_input_removed.connect(sigc::mem_fun(*this, &StreamInputEffectsUi::on_app_removed)));

  connections.emplace_back(sie->pm->source_changed.connect([&](auto nd_info) {
    // nd_info is a reference of a copy previously made

    if (nd_info.id == sie->pm->ee_source_node.id) {
      const auto& v = Glib::ustring::format(std::setprecision(1), std::fixed,
                                            static_cast<float>(sie->pm->ee_source_node.rate) * 0.001F);

      device_state->set_text(v + " kHz" + Glib::ustring(5, ' '));
    }
  }));

  const auto& v = Glib::ustring::format(std::setprecision(1), std::fixed,
                                        static_cast<float>(sie->pm->ee_source_node.rate) * 0.001F);

  device_state->set_text(v + " kHz" + Glib::ustring(5, ' '));
}

StreamInputEffectsUi::~StreamInputEffectsUi() {
  util::debug(log_tag + "destroyed");
}

auto StreamInputEffectsUi::add_to_stack(Gtk::Stack* stack,
                                        StreamInputEffects* sie_ptr,
                                        Glib::RefPtr<Gtk::IconTheme> icon_ptr) -> StreamInputEffectsUi* {
  const auto& builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/effects_base.ui");

  auto* const ui = Gtk::Builder::get_widget_derived<StreamInputEffectsUi>(builder, "top_box", icon_ptr, sie_ptr,
                                                                          "com.github.wwmm.easyeffects.streaminputs");

  auto stack_page = stack->add(*ui, "stream_input");

  stack_page->set_title(_("Input"));

  return ui;
}
