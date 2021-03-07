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

#include "stream_output_effects_ui.hpp"

StreamOutputEffectsUi::StreamOutputEffectsUi(BaseObjectType* cobject,
                                             const Glib::RefPtr<Gtk::Builder>& refBuilder,
                                             const Glib::RefPtr<Gio::Settings>& refSettings,
                                             StreamOutputEffects* soe_ptr)
    : Gtk::Box(cobject), EffectsBaseUi(refBuilder, refSettings, soe_ptr->pm), soe(soe_ptr) {
  // populate stack

  //   auto b_limiter = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/limiter.glade");
  //   auto b_compressor = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/compressor.glade");
  //   auto b_filter = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/filter.glade");
  //   auto b_equalizer = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/equalizer.glade");
  //   auto b_pitch = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/pitch.glade");
  //   auto b_reverb = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/reverb.glade");
  //   auto b_bass_enhancer =
  //   Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/bass_enhancer.glade"); auto b_exciter =
  //   Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/exciter.glade"); auto b_crossfeed =
  //   Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/crossfeed.glade"); auto b_maximizer =
  //   Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/maximizer.glade"); auto
  //   b_multiband_compressor =
  //       Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/multiband_compressor.glade");
  //   auto b_loudness = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/loudness.glade");
  //   auto b_gate = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/gate.glade");
  //   auto b_multiband_gate =
  //   Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/multiband_gate.glade"); auto b_deesser =
  //   Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/deesser.glade"); auto b_stereo_tools =
  //   Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/stereo_tools.glade"); auto b_convolver =
  //   Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/convolver.glade"); auto b_crystalizer =
  //   Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/crystalizer.glade"); auto b_autogain =
  //   Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/autogain.glade"); auto b_delay =
  //   Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/delay.glade"); auto b_rnnoise =
  //   Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/rnnoise.glade");

  //   b_limiter->get_widget_derived("widgets_grid", limiter_ui, "com.github.wwmm.pulseeffects.limiter",
  //                                 "/com/github/wwmm/pulseeffects/sinkinputs/limiter/");

  //   b_compressor->get_widget_derived("widgets_grid", compressor_ui, "com.github.wwmm.pulseeffects.compressor",
  //                                    "/com/github/wwmm/pulseeffects/sinkinputs/compressor/");

  //   b_filter->get_widget_derived("widgets_grid", filter_ui, "com.github.wwmm.pulseeffects.filter",
  //                                "/com/github/wwmm/pulseeffects/sinkinputs/filter/");

  //   b_equalizer->get_widget_derived("widgets_grid", equalizer_ui, "com.github.wwmm.pulseeffects.equalizer",
  //                                   "/com/github/wwmm/pulseeffects/sinkinputs/equalizer/",
  //                                   "com.github.wwmm.pulseeffects.equalizer.channel",
  //                                   "/com/github/wwmm/pulseeffects/sinkinputs/equalizer/leftchannel/",
  //                                   "/com/github/wwmm/pulseeffects/sinkinputs/equalizer/rightchannel/");

  //   b_pitch->get_widget_derived("widgets_grid", pitch_ui, "com.github.wwmm.pulseeffects.pitch",
  //                               "/com/github/wwmm/pulseeffects/sinkinputs/pitch/");

  //   b_reverb->get_widget_derived("widgets_grid", reverb_ui, "com.github.wwmm.pulseeffects.reverb",
  //                                "/com/github/wwmm/pulseeffects/sinkinputs/reverb/");

  //   b_bass_enhancer->get_widget_derived("widgets_grid", bass_enhancer_ui,
  //   "com.github.wwmm.pulseeffects.bassenhancer",
  //                                       "/com/github/wwmm/pulseeffects/sinkinputs/bassenhancer/");

  //   b_exciter->get_widget_derived("widgets_grid", exciter_ui, "com.github.wwmm.pulseeffects.exciter",
  //                                 "/com/github/wwmm/pulseeffects/sinkinputs/exciter/");

  //   b_crossfeed->get_widget_derived("widgets_grid", crossfeed_ui, "com.github.wwmm.pulseeffects.crossfeed",
  //                                   "/com/github/wwmm/pulseeffects/sinkinputs/crossfeed/");

  //   b_maximizer->get_widget_derived("widgets_grid", maximizer_ui, "com.github.wwmm.pulseeffects.maximizer",
  //                                   "/com/github/wwmm/pulseeffects/sinkinputs/maximizer/");

  //   b_multiband_compressor->get_widget_derived("widgets_grid", multiband_compressor_ui,
  //                                              "com.github.wwmm.pulseeffects.multibandcompressor",
  //                                              "/com/github/wwmm/pulseeffects/sinkinputs/multibandcompressor/");

  //   b_loudness->get_widget_derived("widgets_grid", loudness_ui, "com.github.wwmm.pulseeffects.loudness",
  //                                  "/com/github/wwmm/pulseeffects/sinkinputs/loudness/");

  //   b_gate->get_widget_derived("widgets_grid", gate_ui, "com.github.wwmm.pulseeffects.gate",
  //                              "/com/github/wwmm/pulseeffects/sinkinputs/gate/");

  //   b_multiband_gate->get_widget_derived("widgets_grid", multiband_gate_ui,
  //   "com.github.wwmm.pulseeffects.multibandgate",
  //                                        "/com/github/wwmm/pulseeffects/sinkinputs/multibandgate/");

  //   b_deesser->get_widget_derived("widgets_grid", deesser_ui, "com.github.wwmm.pulseeffects.deesser",
  //                                 "/com/github/wwmm/pulseeffects/sinkinputs/deesser/");

  //   b_stereo_tools->get_widget_derived("widgets_grid", stereo_tools_ui, "com.github.wwmm.pulseeffects.stereotools",
  //                                      "/com/github/wwmm/pulseeffects/sinkinputs/stereotools/");

  //   b_convolver->get_widget_derived("widgets_grid", convolver_ui, "com.github.wwmm.pulseeffects.convolver",
  //                                   "/com/github/wwmm/pulseeffects/sinkinputs/convolver/");

  //   b_crystalizer->get_widget_derived("widgets_grid", crystalizer_ui, "com.github.wwmm.pulseeffects.crystalizer",
  //                                     "/com/github/wwmm/pulseeffects/sinkinputs/crystalizer/");

  //   b_autogain->get_widget_derived("widgets_grid", autogain_ui, "com.github.wwmm.pulseeffects.autogain",
  //                                  "/com/github/wwmm/pulseeffects/sinkinputs/autogain/");

  //   b_delay->get_widget_derived("widgets_grid", delay_ui, "com.github.wwmm.pulseeffects.delay",
  //                               "/com/github/wwmm/pulseeffects/sinkinputs/delay/");

  //   b_rnnoise->get_widget_derived("widgets_grid", rnnoise_ui, "com.github.wwmm.pulseeffects.rnnoise",
  //                                 "/com/github/wwmm/pulseeffects/sinkinputs/rnnoise/");

  // add to stack

  //   stack->add(*limiter_ui, limiter_ui->name);
  //   stack->add(*compressor_ui, compressor_ui->name);
  //   stack->add(*filter_ui, filter_ui->name);
  //   stack->add(*equalizer_ui, equalizer_ui->name);
  //   stack->add(*pitch_ui, pitch_ui->name);
  //   stack->add(*reverb_ui, reverb_ui->name);
  //   stack->add(*bass_enhancer_ui, bass_enhancer_ui->name);
  //   stack->add(*exciter_ui, exciter_ui->name);
  //   stack->add(*crossfeed_ui, crossfeed_ui->name);
  //   stack->add(*maximizer_ui, maximizer_ui->name);
  //   stack->add(*multiband_compressor_ui, multiband_compressor_ui->name);
  //   stack->add(*loudness_ui, loudness_ui->name);
  //   stack->add(*gate_ui, gate_ui->name);
  //   stack->add(*multiband_gate_ui, multiband_gate_ui->name);
  //   stack->add(*deesser_ui, deesser_ui->name);
  //   stack->add(*stereo_tools_ui, stereo_tools_ui->name);
  //   stack->add(*convolver_ui, convolver_ui->name);
  //   stack->add(*crystalizer_ui, crystalizer_ui->name);
  //   stack->add(*autogain_ui, autogain_ui->name);
  //   stack->add(*delay_ui, delay_ui->name);
  //   stack->add(*rnnoise_ui, rnnoise_ui->name);

  // populate_listbox

  //   add_to_listbox(limiter_ui);
  //   add_to_listbox(compressor_ui);
  //   add_to_listbox(filter_ui);
  //   add_to_listbox(equalizer_ui);
  //   add_to_listbox(pitch_ui);
  //   add_to_listbox(reverb_ui);
  //   add_to_listbox(bass_enhancer_ui);
  //   add_to_listbox(exciter_ui);
  //   add_to_listbox(crossfeed_ui);
  //   add_to_listbox(maximizer_ui);
  //   add_to_listbox(multiband_compressor_ui);
  //   add_to_listbox(loudness_ui);
  //   add_to_listbox(gate_ui);
  //   add_to_listbox(multiband_gate_ui);
  //   add_to_listbox(deesser_ui);
  //   add_to_listbox(stereo_tools_ui);
  //   add_to_listbox(convolver_ui);
  //   add_to_listbox(crystalizer_ui);
  //   add_to_listbox(autogain_ui);
  //   add_to_listbox(delay_ui);
  //   add_to_listbox(rnnoise_ui);

  // show only speaker icon before "Application" label

  //   app_output_icon->set_visible(true);

  level_meters_connections();
  up_down_connections();

  for (auto& node : pm->list_nodes) {
    if (node.media_class == "Stream/Output/Audio") {
      on_app_added(node);
    }
  }

  //   connections.emplace_back(soe->new_spectrum.connect(sigc::mem_fun(*spectrum_ui, &SpectrumUi::on_new_spectrum)));

  connections.emplace_back(
      soe->pm->stream_output_added.connect(sigc::mem_fun(*this, &StreamOutputEffectsUi::on_app_added)));
  connections.emplace_back(
      soe->pm->stream_output_changed.connect(sigc::mem_fun(*this, &StreamOutputEffectsUi::on_app_changed)));
  connections.emplace_back(
      soe->pm->stream_output_removed.connect(sigc::mem_fun(*this, &StreamOutputEffectsUi::on_app_removed)));

  connections.emplace_back(soe->pm->sink_changed.connect([&](auto nd_info) {
    if (nd_info.id == soe->pm->pe_sink_node.id) {
      sink_state->set_text(node_state_to_string(soe->pm->pe_sink_node.state));

      std::ostringstream str_rate;

      str_rate.imbue(global_locale);
      str_rate.precision(1);

      str_rate << std::fixed << soe->pm->pe_sink_node.rate * 0.001F << " kHz";

      sink_rate->set_text(str_rate.str());
    }
  }));
}

StreamOutputEffectsUi::~StreamOutputEffectsUi() {
  soe->disable_spectrum();

  util::debug(log_tag + "destroyed");
}

auto StreamOutputEffectsUi::add_to_stack(Gtk::Stack* stack, StreamOutputEffects* soe_ptr) -> StreamOutputEffectsUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/effects_base.ui");

  auto settings = Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs");

  auto* ui = Gtk::Builder::get_widget_derived<StreamOutputEffectsUi>(builder, "top_box", settings, soe_ptr);

  auto stack_page = stack->add(*ui, "stream_output");

  stack_page->set_icon_name("audio-speakers-symbolic");

  return ui;
}

void StreamOutputEffectsUi::on_app_added(NodeInfo node_info) {
  // do not add the same stream twice

  for (guint n = 0; n < players_model->get_n_items(); n++) {
    auto item = players_model->get_item(n);

    if (item->info.id == node_info.id) {
      return;
    }
  }

  // Blocklist check

  auto forbidden_app = BlocklistSettingsUi::app_is_blocklisted(node_info.name, PresetType::output);

  if (forbidden_app) {
    node_info.visible_to_user = BlocklistSettingsUi::get_blocklisted_apps_visibility();

    if (!node_info.visible_to_user) {
      return;
    }
  } else {
    node_info.visible_to_user = true;
  }

  // auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/app_info.glade");

  // auto* appui = Gtk::Builder::get_widget_derived<AppInfoUi>(builder, "widgets_grid", node_info, pm);

  // apps_box->append(*appui);

  // apps_list.emplace_back(appui);

  players_model->append(NodeInfoHolder::create(node_info));
}

void StreamOutputEffectsUi::level_meters_connections() {
  // global output level meter connection

  connections.emplace_back(
      soe->global_output_level.connect(sigc::mem_fun(*this, &StreamOutputEffectsUi::on_new_output_level_db)));

  // limiter level meters connections

  //   connections.emplace_back(
  //       soe->limiter->input_level.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_input_level)));
  //   connections.emplace_back(
  //       soe->limiter->output_level.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_output_level)));
  //   connections.emplace_back(
  //       soe->limiter->attenuation.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_attenuation)));

  //   // compressor level meters connections

  //   connections.emplace_back(
  //       soe->compressor->input_level.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_input_level)));
  //   connections.emplace_back(
  //       soe->compressor->output_level.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_output_level)));
  //   connections.emplace_back(
  //       soe->compressor->reduction.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_reduction)));
  //   connections.emplace_back(
  //       soe->compressor->sidechain.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_sidechain)));
  //   connections.emplace_back(soe->compressor->curve.connect(sigc::mem_fun(*compressor_ui,
  //   &CompressorUi::on_new_curve)));

  //   // filter level meters connections

  //   connections.emplace_back(soe->filter->input_level.connect(sigc::mem_fun(*filter_ui,
  //   &FilterUi::on_new_input_level))); connections.emplace_back(
  //       soe->filter->output_level.connect(sigc::mem_fun(*filter_ui, &FilterUi::on_new_output_level)));

  //   // equalizer level meters connections

  //   connections.emplace_back(
  //       soe->equalizer_input_level.connect(sigc::mem_fun(*equalizer_ui, &EqualizerUi::on_new_input_level_db)));
  //   connections.emplace_back(
  //       soe->equalizer_output_level.connect(sigc::mem_fun(*equalizer_ui, &EqualizerUi::on_new_output_level_db)));

  //   // pitch level meters connections

  //   connections.emplace_back(soe->pitch_input_level.connect(sigc::mem_fun(*pitch_ui,
  //   &PitchUi::on_new_input_level_db)));
  //   connections.emplace_back(soe->pitch_output_level.connect(sigc::mem_fun(*pitch_ui,
  //   &PitchUi::on_new_output_level_db)));

  //   // reverb level meters connections

  //   connections.emplace_back(soe->reverb->input_level.connect(sigc::mem_fun(*reverb_ui,
  //   &ReverbUi::on_new_input_level))); connections.emplace_back(
  //       soe->reverb->output_level.connect(sigc::mem_fun(*reverb_ui, &ReverbUi::on_new_output_level)));

  //   // bass_enhancer level meters connections

  //   connections.emplace_back(
  //       soe->bass_enhancer_input_level.connect(sigc::mem_fun(*bass_enhancer_ui,
  //       &BassEnhancerUi::on_new_input_level_db)));
  //   connections.emplace_back(soe->bass_enhancer_output_level.connect(
  //       sigc::mem_fun(*bass_enhancer_ui, &BassEnhancerUi::on_new_output_level_db)));
  //   connections.emplace_back(
  //       soe->bass_enhancer->harmonics.connect(sigc::mem_fun(*bass_enhancer_ui,
  //       &BassEnhancerUi::on_new_harmonics_level)));

  //   // exciter level meters connections

  //   connections.emplace_back(
  //       soe->exciter_input_level.connect(sigc::mem_fun(*exciter_ui, &ExciterUi::on_new_input_level_db)));
  //   connections.emplace_back(
  //       soe->exciter_output_level.connect(sigc::mem_fun(*exciter_ui, &ExciterUi::on_new_output_level_db)));
  //   connections.emplace_back(
  //       soe->exciter->harmonics.connect(sigc::mem_fun(*exciter_ui, &ExciterUi::on_new_harmonics_level)));

  //   // crossfeed level meters connections

  //   connections.emplace_back(
  //       soe->crossfeed_input_level.connect(sigc::mem_fun(*crossfeed_ui, &CrossfeedUi::on_new_input_level_db)));
  //   connections.emplace_back(
  //       soe->crossfeed_output_level.connect(sigc::mem_fun(*crossfeed_ui, &CrossfeedUi::on_new_output_level_db)));

  //   // maximizer level meters connections

  //   connections.emplace_back(
  //       soe->maximizer_input_level.connect(sigc::mem_fun(*maximizer_ui, &MaximizerUi::on_new_input_level_db)));
  //   connections.emplace_back(
  //       soe->maximizer_output_level.connect(sigc::mem_fun(*maximizer_ui, &MaximizerUi::on_new_output_level_db)));
  //   connections.emplace_back(
  //       soe->maximizer->reduction.connect(sigc::mem_fun(*maximizer_ui, &MaximizerUi::on_new_reduction)));

  //   // multiband_compressor level meters connections

  //   connections.emplace_back(soe->multiband_compressor->input_level.connect(
  //       sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_input_level)));
  //   connections.emplace_back(soe->multiband_compressor->output_level.connect(
  //       sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_output_level)));

  //   connections.emplace_back(soe->multiband_compressor->output0.connect(
  //       sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_output0)));
  //   connections.emplace_back(soe->multiband_compressor->output1.connect(
  //       sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_output1)));
  //   connections.emplace_back(soe->multiband_compressor->output2.connect(
  //       sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_output2)));
  //   connections.emplace_back(soe->multiband_compressor->output3.connect(
  //       sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_output3)));

  //   connections.emplace_back(soe->multiband_compressor->compression0.connect(
  //       sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_compression0)));
  //   connections.emplace_back(soe->multiband_compressor->compression1.connect(
  //       sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_compression1)));
  //   connections.emplace_back(soe->multiband_compressor->compression2.connect(
  //       sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_compression2)));
  //   connections.emplace_back(soe->multiband_compressor->compression3.connect(
  //       sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_compression3)));

  //   // loudness level meters connections

  //   connections.emplace_back(
  //       soe->loudness_input_level.connect(sigc::mem_fun(*loudness_ui, &LoudnessUi::on_new_input_level_db)));
  //   connections.emplace_back(
  //       soe->loudness_output_level.connect(sigc::mem_fun(*loudness_ui, &LoudnessUi::on_new_output_level_db)));

  //   // gate level meters connections

  //   connections.emplace_back(soe->gate_input_level.connect(sigc::mem_fun(*gate_ui, &GateUi::on_new_input_level_db)));
  //   connections.emplace_back(soe->gate_output_level.connect(sigc::mem_fun(*gate_ui,
  //   &GateUi::on_new_output_level_db))); connections.emplace_back(soe->gate->gating.connect(sigc::mem_fun(*gate_ui,
  //   &GateUi::on_new_gating)));

  //   // multiband_gate level meters connections

  //   connections.emplace_back(soe->multiband_gate->input_level.connect(
  //       sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_input_level)));
  //   connections.emplace_back(soe->multiband_gate->output_level.connect(
  //       sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output_level)));

  //   connections.emplace_back(
  //       soe->multiband_gate->output0.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output0)));
  //   connections.emplace_back(
  //       soe->multiband_gate->output1.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output1)));
  //   connections.emplace_back(
  //       soe->multiband_gate->output2.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output2)));
  //   connections.emplace_back(
  //       soe->multiband_gate->output3.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output3)));

  //   connections.emplace_back(
  //       soe->multiband_gate->gating0.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating0)));
  //   connections.emplace_back(
  //       soe->multiband_gate->gating1.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating1)));
  //   connections.emplace_back(
  //       soe->multiband_gate->gating2.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating2)));
  //   connections.emplace_back(
  //       soe->multiband_gate->gating3.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating3)));

  //   // deesser level meters connections

  //   connections.emplace_back(
  //       soe->deesser_input_level.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_input_level_db)));
  //   connections.emplace_back(
  //       soe->deesser_output_level.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_output_level_db)));
  //   connections.emplace_back(
  //       soe->deesser->compression.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_compression)));
  //   connections.emplace_back(soe->deesser->detected.connect(sigc::mem_fun(*deesser_ui,
  //   &DeesserUi::on_new_detected)));

  //   // stereo_tools level meters connections

  //   connections.emplace_back(
  //       soe->stereo_tools->input_level.connect(sigc::mem_fun(*stereo_tools_ui, &StereoToolsUi::on_new_input_level)));
  //   connections.emplace_back(
  //       soe->stereo_tools->output_level.connect(sigc::mem_fun(*stereo_tools_ui,
  //       &StereoToolsUi::on_new_output_level)));

  //   // convolver level meters connections

  //   connections.emplace_back(
  //       soe->convolver_input_level.connect(sigc::mem_fun(*convolver_ui, &ConvolverUi::on_new_input_level_db)));
  //   connections.emplace_back(
  //       soe->convolver_output_level.connect(sigc::mem_fun(*convolver_ui, &ConvolverUi::on_new_output_level_db)));

  //   // crystalizer level meters connections

  //   connections.emplace_back(
  //       soe->crystalizer_input_level.connect(sigc::mem_fun(*crystalizer_ui, &CrystalizerUi::on_new_input_level_db)));
  //   connections.emplace_back(
  //       soe->crystalizer_output_level.connect(sigc::mem_fun(*crystalizer_ui,
  //       &CrystalizerUi::on_new_output_level_db)));
  //   connections.emplace_back(
  //       soe->crystalizer->range_before.connect(sigc::mem_fun(*crystalizer_ui, &CrystalizerUi::on_new_range_before)));
  //   connections.emplace_back(
  //       soe->crystalizer->range_after.connect(sigc::mem_fun(*crystalizer_ui, &CrystalizerUi::on_new_range_after)));

  //   // autogain level meters connections

  //   connections.emplace_back(
  //       soe->autogain_input_level.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_input_level_db)));
  //   connections.emplace_back(
  //       soe->autogain_output_level.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_output_level_db)));
  //   connections.emplace_back(
  //       soe->autogain->momentary.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_momentary)));
  //   connections.emplace_back(
  //       soe->autogain->shortterm.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_shortterm)));
  //   connections.emplace_back(
  //       soe->autogain->integrated.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_integrated)));
  //   connections.emplace_back(soe->autogain->relative.connect(sigc::mem_fun(*autogain_ui,
  //   &AutoGainUi::on_new_relative)));
  //   connections.emplace_back(soe->autogain->loudness.connect(sigc::mem_fun(*autogain_ui,
  //   &AutoGainUi::on_new_loudness)));
  //   connections.emplace_back(soe->autogain->range.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_range)));
  //   connections.emplace_back(soe->autogain->gain.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_gain)));

  //   // delay level meters connections

  //   connections.emplace_back(soe->delay_input_level.connect(sigc::mem_fun(*delay_ui,
  //   &DelayUi::on_new_input_level_db)));
  //   connections.emplace_back(soe->delay_output_level.connect(sigc::mem_fun(*delay_ui,
  //   &DelayUi::on_new_output_level_db)));

  //   // rnnoise level meters connections

  //   connections.emplace_back(
  //       soe->rnnoise_input_level.connect(sigc::mem_fun(*rnnoise_ui, &RNNoiseUi::on_new_input_level_db)));
  //   connections.emplace_back(
  //       soe->rnnoise_output_level.connect(sigc::mem_fun(*rnnoise_ui, &RNNoiseUi::on_new_output_level_db)));
}

void StreamOutputEffectsUi::up_down_connections() {
  //   auto on_up = [=](auto p) {
  //     auto order = Glib::Variant<std::vector<Glib::ustring>>();

  //     settings->get_value("plugins", order);

  //     auto vorder = order.get();

  //     auto r = std::find(std::begin(vorder), std::end(vorder), p->name);

  //     if (r != std::begin(vorder)) {
  //       std::iter_swap(r, r - 1);

  //       settings->set_string_array("plugins", vorder);
  //     }
  //   };

  //   auto on_down = [=](auto p) {
  //     auto order = Glib::Variant<std::vector<Glib::ustring>>();

  //     settings->get_value("plugins", order);

  //     auto vorder = order.get();

  //     auto r = std::find(std::begin(vorder), std::end(vorder), p->name);

  //     if (r != std::end(vorder) - 1) {
  //       std::iter_swap(r, r + 1);

  //       settings->set_string_array("plugins", vorder);
  //     }
  //   };

  //   connections.emplace_back(limiter_ui->plugin_up->signal_clicked().connect([=]() { on_up(limiter_ui); }));
  //   connections.emplace_back(limiter_ui->plugin_down->signal_clicked().connect([=]() { on_down(limiter_ui); }));

  //   connections.emplace_back(compressor_ui->plugin_up->signal_clicked().connect([=]() { on_up(compressor_ui); }));
  //   connections.emplace_back(compressor_ui->plugin_down->signal_clicked().connect([=]() { on_down(compressor_ui);
  //   }));

  //   connections.emplace_back(filter_ui->plugin_up->signal_clicked().connect([=]() { on_up(filter_ui); }));
  //   connections.emplace_back(filter_ui->plugin_down->signal_clicked().connect([=]() { on_down(filter_ui); }));

  //   connections.emplace_back(pitch_ui->plugin_up->signal_clicked().connect([=]() { on_up(pitch_ui); }));
  //   connections.emplace_back(pitch_ui->plugin_down->signal_clicked().connect([=]() { on_down(pitch_ui); }));

  //   connections.emplace_back(equalizer_ui->plugin_up->signal_clicked().connect([=]() { on_up(equalizer_ui); }));
  //   connections.emplace_back(equalizer_ui->plugin_down->signal_clicked().connect([=]() { on_down(equalizer_ui); }));

  //   connections.emplace_back(reverb_ui->plugin_up->signal_clicked().connect([=]() { on_up(reverb_ui); }));
  //   connections.emplace_back(reverb_ui->plugin_down->signal_clicked().connect([=]() { on_down(reverb_ui); }));

  //   connections.emplace_back(bass_enhancer_ui->plugin_up->signal_clicked().connect([=]() { on_up(bass_enhancer_ui);
  //   })); connections.emplace_back(
  //       bass_enhancer_ui->plugin_down->signal_clicked().connect([=]() { on_down(bass_enhancer_ui); }));

  //   connections.emplace_back(exciter_ui->plugin_up->signal_clicked().connect([=]() { on_up(exciter_ui); }));
  //   connections.emplace_back(exciter_ui->plugin_down->signal_clicked().connect([=]() { on_down(exciter_ui); }));

  //   connections.emplace_back(crossfeed_ui->plugin_up->signal_clicked().connect([=]() { on_up(crossfeed_ui); }));
  //   connections.emplace_back(crossfeed_ui->plugin_down->signal_clicked().connect([=]() { on_down(crossfeed_ui); }));

  //   connections.emplace_back(maximizer_ui->plugin_up->signal_clicked().connect([=]() { on_up(maximizer_ui); }));
  //   connections.emplace_back(maximizer_ui->plugin_down->signal_clicked().connect([=]() { on_down(maximizer_ui); }));

  //   connections.emplace_back(
  //       multiband_compressor_ui->plugin_up->signal_clicked().connect([=]() { on_up(multiband_compressor_ui); }));
  //   connections.emplace_back(
  //       multiband_compressor_ui->plugin_down->signal_clicked().connect([=]() { on_down(multiband_compressor_ui); }));

  //   connections.emplace_back(loudness_ui->plugin_up->signal_clicked().connect([=]() { on_up(loudness_ui); }));
  //   connections.emplace_back(loudness_ui->plugin_down->signal_clicked().connect([=]() { on_down(loudness_ui); }));

  //   connections.emplace_back(gate_ui->plugin_up->signal_clicked().connect([=]() { on_up(gate_ui); }));
  //   connections.emplace_back(gate_ui->plugin_down->signal_clicked().connect([=]() { on_down(gate_ui); }));

  //   connections.emplace_back(multiband_gate_ui->plugin_up->signal_clicked().connect([=]() { on_up(multiband_gate_ui);
  //   })); connections.emplace_back(
  //       multiband_gate_ui->plugin_down->signal_clicked().connect([=]() { on_down(multiband_gate_ui); }));

  //   connections.emplace_back(deesser_ui->plugin_up->signal_clicked().connect([=]() { on_up(deesser_ui); }));
  //   connections.emplace_back(deesser_ui->plugin_down->signal_clicked().connect([=]() { on_down(deesser_ui); }));

  //   connections.emplace_back(stereo_tools_ui->plugin_up->signal_clicked().connect([=]() { on_up(stereo_tools_ui);
  //   })); connections.emplace_back(stereo_tools_ui->plugin_down->signal_clicked().connect([=]() {
  //   on_down(stereo_tools_ui); }));

  //   connections.emplace_back(convolver_ui->plugin_up->signal_clicked().connect([=]() { on_up(convolver_ui); }));
  //   connections.emplace_back(convolver_ui->plugin_down->signal_clicked().connect([=]() { on_down(convolver_ui); }));

  //   connections.emplace_back(crystalizer_ui->plugin_up->signal_clicked().connect([=]() { on_up(crystalizer_ui); }));
  //   connections.emplace_back(crystalizer_ui->plugin_down->signal_clicked().connect([=]() { on_down(crystalizer_ui);
  //   }));

  //   connections.emplace_back(autogain_ui->plugin_up->signal_clicked().connect([=]() { on_up(autogain_ui); }));
  //   connections.emplace_back(autogain_ui->plugin_down->signal_clicked().connect([=]() { on_down(autogain_ui); }));

  //   connections.emplace_back(delay_ui->plugin_up->signal_clicked().connect([=]() { on_up(delay_ui); }));
  //   connections.emplace_back(delay_ui->plugin_down->signal_clicked().connect([=]() { on_down(delay_ui); }));

  //   connections.emplace_back(rnnoise_ui->plugin_up->signal_clicked().connect([=]() { on_up(rnnoise_ui); }));
  //   connections.emplace_back(rnnoise_ui->plugin_down->signal_clicked().connect([=]() { on_down(rnnoise_ui); }));
}
