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

#include "stream_output_effects_ui.hpp"

StreamOutputEffectsUi::StreamOutputEffectsUi(BaseObjectType* cobject,
                                             const Glib::RefPtr<Gtk::Builder>& refBuilder,
                                             Glib::RefPtr<Gtk::IconTheme> icon_ptr,
                                             StreamOutputEffects* soe_ptr,
                                             const std::string& schema)
    : Gtk::Box(cobject), EffectsBaseUi(refBuilder, icon_ptr, soe_ptr, schema), soe(soe_ptr) {
  for (const auto& [id, node] : pm->node_map) {
    if (node.media_class == "Stream/Output/Audio" && node.name != pm->loopback_output_name) {
      on_app_added(node.id, node.name);
    }
  }

  connections.emplace_back(
      soe->output_level->output_level.connect(sigc::mem_fun(*this, &StreamOutputEffectsUi::on_new_output_level_db)));

  connections.emplace_back(soe->spectrum->power.connect(sigc::mem_fun(*spectrum_ui, &SpectrumUi::on_new_spectrum)));

  connections.emplace_back(
      soe->pm->stream_output_added.connect(sigc::mem_fun(*this, &StreamOutputEffectsUi::on_app_added)));
  connections.emplace_back(
      soe->pm->stream_output_changed.connect(sigc::mem_fun(*this, &StreamOutputEffectsUi::on_app_changed)));
  connections.emplace_back(
      soe->pm->stream_output_removed.connect(sigc::mem_fun(*this, &StreamOutputEffectsUi::on_app_removed)));

  connections.emplace_back(soe->pm->sink_changed.connect([&](auto nd_info) {
    if (nd_info.id == soe->pm->ee_sink_node.id) {
      const auto& v = Glib::ustring::format(std::setprecision(1), std::fixed,
                                            static_cast<float>(soe->pm->ee_sink_node.rate) * 0.001F);

      device_state->set_text(v + " kHz" + Glib::ustring(5, ' '));
    }
  }));

  const auto& v =
      Glib::ustring::format(std::setprecision(1), std::fixed, static_cast<float>(soe->pm->ee_sink_node.rate) * 0.001F);

  device_state->set_text(v + " kHz" + Glib::ustring(5, ' '));
}

StreamOutputEffectsUi::~StreamOutputEffectsUi() {
  util::debug(log_tag + "destroyed");
}

auto StreamOutputEffectsUi::add_to_stack(Gtk::Stack* stack,
                                         StreamOutputEffects* soe_ptr,
                                         Glib::RefPtr<Gtk::IconTheme> icon_ptr) -> StreamOutputEffectsUi* {
  const auto& builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/effects_base.ui");

  auto* const ui = Gtk::Builder::get_widget_derived<StreamOutputEffectsUi>(builder, "top_box", icon_ptr, soe_ptr,
                                                                           "com.github.wwmm.easyeffects.streamoutputs");

  auto stack_page = stack->add(*ui, "stream_output");

  stack_page->set_title(_("Output"));

  return ui;
}
