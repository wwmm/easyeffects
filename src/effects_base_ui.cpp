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

#include "effects_base_ui.hpp"

EffectsBaseUi::EffectsBaseUi(const Glib::RefPtr<Gtk::Builder>& builder,
                             Glib::RefPtr<Gtk::IconTheme> icon_ptr,
                             EffectsBase* effects_base,
                             const std::string& schema)
    : effects_base(effects_base),
      schema(schema),
      settings(Gio::Settings::create(schema)),
      app_settings(Gio::Settings::create("com.github.wwmm.easyeffects")),
      pm(effects_base->pm),
      selected_plugins(Gtk::StringList::create({"initial_value"})) {
  // loading builder widgets

  stack_top = builder->get_widget<Gtk::Stack>("stack_top");

  popover_plugins = builder->get_widget<Gtk::Popover>("popover_plugins");
  scrolled_window_plugins = builder->get_widget<Gtk::ScrolledWindow>("scrolled_window_plugins");
  listview_selected_plugins = builder->get_widget<Gtk::ListView>("listview_selected_plugins");
  stack_plugins = builder->get_widget<Gtk::Stack>("stack_plugins");

  add_plugins_to_stack_plugins();

  // configuring widgets

  setup_listview_selected_plugins();

  settings->signal_changed("plugins").connect([&, this](const auto& key) { add_plugins_to_stack_plugins(); });

  // signals connections

  popover_plugins->signal_show().connect([=, this]() {
    const int height = static_cast<int>(0.5F * static_cast<float>(stack_top->get_allocated_height()));

    scrolled_window_plugins->set_max_content_height(height);
  });

  // enabling notifications

  effects_base->autogain->post_messages = true;
  effects_base->bass_enhancer->post_messages = true;
  effects_base->bass_loudness->post_messages = true;
  effects_base->compressor->post_messages = true;
  effects_base->convolver->post_messages = true;
  effects_base->crossfeed->post_messages = true;
  effects_base->crystalizer->post_messages = true;
  effects_base->deesser->post_messages = true;
  effects_base->delay->post_messages = true;
  effects_base->echo_canceller->post_messages = true;
  effects_base->equalizer->post_messages = true;
  effects_base->exciter->post_messages = true;
  effects_base->filter->post_messages = true;
  effects_base->gate->post_messages = true;
  effects_base->limiter->post_messages = true;
  effects_base->loudness->post_messages = true;
  effects_base->maximizer->post_messages = true;
  effects_base->multiband_compressor->post_messages = true;
  effects_base->multiband_gate->post_messages = true;
  effects_base->output_level->post_messages = true;
  effects_base->pitch->post_messages = true;
  effects_base->reverb->post_messages = true;
  effects_base->rnnoise->post_messages = true;
  effects_base->stereo_tools->post_messages = true;
}

EffectsBaseUi::~EffectsBaseUi() {
  util::debug("effects_base_ui: destroyed");

  for (auto& c : connections) {
    c.disconnect();
  }

  // do not send notifications when the window is closed

  effects_base->autogain->post_messages = false;
  effects_base->bass_enhancer->post_messages = false;
  effects_base->bass_loudness->post_messages = false;
  effects_base->compressor->post_messages = false;
  effects_base->convolver->post_messages = false;
  effects_base->crossfeed->post_messages = false;
  effects_base->crystalizer->post_messages = false;
  effects_base->deesser->post_messages = false;
  effects_base->delay->post_messages = false;
  effects_base->echo_canceller->post_messages = false;
  effects_base->equalizer->post_messages = false;
  effects_base->exciter->post_messages = false;
  effects_base->filter->post_messages = false;
  effects_base->gate->post_messages = false;
  effects_base->limiter->post_messages = false;
  effects_base->loudness->post_messages = false;
  effects_base->maximizer->post_messages = false;
  effects_base->multiband_compressor->post_messages = false;
  effects_base->multiband_gate->post_messages = false;
  effects_base->output_level->post_messages = false;
  effects_base->pitch->post_messages = false;
  effects_base->reverb->post_messages = false;
  effects_base->rnnoise->post_messages = false;
  effects_base->spectrum->post_messages = false;
  effects_base->stereo_tools->post_messages = false;

  // disabling bypass when closing the window

  effects_base->autogain->bypass = false;
  effects_base->bass_enhancer->bypass = false;
  effects_base->bass_loudness->bypass = false;
  effects_base->compressor->bypass = false;
  effects_base->convolver->bypass = false;
  effects_base->crossfeed->bypass = false;
  effects_base->crystalizer->bypass = false;
  effects_base->deesser->bypass = false;
  effects_base->delay->bypass = false;
  effects_base->echo_canceller->bypass = false;
  effects_base->equalizer->bypass = false;
  effects_base->exciter->bypass = false;
  effects_base->filter->bypass = false;
  effects_base->gate->bypass = false;
  effects_base->limiter->bypass = false;
  effects_base->loudness->bypass = false;
  effects_base->maximizer->bypass = false;
  effects_base->multiband_compressor->bypass = false;
  effects_base->multiband_gate->bypass = false;
  effects_base->output_level->bypass = false;
  effects_base->pitch->bypass = false;
  effects_base->reverb->bypass = false;
  effects_base->rnnoise->bypass = false;
  effects_base->spectrum->bypass = false;
  effects_base->stereo_tools->bypass = false;
}

void EffectsBaseUi::add_plugins_to_stack_plugins() {
  std::string path = "/" + schema + "/";

  std::replace(path.begin(), path.end(), '.', '/');

  // removing plugins that are not in the list

  for (auto* child = stack_plugins->get_first_child(); child != nullptr;) {
    auto found = false;

    for (const auto& name : settings->get_string_array("plugins")) {
      if (name == stack_plugins->get_page(*child)->get_name()) {
        found = true;

        break;
      }
    }

    auto* next_child = child->get_next_sibling();

    if (!found) {
      stack_plugins->remove(*child);
    }

    child = next_child;
  }

  // Adding to the stack the plugins in the list that are not there yet

  for (const auto& name : settings->get_string_array("plugins")) {
    auto found = false;

    for (auto* child = stack_plugins->get_first_child(); child != nullptr; child = child->get_next_sibling()) {
      if (name == stack_plugins->get_page(*child)->get_name()) {
        found = true;

        break;
      }
    }

    if (found) {
      continue;
    }

    if (name == plugin_name::autogain) {
      auto* const autogain_ui = AutoGainUi::add_to_stack(stack_plugins, path);

      autogain_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->autogain->bypass = autogain_ui->bypass->get_active(); });

      effects_base->autogain->input_level.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_input_level));
      effects_base->autogain->output_level.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_output_level));
      effects_base->autogain->results.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_results));

      effects_base->autogain->bypass = false;
    } else if (name == plugin_name::bass_enhancer) {
      auto* const bass_enhancer_ui = BassEnhancerUi::add_to_stack(stack_plugins, path);

      bass_enhancer_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->bass_enhancer->bypass = bass_enhancer_ui->bypass->get_active(); });

      effects_base->bass_enhancer->input_level.connect(
          sigc::mem_fun(*bass_enhancer_ui, &BassEnhancerUi::on_new_input_level));
      effects_base->bass_enhancer->output_level.connect(
          sigc::mem_fun(*bass_enhancer_ui, &BassEnhancerUi::on_new_output_level));
      effects_base->bass_enhancer->harmonics.connect(
          sigc::mem_fun(*bass_enhancer_ui, &BassEnhancerUi::on_new_harmonics_level));

      effects_base->bass_enhancer->bypass = false;
    } else if (name == plugin_name::bass_loudness) {
      auto* const bass_loudness_ui = BassLoudnessUi::add_to_stack(stack_plugins, path);

      bass_loudness_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->bass_loudness->bypass = bass_loudness_ui->bypass->get_active(); });

      effects_base->bass_loudness->input_level.connect(
          sigc::mem_fun(*bass_loudness_ui, &BassLoudnessUi::on_new_input_level));
      effects_base->bass_loudness->output_level.connect(
          sigc::mem_fun(*bass_loudness_ui, &BassLoudnessUi::on_new_output_level));

      effects_base->bass_loudness->bypass = false;
    } else if (name == plugin_name::compressor) {
      auto* const compressor_ui = CompressorUi::add_to_stack(stack_plugins, path);

      compressor_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->compressor->bypass = compressor_ui->bypass->get_active(); });

      compressor_ui->set_pipe_manager_ptr(pm);

      effects_base->compressor->input_level.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_input_level));
      effects_base->compressor->output_level.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_output_level));
      effects_base->compressor->reduction.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_reduction));
      effects_base->compressor->envelope.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_envelope));
      effects_base->compressor->sidechain.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_sidechain));
      effects_base->compressor->curve.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_curve));

      effects_base->compressor->bypass = false;
    } else if (name == plugin_name::convolver) {
      auto* const convolver_ui = ConvolverUi::add_to_stack(stack_plugins, path);

      convolver_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->convolver->bypass = convolver_ui->bypass->get_active(); });

      convolver_ui->set_transient_window(transient_window);

      effects_base->convolver->input_level.connect(sigc::mem_fun(*convolver_ui, &ConvolverUi::on_new_input_level));
      effects_base->convolver->output_level.connect(sigc::mem_fun(*convolver_ui, &ConvolverUi::on_new_output_level));

      effects_base->convolver->bypass = false;
    } else if (name == plugin_name::crossfeed) {
      auto* const crossfeed_ui = CrossfeedUi::add_to_stack(stack_plugins, path);

      crossfeed_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->crossfeed->bypass = crossfeed_ui->bypass->get_active(); });

      effects_base->crossfeed->input_level.connect(sigc::mem_fun(*crossfeed_ui, &CrossfeedUi::on_new_input_level));
      effects_base->crossfeed->output_level.connect(sigc::mem_fun(*crossfeed_ui, &CrossfeedUi::on_new_output_level));

      effects_base->crossfeed->bypass = false;
    } else if (name == plugin_name::crystalizer) {
      auto* const crystalizer_ui = CrystalizerUi::add_to_stack(stack_plugins, path);

      crystalizer_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->crystalizer->bypass = crystalizer_ui->bypass->get_active(); });

      effects_base->crystalizer->input_level.connect(
          sigc::mem_fun(*crystalizer_ui, &CrystalizerUi::on_new_input_level));
      effects_base->crystalizer->output_level.connect(
          sigc::mem_fun(*crystalizer_ui, &CrystalizerUi::on_new_output_level));

      effects_base->crystalizer->bypass = false;
    } else if (name == plugin_name::deesser) {
      auto* const deesser_ui = DeesserUi::add_to_stack(stack_plugins, path);

      deesser_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->deesser->bypass = deesser_ui->bypass->get_active(); });

      effects_base->deesser->input_level.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_input_level));
      effects_base->deesser->output_level.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_output_level));
      effects_base->deesser->compression.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_compression));
      effects_base->deesser->detected.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_detected));

      effects_base->deesser->bypass = false;
    } else if (name == plugin_name::delay) {
      auto* const delay_ui = DelayUi::add_to_stack(stack_plugins, path);

      delay_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->delay->bypass = delay_ui->bypass->get_active(); });

      effects_base->delay->input_level.connect(sigc::mem_fun(*delay_ui, &DelayUi::on_new_input_level));
      effects_base->delay->output_level.connect(sigc::mem_fun(*delay_ui, &DelayUi::on_new_output_level));

      effects_base->delay->bypass = false;
    } else if (name == plugin_name::echo_canceller) {
      auto* const echo_canceller_ui = EchoCancellerUi::add_to_stack(stack_plugins, path);

      echo_canceller_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->echo_canceller->bypass = echo_canceller_ui->bypass->get_active(); });

      effects_base->echo_canceller->input_level.connect(
          sigc::mem_fun(*echo_canceller_ui, &EchoCancellerUi::on_new_input_level));
      effects_base->echo_canceller->output_level.connect(
          sigc::mem_fun(*echo_canceller_ui, &EchoCancellerUi::on_new_output_level));

      effects_base->echo_canceller->bypass = false;
    } else if (name == plugin_name::equalizer) {
      auto* const equalizer_ui = EqualizerUi::add_to_stack(stack_plugins, path);

      equalizer_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->equalizer->bypass = equalizer_ui->bypass->get_active(); });

      equalizer_ui->set_transient_window(transient_window);

      effects_base->equalizer->input_level.connect(sigc::mem_fun(*equalizer_ui, &EqualizerUi::on_new_input_level));
      effects_base->equalizer->output_level.connect(sigc::mem_fun(*equalizer_ui, &EqualizerUi::on_new_output_level));

      effects_base->equalizer->bypass = false;
    } else if (name == plugin_name::exciter) {
      auto* const exciter_ui = ExciterUi::add_to_stack(stack_plugins, path);

      exciter_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->exciter->bypass = exciter_ui->bypass->get_active(); });

      effects_base->exciter->input_level.connect(sigc::mem_fun(*exciter_ui, &ExciterUi::on_new_input_level));
      effects_base->exciter->output_level.connect(sigc::mem_fun(*exciter_ui, &ExciterUi::on_new_output_level));
      effects_base->exciter->harmonics.connect(sigc::mem_fun(*exciter_ui, &ExciterUi::on_new_harmonics_level));

      effects_base->exciter->bypass = false;
    } else if (name == plugin_name::filter) {
      auto* const filter_ui = FilterUi::add_to_stack(stack_plugins, path);

      filter_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->filter->bypass = filter_ui->bypass->get_active(); });

      effects_base->filter->input_level.connect(sigc::mem_fun(*filter_ui, &FilterUi::on_new_input_level));
      effects_base->filter->output_level.connect(sigc::mem_fun(*filter_ui, &FilterUi::on_new_output_level));

      effects_base->filter->bypass = false;
    } else if (name == plugin_name::gate) {
      auto* const gate_ui = GateUi::add_to_stack(stack_plugins, path);

      gate_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->gate->bypass = gate_ui->bypass->get_active(); });

      effects_base->gate->input_level.connect(sigc::mem_fun(*gate_ui, &GateUi::on_new_input_level));
      effects_base->gate->output_level.connect(sigc::mem_fun(*gate_ui, &GateUi::on_new_output_level));
      effects_base->gate->gating.connect(sigc::mem_fun(*gate_ui, &GateUi::on_new_gating));

      effects_base->gate->bypass = false;
    } else if (name == plugin_name::limiter) {
      auto* const limiter_ui = LimiterUi::add_to_stack(stack_plugins, path);

      limiter_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->limiter->bypass = limiter_ui->bypass->get_active(); });

      limiter_ui->set_pipe_manager_ptr(pm);

      effects_base->limiter->input_level.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_input_level));
      effects_base->limiter->output_level.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_output_level));
      effects_base->limiter->gain_left.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_left_gain));
      effects_base->limiter->gain_right.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_right_gain));
      effects_base->limiter->sidechain_left.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_left_sidechain));
      effects_base->limiter->sidechain_right.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_right_sidechain));

      effects_base->limiter->bypass = false;
    } else if (name == plugin_name::loudness) {
      auto* const loudness_ui = LoudnessUi::add_to_stack(stack_plugins, path);

      loudness_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->loudness->bypass = loudness_ui->bypass->get_active(); });

      effects_base->loudness->input_level.connect(sigc::mem_fun(*loudness_ui, &LoudnessUi::on_new_input_level));
      effects_base->loudness->output_level.connect(sigc::mem_fun(*loudness_ui, &LoudnessUi::on_new_output_level));

      effects_base->loudness->bypass = false;
    } else if (name == plugin_name::maximizer) {
      auto* const maximizer_ui = MaximizerUi::add_to_stack(stack_plugins, path);

      maximizer_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->maximizer->bypass = maximizer_ui->bypass->get_active(); });

      effects_base->maximizer->input_level.connect(sigc::mem_fun(*maximizer_ui, &MaximizerUi::on_new_input_level));
      effects_base->maximizer->output_level.connect(sigc::mem_fun(*maximizer_ui, &MaximizerUi::on_new_output_level));
      effects_base->maximizer->reduction.connect(sigc::mem_fun(*maximizer_ui, &MaximizerUi::on_new_reduction));

      effects_base->maximizer->bypass = false;
    } else if (name == plugin_name::multiband_compressor) {
      auto* const multiband_compressor_ui = MultibandCompressorUi::add_to_stack(stack_plugins, path);

      multiband_compressor_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->multiband_compressor->bypass = multiband_compressor_ui->bypass->get_active(); });

      multiband_compressor_ui->set_pipe_manager_ptr(pm);

      effects_base->multiband_compressor->input_level.connect(
          sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_input_level));
      effects_base->multiband_compressor->output_level.connect(
          sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_output_level));

      effects_base->multiband_compressor->frequency_range.connect(
          sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_frequency_range));
      effects_base->multiband_compressor->envelope.connect(
          sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_envelope));
      effects_base->multiband_compressor->curve.connect(
          sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_curve));
      effects_base->multiband_compressor->reduction.connect(
          sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_reduction));

      effects_base->multiband_compressor->bypass = false;
    } else if (name == plugin_name::multiband_gate) {
      auto* const multiband_gate_ui = MultibandGateUi::add_to_stack(stack_plugins, path);

      multiband_gate_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->multiband_gate->bypass = multiband_gate_ui->bypass->get_active(); });

      effects_base->multiband_gate->input_level.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_input_level));
      effects_base->multiband_gate->output_level.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output_level));

      effects_base->multiband_gate->output0.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output0));
      effects_base->multiband_gate->output1.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output1));
      effects_base->multiband_gate->output2.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output2));
      effects_base->multiband_gate->output3.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output3));

      effects_base->multiband_gate->gating0.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating0));
      effects_base->multiband_gate->gating1.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating1));
      effects_base->multiband_gate->gating2.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating2));
      effects_base->multiband_gate->gating3.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating3));

      effects_base->multiband_gate->bypass = false;
    } else if (name == plugin_name::pitch) {
      auto* const pitch_ui = PitchUi::add_to_stack(stack_plugins, path);

      pitch_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->pitch->bypass = pitch_ui->bypass->get_active(); });

      effects_base->pitch->input_level.connect(sigc::mem_fun(*pitch_ui, &PitchUi::on_new_input_level));
      effects_base->pitch->output_level.connect(sigc::mem_fun(*pitch_ui, &PitchUi::on_new_output_level));

      effects_base->pitch->bypass = false;
    } else if (name == plugin_name::reverb) {
      auto* const reverb_ui = ReverbUi::add_to_stack(stack_plugins, path);

      reverb_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->reverb->bypass = reverb_ui->bypass->get_active(); });

      effects_base->reverb->input_level.connect(sigc::mem_fun(*reverb_ui, &ReverbUi::on_new_input_level));
      effects_base->reverb->output_level.connect(sigc::mem_fun(*reverb_ui, &ReverbUi::on_new_output_level));

      effects_base->reverb->bypass = false;
    } else if (name == plugin_name::rnnoise) {
      auto* const rnnoise_ui = RNNoiseUi::add_to_stack(stack_plugins, path);

      rnnoise_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->rnnoise->bypass = rnnoise_ui->bypass->get_active(); });

      rnnoise_ui->set_transient_window(transient_window);

      effects_base->rnnoise->input_level.connect(sigc::mem_fun(*rnnoise_ui, &RNNoiseUi::on_new_input_level));
      effects_base->rnnoise->output_level.connect(sigc::mem_fun(*rnnoise_ui, &RNNoiseUi::on_new_output_level));

      effects_base->rnnoise->bypass = false;
    } else if (name == plugin_name::stereo_tools) {
      auto* const stereo_tools_ui = StereoToolsUi::add_to_stack(stack_plugins, path);

      stereo_tools_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->stereo_tools->bypass = stereo_tools_ui->bypass->get_active(); });

      effects_base->stereo_tools->input_level.connect(
          sigc::mem_fun(*stereo_tools_ui, &StereoToolsUi::on_new_input_level));
      effects_base->stereo_tools->output_level.connect(
          sigc::mem_fun(*stereo_tools_ui, &StereoToolsUi::on_new_output_level));

      effects_base->stereo_tools->bypass = false;
    }
  }
}

void EffectsBaseUi::setup_listview_selected_plugins() {
  // setting the listview model and factory

  listview_selected_plugins->set_model(Gtk::SingleSelection::create(selected_plugins));

  auto factory = Gtk::SignalListItemFactory::create();

  listview_selected_plugins->set_factory(factory);

  // setting the item selection callback

  listview_selected_plugins->get_model()->signal_selection_changed().connect([&](guint position, guint n_items) {
    auto single = std::dynamic_pointer_cast<Gtk::SingleSelection>(listview_selected_plugins->get_model());

    const auto selected_name = single->get_selected_item()->get_property<Glib::ustring>("string");

    for (auto* child = stack_plugins->get_first_child(); child != nullptr; child = child->get_next_sibling()) {
      if (stack_plugins->get_page(*child)->get_name() == selected_name) {
        stack_plugins->set_visible_child(*child);

        return;
      }
    }
  });

  // setting the factory callbacks

  factory->signal_setup().connect([=, this](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* box = Gtk::make_managed<Gtk::Box>();
    auto* const label = Gtk::make_managed<Gtk::Label>();
    auto* const remove = Gtk::make_managed<Gtk::Button>();
    auto* const drag_handle = Gtk::make_managed<Gtk::Image>();
    auto* const plugin_icon = Gtk::make_managed<Gtk::Image>();

    label->set_hexpand(true);
    label->set_halign(Gtk::Align::START);

    remove->set_icon_name("edit-delete-symbolic");
    remove->set_css_classes({"flat"});

    drag_handle->set_from_icon_name("view-app-grid-symbolic");

    // it is Adwaita folder-download-symbolic icon renamed
    plugin_icon->set_from_icon_name("ee-arrow-down-symbolic");

    plugin_icon->set_margin_start(6);
    plugin_icon->set_margin_end(6);

    box->set_spacing(6);
    box->append(*plugin_icon);
    box->append(*label);
    box->append(*remove);
    box->append(*drag_handle);

    remove->set_opacity(0.0);
    drag_handle->set_opacity(0.0);

    auto controller = Gtk::EventControllerMotion::create();

    controller->signal_enter().connect([=](const double& x, const double& y) {
      remove->set_opacity(1.0);
      drag_handle->set_opacity(1.0);
    });

    controller->signal_leave().connect([=]() {
      remove->set_opacity(0.0);
      drag_handle->set_opacity(0.0);
    });

    box->add_controller(controller);

    // drag and drop

    auto drag_source = Gtk::DragSource::create();
    auto drop_target = Gtk::DropTarget::create(Glib::Value<Glib::ustring>::value_type(), Gdk::DragAction::MOVE);

    drag_source->set_actions(Gdk::DragAction::MOVE);

    drag_source->signal_prepare().connect(
        [=](const double& x, const double& y) {
          const auto paintable = Gtk::WidgetPaintable::create(*box);

          drag_source->set_icon(paintable, 0, 0);

          return Gdk::ContentProvider::create(util::glib_value(label->get_name()));
        },
        false);

    drop_target->signal_drop().connect(
        [=, this](const Glib::ValueBase& v, const double& x, const double& y) {
          Glib::Value<Glib::ustring> name_value;

          name_value.init(v.gobj());

          const auto src = name_value.get();
          const auto dst = label->get_name();

          if (src != dst) {
            auto list = settings->get_string_array("plugins");

            const auto iter_src = std::ranges::find(list, src);
            auto iter_dst = std::ranges::find(list, dst);

            const auto insert_after = (iter_src - list.begin() < iter_dst - list.begin()) ? true : false;

            list.erase(iter_src);

            iter_dst = std::ranges::find(list, dst);

            list.insert(((insert_after) ? (iter_dst + 1) : iter_dst), src);

            Glib::signal_idle().connect_once([=, this] { settings->set_string_array("plugins", list); });

            return true;
          }

          return false;
        },
        false);

    drag_handle->add_controller(drag_source);
    box->add_controller(drop_target);

    // setting list_item data

    list_item->set_data("name", label);
    list_item->set_data("remove", remove);
    list_item->set_data("plugin_icon", plugin_icon);

    list_item->set_child(*box);
  });

  factory->signal_bind().connect([=, this](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const label = static_cast<Gtk::Label*>(list_item->get_data("name"));
    auto* const remove = static_cast<Gtk::Button*>(list_item->get_data("remove"));
    auto* const plugin_icon = static_cast<Gtk::Image*>(list_item->get_data("plugin_icon"));

    const auto name = list_item->get_item()->get_property<Glib::ustring>("string");
    // const auto translated_name = plugins_names[name];

    label->set_name(name);
    // label->set_text(translated_name);

    const auto selected_plugins_list = settings->get_string_array("plugins");

    if (const auto iter_name = std::ranges::find(selected_plugins_list, name);
        (iter_name == selected_plugins_list.begin() && iter_name != selected_plugins_list.end() - 2) ||
        iter_name == selected_plugins_list.end() - 1) {
      // it is Adwaita media-playback-stop-symbolic icon renamed
      plugin_icon->set_from_icon_name("ee-square-symbolic");
    } else {
      plugin_icon->set_from_icon_name("ee-arrow-down-symbolic");
    }

    // remove->update_property(Gtk::Accessible::Property::LABEL,
    //                         util::glib_value(Glib::ustring(_("Remove")) + " " + translated_name));

    auto connection_remove = remove->signal_clicked().connect([=, this]() {
      auto list = settings->get_string_array("plugins");

      list.erase(std::remove_if(list.begin(), list.end(), [=](const auto& plugin_name) { return plugin_name == name; }),
                 list.end());

      settings->set_string_array("plugins", list);
    });

    // setting list_item data

    list_item->set_data("connection_remove", new sigc::connection(connection_remove),
                        Glib::destroy_notify_delete<sigc::connection>);
  });

  factory->signal_unbind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    for (const auto* conn : {"connection_remove"}) {
      if (auto* connection = static_cast<sigc::connection*>(list_item->get_data(conn))) {
        connection->disconnect();

        list_item->set_data(conn, nullptr);
      }
    }
  });
}

void EffectsBaseUi::set_transient_window(Gtk::Window* transient_window) {
  this->transient_window = transient_window;

  for (auto* child = stack_plugins->get_first_child(); child != nullptr; child = child->get_next_sibling()) {
    const auto page = stack_plugins->get_page(*child);

    if (page->get_name() == plugin_name::equalizer) {
      dynamic_cast<EqualizerUi*>(child)->set_transient_window(transient_window);
    } else if (page->get_name() == plugin_name::rnnoise) {
      dynamic_cast<RNNoiseUi*>(child)->set_transient_window(transient_window);
    } else if (page->get_name() == plugin_name::convolver) {
      dynamic_cast<ConvolverUi*>(child)->set_transient_window(transient_window);
    }
  }
}
