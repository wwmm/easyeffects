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

#include "stream_input_effects_ui.hpp"

StreamInputEffectsUi::StreamInputEffectsUi(BaseObjectType* cobject,
                                           const Glib::RefPtr<Gtk::Builder>& refBuilder,
                                           StreamInputEffects* sie_ptr,
                                           const std::string& schema)
    : Gtk::Box(cobject), EffectsBaseUi(refBuilder, sie_ptr, schema), sie(sie_ptr) {
  auto* toggle_players_icon = dynamic_cast<Gtk::Image*>(toggle_players->get_child()->get_first_child());
  auto* toggle_players_label = dynamic_cast<Gtk::Label*>(toggle_players_icon->get_next_sibling());

  toggle_players_icon->set_from_icon_name("media-record-symbolic");
  toggle_players_label->set_text(_("Recorders"));

  // populate stack

  //   auto b_deesser = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/deesser.glade");
  //   auto b_pitch = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/pitch.glade");
  //   auto b_webrtc = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/webrtc.glade");
  //   auto b_multiband_gate =
  //   Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/multiband_gate.glade");
  // auto b_rnnoise =
  //   Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/rnnoise.glade");

  //   b_deesser->get_widget_derived("widgets_grid", deesser_ui, "com.github.wwmm.pulseeffects.deesser",
  //                                 "/com/github/wwmm/pulseeffects/sourceoutputs/deesser/");

  //   b_pitch->get_widget_derived("widgets_grid", pitch_ui, "com.github.wwmm.pulseeffects.pitch",
  //                               "/com/github/wwmm/pulseeffects/sourceoutputs/pitch/");

  //   b_webrtc->get_widget_derived("widgets_grid", webrtc_ui, "com.github.wwmm.pulseeffects.webrtc",
  //                                "/com/github/wwmm/pulseeffects/sourceoutputs/webrtc/");

  //   b_rnnoise->get_widget_derived("widgets_grid", rnnoise_ui, "com.github.wwmm.pulseeffects.rnnoise",
  //                                 "/com/github/wwmm/pulseeffects/sourceoutputs/rnnoise/");

  //   // add to stack

  //   stack->add(*deesser_ui, deesser_ui->name);
  //   stack->add(*pitch_ui, pitch_ui->name);
  //   stack->add(*webrtc_ui, webrtc_ui->name);
  //   stack->add(*rnnoise_ui, rnnoise_ui->name);

  //   // populate listbox

  //   add_to_listbox(deesser_ui);
  //   add_to_listbox(pitch_ui);
  //   add_to_listbox(webrtc_ui);
  //   add_to_listbox(rnnoise_ui);

  level_meters_connections();
  up_down_connections();

  for (auto& node : pm->list_nodes) {
    if (node.media_class == "Stream/Input/Audio") {
      on_app_added(node);
    }
  }

  connections.emplace_back(sie->spectrum->power.connect(sigc::mem_fun(*spectrum_ui, &SpectrumUi::on_new_spectrum)));

  connections.emplace_back(
      sie->pm->stream_input_added.connect(sigc::mem_fun(*this, &StreamInputEffectsUi::on_app_added)));
  connections.emplace_back(
      sie->pm->stream_input_changed.connect(sigc::mem_fun(*this, &StreamInputEffectsUi::on_app_changed)));
  connections.emplace_back(
      sie->pm->stream_input_removed.connect(sigc::mem_fun(*this, &StreamInputEffectsUi::on_app_removed)));

  connections.emplace_back(sie->pm->source_changed.connect([&](auto nd_info) {
    if (nd_info.id == sie->pm->pe_source_node.id) {
      std::ostringstream str;

      str << node_state_to_string(sie->pm->pe_source_node.state) << std::string(5, ' ');

      str.precision(1);

      str << std::fixed << sie->pm->pe_source_node.rate * 0.001F << " kHz" << std::string(5, ' ');

      device_state->set_text(str.str());
    }
  }));

  std::ostringstream str;

  str << node_state_to_string(sie->pm->pe_source_node.state) << std::string(5, ' ');

  str.precision(1);

  str << std::fixed << sie->pm->pe_source_node.rate * 0.001F << " kHz" << std::string(5, ' ');

  device_state->set_text(str.str());
}

StreamInputEffectsUi::~StreamInputEffectsUi() {
  util::debug(log_tag + "destroyed");
}

auto StreamInputEffectsUi::add_to_stack(Gtk::Stack* stack, StreamInputEffects* sie_ptr) -> StreamInputEffectsUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/effects_base.ui");

  auto* ui = Gtk::Builder::get_widget_derived<StreamInputEffectsUi>(builder, "top_box", sie_ptr,
                                                                    "com.github.wwmm.pulseeffects.sourceoutputs");

  auto stack_page = stack->add(*ui, "stream_input");

  stack_page->set_title(_("Input"));

  return ui;
}

void StreamInputEffectsUi::level_meters_connections() {
  //   // global output level meter connection

  connections.emplace_back(
      sie->output_level->output_level.connect(sigc::mem_fun(*this, &StreamInputEffectsUi::on_new_output_level_db)));

  //   // deesser level meters connections

  //   connections.emplace_back(
  //       sie->deesser_input_level.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_input_level_db)));
  //   connections.emplace_back(
  //       sie->deesser_output_level.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_output_level_db)));
  //   connections.emplace_back(
  //       sie->deesser->compression.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_compression)));
  //   connections.emplace_back(sie->deesser->detected.connect(sigc::mem_fun(*deesser_ui,
  //   &DeesserUi::on_new_detected)));

  //   // pitch level meters connections

  //   connections.emplace_back(sie->pitch_input_level.connect(sigc::mem_fun(*pitch_ui,
  //   &PitchUi::on_new_input_level_db)));
  //   connections.emplace_back(sie->pitch_output_level.connect(sigc::mem_fun(*pitch_ui,
  //   &PitchUi::on_new_output_level_db)));

  //   // webrtc level meters connections

  //   connections.emplace_back(
  //       sie->webrtc_input_level.connect(sigc::mem_fun(*webrtc_ui, &WebrtcUi::on_new_input_level_db)));
  //   connections.emplace_back(
  //       sie->webrtc_output_level.connect(sigc::mem_fun(*webrtc_ui, &WebrtcUi::on_new_output_level_db)));

  //   // rnnoise level meters connections

  //   connections.emplace_back(
  //       sie->rnnoise_input_level.connect(sigc::mem_fun(*rnnoise_ui, &RNNoiseUi::on_new_input_level_db)));
  //   connections.emplace_back(
  //       sie->rnnoise_output_level.connect(sigc::mem_fun(*rnnoise_ui, &RNNoiseUi::on_new_output_level_db)));
}

void StreamInputEffectsUi::up_down_connections() {
  //   connections.emplace_back(deesser_ui->plugin_up->signal_clicked().connect([=, this]() { on_up(deesser_ui); }));
  //   connections.emplace_back(deesser_ui->plugin_down->signal_clicked().connect([=, this]() { on_down(deesser_ui);
  //   }));

  //   connections.emplace_back(pitch_ui->plugin_up->signal_clicked().connect([=, this]() { on_up(pitch_ui); }));
  //   connections.emplace_back(pitch_ui->plugin_down->signal_clicked().connect([=, this]() { on_down(pitch_ui); }));

  //   connections.emplace_back(webrtc_ui->plugin_up->signal_clicked().connect([=, this]() { on_up(webrtc_ui); }));
  //   connections.emplace_back(webrtc_ui->plugin_down->signal_clicked().connect([=, this]() { on_down(webrtc_ui); }));

  //   connections.emplace_back(rnnoise_ui->plugin_up->signal_clicked().connect([=, this]() { on_up(rnnoise_ui); }));
  //   connections.emplace_back(rnnoise_ui->plugin_down->signal_clicked().connect([=, this]() { on_down(rnnoise_ui);
  //   }));
}
