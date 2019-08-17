#include "sink_input_effects_ui.hpp"

SinkInputEffectsUi::SinkInputEffectsUi(BaseObjectType* cobject,
                                       const Glib::RefPtr<Gtk::Builder>& refBuilder,
                                       const Glib::RefPtr<Gio::Settings>& refSettings,
                                       SinkInputEffects* sie_ptr)
    : Gtk::Box(cobject), EffectsBaseUi(refBuilder, refSettings, sie_ptr->pm), sie(sie_ptr) {
  // populate stack

  auto b_limiter = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/limiter.glade");
  auto b_compressor = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/compressor.glade");
  auto b_filter = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/filter.glade");
  auto b_equalizer = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/equalizer.glade");
  auto b_pitch = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/pitch.glade");
  auto b_reverb = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/reverb.glade");
  auto b_bass_enhancer = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/bass_enhancer.glade");
  auto b_exciter = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/exciter.glade");
  auto b_crossfeed = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/crossfeed.glade");
  auto b_maximizer = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/maximizer.glade");
  auto b_multiband_compressor =
      Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/multiband_compressor.glade");
  auto b_loudness = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/loudness.glade");
  auto b_gate = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/gate.glade");
  auto b_multiband_gate = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/multiband_gate.glade");
  auto b_deesser = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/deesser.glade");
  auto b_stereo_tools = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/stereo_tools.glade");
  auto b_convolver = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/convolver.glade");
  auto b_crystalizer = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/crystalizer.glade");
  auto b_autogain = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/autogain.glade");
  auto b_delay = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/delay.glade");

  b_limiter->get_widget_derived("widgets_grid", limiter_ui, "com.github.wwmm.pulseeffects.sinkinputs.limiter");
  b_compressor->get_widget_derived("widgets_grid", compressor_ui, "com.github.wwmm.pulseeffects.sinkinputs.compressor");
  b_filter->get_widget_derived("widgets_grid", filter_ui, "com.github.wwmm.pulseeffects.sinkinputs.filter");
  b_equalizer->get_widget_derived("widgets_grid", equalizer_ui, "com.github.wwmm.pulseeffects.sinkinputs.equalizer",
                                  "com.github.wwmm.pulseeffects.sinkinputs.equalizer.leftchannel",
                                  "com.github.wwmm.pulseeffects.sinkinputs.equalizer.rightchannel");
  b_pitch->get_widget_derived("widgets_grid", pitch_ui, "com.github.wwmm.pulseeffects.sinkinputs.pitch");
  b_reverb->get_widget_derived("widgets_grid", reverb_ui, "com.github.wwmm.pulseeffects.sinkinputs.reverb");
  b_bass_enhancer->get_widget_derived("widgets_grid", bass_enhancer_ui,
                                      "com.github.wwmm.pulseeffects.sinkinputs.bassenhancer");
  b_exciter->get_widget_derived("widgets_grid", exciter_ui, "com.github.wwmm.pulseeffects.sinkinputs.exciter");
  b_crossfeed->get_widget_derived("widgets_grid", crossfeed_ui, "com.github.wwmm.pulseeffects.sinkinputs.crossfeed");
  b_maximizer->get_widget_derived("widgets_grid", maximizer_ui, "com.github.wwmm.pulseeffects.sinkinputs.maximizer");
  b_multiband_compressor->get_widget_derived("widgets_grid", multiband_compressor_ui,
                                             "com.github.wwmm.pulseeffects.sinkinputs.multibandcompressor");
  b_loudness->get_widget_derived("widgets_grid", loudness_ui, "com.github.wwmm.pulseeffects.sinkinputs.loudness");
  b_gate->get_widget_derived("widgets_grid", gate_ui, "com.github.wwmm.pulseeffects.sinkinputs.gate");
  b_multiband_gate->get_widget_derived("widgets_grid", multiband_gate_ui,
                                       "com.github.wwmm.pulseeffects.sinkinputs.multibandgate");
  b_deesser->get_widget_derived("widgets_grid", deesser_ui, "com.github.wwmm.pulseeffects.sinkinputs.deesser");
  b_stereo_tools->get_widget_derived("widgets_grid", stereo_tools_ui,
                                     "com.github.wwmm.pulseeffects.sinkinputs.stereotools");
  b_convolver->get_widget_derived("widgets_grid", convolver_ui, "com.github.wwmm.pulseeffects.sinkinputs.convolver");
  b_crystalizer->get_widget_derived("widgets_grid", crystalizer_ui,
                                    "com.github.wwmm.pulseeffects.sinkinputs.crystalizer");
  b_autogain->get_widget_derived("widgets_grid", autogain_ui, "com.github.wwmm.pulseeffects.sinkinputs.autogain");
  b_delay->get_widget_derived("widgets_grid", delay_ui, "com.github.wwmm.pulseeffects.sinkinputs.delay");

  stack->add(*limiter_ui, limiter_ui->name);
  stack->add(*compressor_ui, compressor_ui->name);
  stack->add(*filter_ui, filter_ui->name);
  stack->add(*equalizer_ui, equalizer_ui->name);
  stack->add(*pitch_ui, pitch_ui->name);
  stack->add(*reverb_ui, reverb_ui->name);
  stack->add(*bass_enhancer_ui, bass_enhancer_ui->name);
  stack->add(*exciter_ui, exciter_ui->name);
  stack->add(*crossfeed_ui, crossfeed_ui->name);
  stack->add(*maximizer_ui, maximizer_ui->name);
  stack->add(*multiband_compressor_ui, multiband_compressor_ui->name);
  stack->add(*loudness_ui, loudness_ui->name);
  stack->add(*gate_ui, gate_ui->name);
  stack->add(*multiband_gate_ui, multiband_gate_ui->name);
  stack->add(*deesser_ui, deesser_ui->name);
  stack->add(*stereo_tools_ui, stereo_tools_ui->name);
  stack->add(*convolver_ui, convolver_ui->name);
  stack->add(*crystalizer_ui, crystalizer_ui->name);
  stack->add(*autogain_ui, autogain_ui->name);
  stack->add(*delay_ui, delay_ui->name);

  // populate_listbox

  add_to_listbox(limiter_ui);
  add_to_listbox(compressor_ui);
  add_to_listbox(filter_ui);
  add_to_listbox(equalizer_ui);
  add_to_listbox(pitch_ui);
  add_to_listbox(reverb_ui);
  add_to_listbox(bass_enhancer_ui);
  add_to_listbox(exciter_ui);
  add_to_listbox(crossfeed_ui);
  add_to_listbox(maximizer_ui);
  add_to_listbox(multiband_compressor_ui);
  add_to_listbox(loudness_ui);
  add_to_listbox(gate_ui);
  add_to_listbox(multiband_gate_ui);
  add_to_listbox(deesser_ui);
  add_to_listbox(stereo_tools_ui);
  add_to_listbox(convolver_ui);
  add_to_listbox(crystalizer_ui);
  add_to_listbox(autogain_ui);
  add_to_listbox(delay_ui);

  level_meters_connections();
  up_down_connections();

  connections.push_back(sie->new_spectrum.connect(sigc::mem_fun(*spectrum_ui, &SpectrumUi::on_new_spectrum)));
  connections.push_back(sie->pm->sink_input_added.connect(sigc::mem_fun(this, &SinkInputEffectsUi::on_app_added)));
  connections.push_back(sie->pm->sink_input_changed.connect(sigc::mem_fun(this, &SinkInputEffectsUi::on_app_changed)));
  connections.push_back(sie->pm->sink_input_removed.connect(sigc::mem_fun(this, &SinkInputEffectsUi::on_app_removed)));
}

SinkInputEffectsUi::~SinkInputEffectsUi() {
  sie->disable_spectrum();

  util::debug(log_tag + "destroyed");
}

SinkInputEffectsUi* SinkInputEffectsUi::add_to_stack(Gtk::Stack* stack, SinkInputEffects* sie_ptr) {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/effects_base.glade");

  auto settings = Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs");

  SinkInputEffectsUi* ui;

  builder->get_widget_derived("widgets_box", ui, settings, sie_ptr);

  stack->add(*ui, "sink_inputs");
  stack->child_property_icon_name(*ui).set_value("audio-speakers-symbolic");

  return ui;
}

void SinkInputEffectsUi::level_meters_connections() {
  // limiter level meters connections

  connections.push_back(sie->limiter->input_level.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_input_level)));
  connections.push_back(
      sie->limiter->output_level.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_output_level)));
  connections.push_back(sie->limiter->attenuation.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_attenuation)));

  // compressor level meters connections

  connections.push_back(
      sie->compressor->input_level.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_input_level)));
  connections.push_back(
      sie->compressor->output_level.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_output_level)));
  connections.push_back(
      sie->compressor->reduction.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_reduction)));
  connections.push_back(
      sie->compressor->sidechain.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_sidechain)));
  connections.push_back(sie->compressor->curve.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_curve)));

  // filter level meters connections

  connections.push_back(sie->filter->input_level.connect(sigc::mem_fun(*filter_ui, &FilterUi::on_new_input_level)));
  connections.push_back(sie->filter->output_level.connect(sigc::mem_fun(*filter_ui, &FilterUi::on_new_output_level)));

  // equalizer level meters connections

  connections.push_back(
      sie->equalizer_input_level.connect(sigc::mem_fun(*equalizer_ui, &EqualizerUi::on_new_input_level_db)));
  connections.push_back(
      sie->equalizer_output_level.connect(sigc::mem_fun(*equalizer_ui, &EqualizerUi::on_new_output_level_db)));

  // pitch level meters connections

  connections.push_back(sie->pitch_input_level.connect(sigc::mem_fun(*pitch_ui, &PitchUi::on_new_input_level_db)));
  connections.push_back(sie->pitch_output_level.connect(sigc::mem_fun(*pitch_ui, &PitchUi::on_new_output_level_db)));

  // reverb level meters connections

  connections.push_back(sie->reverb->input_level.connect(sigc::mem_fun(*reverb_ui, &ReverbUi::on_new_input_level)));
  connections.push_back(sie->reverb->output_level.connect(sigc::mem_fun(*reverb_ui, &ReverbUi::on_new_output_level)));

  // bass_enhancer level meters connections

  connections.push_back(
      sie->bass_enhancer_input_level.connect(sigc::mem_fun(*bass_enhancer_ui, &BassEnhancerUi::on_new_input_level_db)));
  connections.push_back(sie->bass_enhancer_output_level.connect(
      sigc::mem_fun(*bass_enhancer_ui, &BassEnhancerUi::on_new_output_level_db)));
  connections.push_back(
      sie->bass_enhancer->harmonics.connect(sigc::mem_fun(*bass_enhancer_ui, &BassEnhancerUi::on_new_harmonics_level)));

  // exciter level meters connections

  connections.push_back(
      sie->exciter_input_level.connect(sigc::mem_fun(*exciter_ui, &ExciterUi::on_new_input_level_db)));
  connections.push_back(
      sie->exciter_output_level.connect(sigc::mem_fun(*exciter_ui, &ExciterUi::on_new_output_level_db)));
  connections.push_back(
      sie->exciter->harmonics.connect(sigc::mem_fun(*exciter_ui, &ExciterUi::on_new_harmonics_level)));

  // crossfeed level meters connections

  connections.push_back(
      sie->crossfeed_input_level.connect(sigc::mem_fun(*crossfeed_ui, &CrossfeedUi::on_new_input_level_db)));
  connections.push_back(
      sie->crossfeed_output_level.connect(sigc::mem_fun(*crossfeed_ui, &CrossfeedUi::on_new_output_level_db)));

  // maximizer level meters connections

  connections.push_back(
      sie->maximizer_input_level.connect(sigc::mem_fun(*maximizer_ui, &MaximizerUi::on_new_input_level_db)));
  connections.push_back(
      sie->maximizer_output_level.connect(sigc::mem_fun(*maximizer_ui, &MaximizerUi::on_new_output_level_db)));
  connections.push_back(
      sie->maximizer->reduction.connect(sigc::mem_fun(*maximizer_ui, &MaximizerUi::on_new_reduction)));

  // multiband_compressor level meters connections

  connections.push_back(sie->multiband_compressor->input_level.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_input_level)));
  connections.push_back(sie->multiband_compressor->output_level.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_output_level)));

  connections.push_back(sie->multiband_compressor->output0.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_output0)));
  connections.push_back(sie->multiband_compressor->output1.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_output1)));
  connections.push_back(sie->multiband_compressor->output2.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_output2)));
  connections.push_back(sie->multiband_compressor->output3.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_output3)));

  connections.push_back(sie->multiband_compressor->compression0.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_compression0)));
  connections.push_back(sie->multiband_compressor->compression1.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_compression1)));
  connections.push_back(sie->multiband_compressor->compression2.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_compression2)));
  connections.push_back(sie->multiband_compressor->compression3.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_compression3)));

  // loudness level meters connections

  connections.push_back(
      sie->loudness_input_level.connect(sigc::mem_fun(*loudness_ui, &LoudnessUi::on_new_input_level_db)));
  connections.push_back(
      sie->loudness_output_level.connect(sigc::mem_fun(*loudness_ui, &LoudnessUi::on_new_output_level_db)));

  // gate level meters connections

  connections.push_back(sie->gate_input_level.connect(sigc::mem_fun(*gate_ui, &GateUi::on_new_input_level_db)));
  connections.push_back(sie->gate_output_level.connect(sigc::mem_fun(*gate_ui, &GateUi::on_new_output_level_db)));
  connections.push_back(sie->gate->gating.connect(sigc::mem_fun(*gate_ui, &GateUi::on_new_gating)));

  // multiband_gate level meters connections

  connections.push_back(sie->multiband_gate->input_level.connect(
      sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_input_level)));
  connections.push_back(sie->multiband_gate->output_level.connect(
      sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output_level)));

  connections.push_back(
      sie->multiband_gate->output0.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output0)));
  connections.push_back(
      sie->multiband_gate->output1.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output1)));
  connections.push_back(
      sie->multiband_gate->output2.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output2)));
  connections.push_back(
      sie->multiband_gate->output3.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output3)));

  connections.push_back(
      sie->multiband_gate->gating0.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating0)));
  connections.push_back(
      sie->multiband_gate->gating1.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating1)));
  connections.push_back(
      sie->multiband_gate->gating2.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating2)));
  connections.push_back(
      sie->multiband_gate->gating3.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating3)));

  // deesser level meters connections

  connections.push_back(
      sie->deesser_input_level.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_input_level_db)));
  connections.push_back(
      sie->deesser_output_level.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_output_level_db)));
  connections.push_back(sie->deesser->compression.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_compression)));
  connections.push_back(sie->deesser->detected.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_detected)));

  // stereo_tools level meters connections

  connections.push_back(
      sie->stereo_tools->input_level.connect(sigc::mem_fun(*stereo_tools_ui, &StereoToolsUi::on_new_input_level)));
  connections.push_back(
      sie->stereo_tools->output_level.connect(sigc::mem_fun(*stereo_tools_ui, &StereoToolsUi::on_new_output_level)));

  // convolver level meters connections

  connections.push_back(
      sie->convolver_input_level.connect(sigc::mem_fun(*convolver_ui, &ConvolverUi::on_new_input_level_db)));
  connections.push_back(
      sie->convolver_output_level.connect(sigc::mem_fun(*convolver_ui, &ConvolverUi::on_new_output_level_db)));

  // crystalizer level meters connections

  connections.push_back(
      sie->crystalizer_input_level.connect(sigc::mem_fun(*crystalizer_ui, &CrystalizerUi::on_new_input_level_db)));
  connections.push_back(
      sie->crystalizer_output_level.connect(sigc::mem_fun(*crystalizer_ui, &CrystalizerUi::on_new_output_level_db)));
  connections.push_back(
      sie->crystalizer->range_before.connect(sigc::mem_fun(*crystalizer_ui, &CrystalizerUi::on_new_range_before)));
  connections.push_back(
      sie->crystalizer->range_after.connect(sigc::mem_fun(*crystalizer_ui, &CrystalizerUi::on_new_range_after)));

  // autogain level meters connections

  connections.push_back(
      sie->autogain_input_level.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_input_level_db)));
  connections.push_back(
      sie->autogain_output_level.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_output_level_db)));
  connections.push_back(sie->autogain->momentary.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_momentary)));
  connections.push_back(sie->autogain->shortterm.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_shortterm)));
  connections.push_back(sie->autogain->integrated.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_integrated)));
  connections.push_back(sie->autogain->relative.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_relative)));
  connections.push_back(sie->autogain->loudness.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_loudness)));
  connections.push_back(sie->autogain->range.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_range)));
  connections.push_back(sie->autogain->gain.connect(sigc::mem_fun(*autogain_ui, &AutoGainUi::on_new_gain)));

  // delay level meters connections

  connections.push_back(sie->delay_input_level.connect(sigc::mem_fun(*delay_ui, &DelayUi::on_new_input_level_db)));
  connections.push_back(sie->delay_output_level.connect(sigc::mem_fun(*delay_ui, &DelayUi::on_new_output_level_db)));
}

void SinkInputEffectsUi::up_down_connections() {
  auto on_up = [=](auto p) {
    auto order = Glib::Variant<std::vector<std::string>>();

    settings->get_value("plugins", order);

    auto vorder = order.get();

    auto r = std::find(std::begin(vorder), std::end(vorder), p->name);

    if (r != std::begin(vorder)) {
      std::iter_swap(r, r - 1);

      settings->set_string_array("plugins", vorder);
    }
  };

  auto on_down = [=](auto p) {
    auto order = Glib::Variant<std::vector<std::string>>();

    settings->get_value("plugins", order);

    auto vorder = order.get();

    auto r = std::find(std::begin(vorder), std::end(vorder), p->name);

    if (r != std::end(vorder) - 1) {
      std::iter_swap(r, r + 1);

      settings->set_string_array("plugins", vorder);
    }
  };

  connections.push_back(limiter_ui->plugin_up->signal_clicked().connect([=]() { on_up(limiter_ui); }));
  connections.push_back(limiter_ui->plugin_down->signal_clicked().connect([=]() { on_down(limiter_ui); }));

  connections.push_back(compressor_ui->plugin_up->signal_clicked().connect([=]() { on_up(compressor_ui); }));
  connections.push_back(compressor_ui->plugin_down->signal_clicked().connect([=]() { on_down(compressor_ui); }));

  connections.push_back(filter_ui->plugin_up->signal_clicked().connect([=]() { on_up(filter_ui); }));
  connections.push_back(filter_ui->plugin_down->signal_clicked().connect([=]() { on_down(filter_ui); }));

  connections.push_back(pitch_ui->plugin_up->signal_clicked().connect([=]() { on_up(pitch_ui); }));
  connections.push_back(pitch_ui->plugin_down->signal_clicked().connect([=]() { on_down(pitch_ui); }));

  connections.push_back(equalizer_ui->plugin_up->signal_clicked().connect([=]() { on_up(equalizer_ui); }));
  connections.push_back(equalizer_ui->plugin_down->signal_clicked().connect([=]() { on_down(equalizer_ui); }));

  connections.push_back(reverb_ui->plugin_up->signal_clicked().connect([=]() { on_up(reverb_ui); }));
  connections.push_back(reverb_ui->plugin_down->signal_clicked().connect([=]() { on_down(reverb_ui); }));

  connections.push_back(bass_enhancer_ui->plugin_up->signal_clicked().connect([=]() { on_up(bass_enhancer_ui); }));
  connections.push_back(bass_enhancer_ui->plugin_down->signal_clicked().connect([=]() { on_down(bass_enhancer_ui); }));

  connections.push_back(exciter_ui->plugin_up->signal_clicked().connect([=]() { on_up(exciter_ui); }));
  connections.push_back(exciter_ui->plugin_down->signal_clicked().connect([=]() { on_down(exciter_ui); }));

  connections.push_back(crossfeed_ui->plugin_up->signal_clicked().connect([=]() { on_up(crossfeed_ui); }));
  connections.push_back(crossfeed_ui->plugin_down->signal_clicked().connect([=]() { on_down(crossfeed_ui); }));

  connections.push_back(maximizer_ui->plugin_up->signal_clicked().connect([=]() { on_up(maximizer_ui); }));
  connections.push_back(maximizer_ui->plugin_down->signal_clicked().connect([=]() { on_down(maximizer_ui); }));

  connections.push_back(
      multiband_compressor_ui->plugin_up->signal_clicked().connect([=]() { on_up(multiband_compressor_ui); }));
  connections.push_back(
      multiband_compressor_ui->plugin_down->signal_clicked().connect([=]() { on_down(multiband_compressor_ui); }));

  connections.push_back(loudness_ui->plugin_up->signal_clicked().connect([=]() { on_up(loudness_ui); }));
  connections.push_back(loudness_ui->plugin_down->signal_clicked().connect([=]() { on_down(loudness_ui); }));

  connections.push_back(gate_ui->plugin_up->signal_clicked().connect([=]() { on_up(gate_ui); }));
  connections.push_back(gate_ui->plugin_down->signal_clicked().connect([=]() { on_down(gate_ui); }));

  connections.push_back(multiband_gate_ui->plugin_up->signal_clicked().connect([=]() { on_up(multiband_gate_ui); }));
  connections.push_back(
      multiband_gate_ui->plugin_down->signal_clicked().connect([=]() { on_down(multiband_gate_ui); }));

  connections.push_back(deesser_ui->plugin_up->signal_clicked().connect([=]() { on_up(deesser_ui); }));
  connections.push_back(deesser_ui->plugin_down->signal_clicked().connect([=]() { on_down(deesser_ui); }));

  connections.push_back(stereo_tools_ui->plugin_up->signal_clicked().connect([=]() { on_up(stereo_tools_ui); }));
  connections.push_back(stereo_tools_ui->plugin_down->signal_clicked().connect([=]() { on_down(stereo_tools_ui); }));

  connections.push_back(convolver_ui->plugin_up->signal_clicked().connect([=]() { on_up(convolver_ui); }));
  connections.push_back(convolver_ui->plugin_down->signal_clicked().connect([=]() { on_down(convolver_ui); }));

  connections.push_back(crystalizer_ui->plugin_up->signal_clicked().connect([=]() { on_up(crystalizer_ui); }));
  connections.push_back(crystalizer_ui->plugin_down->signal_clicked().connect([=]() { on_down(crystalizer_ui); }));

  connections.push_back(autogain_ui->plugin_up->signal_clicked().connect([=]() { on_up(autogain_ui); }));
  connections.push_back(autogain_ui->plugin_down->signal_clicked().connect([=]() { on_down(autogain_ui); }));

  connections.push_back(delay_ui->plugin_up->signal_clicked().connect([=]() { on_up(delay_ui); }));
  connections.push_back(delay_ui->plugin_down->signal_clicked().connect([=]() { on_down(delay_ui); }));
}
