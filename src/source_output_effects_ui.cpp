#include "source_output_effects_ui.hpp"

SourceOutputEffectsUi::SourceOutputEffectsUi(BaseObjectType* cobject,
                                             const Glib::RefPtr<Gtk::Builder>& refBuilder,
                                             const Glib::RefPtr<Gio::Settings>& refSettings,
                                             SourceOutputEffects* soe_ptr)
    : Gtk::Box(cobject), EffectsBaseUi(refBuilder, refSettings, soe_ptr->pm), soe(soe_ptr) {
  // populate stack

  auto b_limiter = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/limiter.glade");
  auto b_compressor = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/compressor.glade");
  auto b_filter = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/filter.glade");
  auto b_equalizer = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/equalizer.glade");
  auto b_reverb = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/reverb.glade");
  auto b_gate = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/gate.glade");
  auto b_deesser = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/deesser.glade");
  auto b_pitch = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/pitch.glade");
  auto b_webrtc = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/webrtc.glade");
  auto b_multiband_compressor =
      Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/multiband_compressor.glade");
  auto b_multiband_gate = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/multiband_gate.glade");
  auto b_stereo_tools = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/stereo_tools.glade");

  b_limiter->get_widget_derived("widgets_grid", limiter_ui, "com.github.wwmm.pulseeffects.limiter",
                                "/com/github/wwmm/pulseeffects/sourceoutputs/limiter/");

  b_compressor->get_widget_derived("widgets_grid", compressor_ui, "com.github.wwmm.pulseeffects.compressor",
                                   "/com/github/wwmm/pulseeffects/sourceoutputs/compressor/");

  b_filter->get_widget_derived("widgets_grid", filter_ui, "com.github.wwmm.pulseeffects.filter",
                               "/com/github/wwmm/pulseeffects/sourceoutputs/filter/");

  b_equalizer->get_widget_derived("widgets_grid", equalizer_ui, "com.github.wwmm.pulseeffects.equalizer",
                                  "/com/github/wwmm/pulseeffects/sourceoutputs/equalizer/",
                                  "com.github.wwmm.pulseeffects.equalizer.channel",
                                  "/com/github/wwmm/pulseeffects/sourceoutputs/equalizer/leftchannel/",
                                  "/com/github/wwmm/pulseeffects/sourceoutputs/equalizer/rightchannel/");

  b_reverb->get_widget_derived("widgets_grid", reverb_ui, "com.github.wwmm.pulseeffects.reverb",
                               "/com/github/wwmm/pulseeffects/sourceoutputs/reverb/");

  b_gate->get_widget_derived("widgets_grid", gate_ui, "com.github.wwmm.pulseeffects.gate",
                             "/com/github/wwmm/pulseeffects/sourceoutputs/gate/");

  b_deesser->get_widget_derived("widgets_grid", deesser_ui, "com.github.wwmm.pulseeffects.deesser",
                                "/com/github/wwmm/pulseeffects/sourceoutputs/deesser/");

  b_pitch->get_widget_derived("widgets_grid", pitch_ui, "com.github.wwmm.pulseeffects.pitch",
                              "/com/github/wwmm/pulseeffects/sourceoutputs/pitch/");

  b_webrtc->get_widget_derived("widgets_grid", webrtc_ui, "com.github.wwmm.pulseeffects.webrtc",
                               "/com/github/wwmm/pulseeffects/sourceoutputs/webrtc/");

  b_multiband_compressor->get_widget_derived("widgets_grid", multiband_compressor_ui,
                                             "com.github.wwmm.pulseeffects.multibandcompressor",
                                             "/com/github/wwmm/pulseeffects/sourceoutputs/multibandcompressor/");

  b_multiband_gate->get_widget_derived("widgets_grid", multiband_gate_ui, "com.github.wwmm.pulseeffects.multibandgate",
                                       "/com/github/wwmm/pulseeffects/sourceoutputs/multibandgate/");

  b_stereo_tools->get_widget_derived("widgets_grid", stereo_tools_ui, "com.github.wwmm.pulseeffects.stereotools",
                                     "/com/github/wwmm/pulseeffects/sourceoutputs/stereotools/");

  // set preset type property inside user interfaces to be intepreted as "input"

  limiter_ui->preset_type = PresetType::input;
  compressor_ui->preset_type = PresetType::input;
  filter_ui->preset_type = PresetType::input;
  equalizer_ui->preset_type = PresetType::input;
  reverb_ui->preset_type = PresetType::input;
  gate_ui->preset_type = PresetType::input;
  deesser_ui->preset_type = PresetType::input;
  pitch_ui->preset_type = PresetType::input;
  webrtc_ui->preset_type = PresetType::input;
  multiband_compressor_ui->preset_type = PresetType::input;
  multiband_gate_ui->preset_type = PresetType::input;

  // add to stack

  stack->add(*limiter_ui, limiter_ui->name);
  stack->add(*compressor_ui, compressor_ui->name);
  stack->add(*filter_ui, filter_ui->name);
  stack->add(*equalizer_ui, equalizer_ui->name);
  stack->add(*reverb_ui, reverb_ui->name);
  stack->add(*gate_ui, gate_ui->name);
  stack->add(*deesser_ui, deesser_ui->name);
  stack->add(*pitch_ui, pitch_ui->name);
  stack->add(*webrtc_ui, webrtc_ui->name);
  stack->add(*multiband_compressor_ui, multiband_compressor_ui->name);
  stack->add(*multiband_gate_ui, multiband_gate_ui->name);
  stack->add(*stereo_tools_ui, stereo_tools_ui->name);

  // populate listbox

  add_to_listbox(limiter_ui);
  add_to_listbox(compressor_ui);
  add_to_listbox(filter_ui);
  add_to_listbox(equalizer_ui);
  add_to_listbox(reverb_ui);
  add_to_listbox(gate_ui);
  add_to_listbox(deesser_ui);
  add_to_listbox(pitch_ui);
  add_to_listbox(webrtc_ui);
  add_to_listbox(multiband_compressor_ui);
  add_to_listbox(multiband_gate_ui);
  add_to_listbox(stereo_tools_ui);

  level_meters_connections();
  up_down_connections();

  connections.emplace_back(soe->new_spectrum.connect(sigc::mem_fun(*spectrum_ui, &SpectrumUi::on_new_spectrum)));
  connections.emplace_back(
      soe->pm->source_output_added.connect(sigc::mem_fun(this, &SourceOutputEffectsUi::on_app_added)));
  connections.emplace_back(
      soe->pm->source_output_changed.connect(sigc::mem_fun(this, &SourceOutputEffectsUi::on_app_changed)));
  connections.emplace_back(
      soe->pm->source_output_removed.connect(sigc::mem_fun(this, &SourceOutputEffectsUi::on_app_removed)));
}

SourceOutputEffectsUi::~SourceOutputEffectsUi() {
  soe->disable_spectrum();

  util::debug(log_tag + "destroyed");
}

auto SourceOutputEffectsUi::add_to_stack(Gtk::Stack* stack, SourceOutputEffects* soe_ptr) -> SourceOutputEffectsUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/effects_base.glade");

  auto settings = Gio::Settings::create("com.github.wwmm.pulseeffects.sourceoutputs");

  SourceOutputEffectsUi* ui = nullptr;

  builder->get_widget_derived("widgets_box", ui, settings, soe_ptr);

  stack->add(*ui, "source_outputs");
  stack->child_property_icon_name(*ui).set_value("audio-input-microphone-symbolic");

  return ui;
}

void SourceOutputEffectsUi::on_app_added(std::shared_ptr<AppInfo> app_info) {
  // Blacklist check
  auto forbidden_app = BlacklistSettingsUi::app_is_blacklisted(app_info->name, PresetType::input);

  if (forbidden_app) {
    app_info->visible = BlacklistSettingsUi::get_blacklisted_apps_visibility();

    if (!app_info->visible) {
      return;
    }
  } else {
    app_info->visible = true;
  }

  // Duplicate entry check
  for (const auto& a : apps_list) {
    if (a->app_info->index == app_info->index) {
      // do not add the same app two times in the interface
      return;
    }
  }

  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/app_info.glade");

  AppInfoUi* appui = nullptr;

  builder->get_widget_derived("widgets_grid", appui, app_info, pm);

  apps_box->add(*appui);

  apps_list.emplace_back(appui);
}

void SourceOutputEffectsUi::level_meters_connections() {
  // global output level meter connection

  connections.emplace_back(
      soe->global_output_level.connect(sigc::mem_fun(this, &SourceOutputEffectsUi::on_new_output_level_db)));

  // limiter level meters connections

  connections.emplace_back(
      soe->limiter->input_level.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_input_level)));
  connections.emplace_back(
      soe->limiter->output_level.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_output_level)));
  connections.emplace_back(
      soe->limiter->attenuation.connect(sigc::mem_fun(*limiter_ui, &LimiterUi::on_new_attenuation)));

  // compressor level meters connections

  connections.emplace_back(
      soe->compressor->input_level.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_input_level)));
  connections.emplace_back(
      soe->compressor->output_level.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_output_level)));
  connections.emplace_back(
      soe->compressor->reduction.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_reduction)));
  connections.emplace_back(
      soe->compressor->sidechain.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_sidechain)));
  connections.emplace_back(soe->compressor->curve.connect(sigc::mem_fun(*compressor_ui, &CompressorUi::on_new_curve)));

  // filter level meters connections

  connections.emplace_back(soe->filter->input_level.connect(sigc::mem_fun(*filter_ui, &FilterUi::on_new_input_level)));
  connections.emplace_back(
      soe->filter->output_level.connect(sigc::mem_fun(*filter_ui, &FilterUi::on_new_output_level)));

  // equalizer level meters connections

  connections.emplace_back(
      soe->equalizer_input_level.connect(sigc::mem_fun(*equalizer_ui, &EqualizerUi::on_new_input_level_db)));
  connections.emplace_back(
      soe->equalizer_output_level.connect(sigc::mem_fun(*equalizer_ui, &EqualizerUi::on_new_output_level_db)));

  // reverb level meters connections

  connections.emplace_back(soe->reverb->input_level.connect(sigc::mem_fun(*reverb_ui, &ReverbUi::on_new_input_level)));
  connections.emplace_back(
      soe->reverb->output_level.connect(sigc::mem_fun(*reverb_ui, &ReverbUi::on_new_output_level)));

  // gate level meters connections

  connections.emplace_back(soe->gate_input_level.connect(sigc::mem_fun(*gate_ui, &GateUi::on_new_input_level_db)));
  connections.emplace_back(soe->gate_output_level.connect(sigc::mem_fun(*gate_ui, &GateUi::on_new_output_level_db)));
  connections.emplace_back(soe->gate->gating.connect(sigc::mem_fun(*gate_ui, &GateUi::on_new_gating)));

  // deesser level meters connections

  connections.emplace_back(
      soe->deesser_input_level.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_input_level_db)));
  connections.emplace_back(
      soe->deesser_output_level.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_output_level_db)));
  connections.emplace_back(
      soe->deesser->compression.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_compression)));
  connections.emplace_back(soe->deesser->detected.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_detected)));

  // pitch level meters connections

  connections.emplace_back(soe->pitch_input_level.connect(sigc::mem_fun(*pitch_ui, &PitchUi::on_new_input_level_db)));
  connections.emplace_back(soe->pitch_output_level.connect(sigc::mem_fun(*pitch_ui, &PitchUi::on_new_output_level_db)));

  // webrtc level meters connections

  connections.emplace_back(
      soe->webrtc_input_level.connect(sigc::mem_fun(*webrtc_ui, &WebrtcUi::on_new_input_level_db)));
  connections.emplace_back(
      soe->webrtc_output_level.connect(sigc::mem_fun(*webrtc_ui, &WebrtcUi::on_new_output_level_db)));

  // multiband_compressor level meters connections

  connections.emplace_back(soe->multiband_compressor->input_level.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_input_level)));
  connections.emplace_back(soe->multiband_compressor->output_level.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_output_level)));

  connections.emplace_back(soe->multiband_compressor->output0.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_output0)));
  connections.emplace_back(soe->multiband_compressor->output1.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_output1)));
  connections.emplace_back(soe->multiband_compressor->output2.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_output2)));
  connections.emplace_back(soe->multiband_compressor->output3.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_output3)));

  connections.emplace_back(soe->multiband_compressor->compression0.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_compression0)));
  connections.emplace_back(soe->multiband_compressor->compression1.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_compression1)));
  connections.emplace_back(soe->multiband_compressor->compression2.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_compression2)));
  connections.emplace_back(soe->multiband_compressor->compression3.connect(
      sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_compression3)));

  // multiband_gate level meters connections

  connections.emplace_back(soe->multiband_gate->input_level.connect(
      sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_input_level)));
  connections.emplace_back(soe->multiband_gate->output_level.connect(
      sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output_level)));

  connections.emplace_back(
      soe->multiband_gate->output0.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output0)));
  connections.emplace_back(
      soe->multiband_gate->output1.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output1)));
  connections.emplace_back(
      soe->multiband_gate->output2.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output2)));
  connections.emplace_back(
      soe->multiband_gate->output3.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output3)));

  connections.emplace_back(
      soe->multiband_gate->gating0.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating0)));
  connections.emplace_back(
      soe->multiband_gate->gating1.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating1)));
  connections.emplace_back(
      soe->multiband_gate->gating2.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating2)));
  connections.emplace_back(
      soe->multiband_gate->gating3.connect(sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating3)));

  // stereo_tools level meters connections

  connections.emplace_back(
      soe->stereo_tools->input_level.connect(sigc::mem_fun(*stereo_tools_ui, &StereoToolsUi::on_new_input_level)));
  connections.emplace_back(
      soe->stereo_tools->output_level.connect(sigc::mem_fun(*stereo_tools_ui, &StereoToolsUi::on_new_output_level)));
}

void SourceOutputEffectsUi::up_down_connections() {
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

  connections.emplace_back(limiter_ui->plugin_up->signal_clicked().connect([=]() { on_up(limiter_ui); }));
  connections.emplace_back(limiter_ui->plugin_down->signal_clicked().connect([=]() { on_down(limiter_ui); }));

  connections.emplace_back(compressor_ui->plugin_up->signal_clicked().connect([=]() { on_up(compressor_ui); }));
  connections.emplace_back(compressor_ui->plugin_down->signal_clicked().connect([=]() { on_down(compressor_ui); }));

  connections.emplace_back(filter_ui->plugin_up->signal_clicked().connect([=]() { on_up(filter_ui); }));
  connections.emplace_back(filter_ui->plugin_down->signal_clicked().connect([=]() { on_down(filter_ui); }));

  connections.emplace_back(equalizer_ui->plugin_up->signal_clicked().connect([=]() { on_up(equalizer_ui); }));
  connections.emplace_back(equalizer_ui->plugin_down->signal_clicked().connect([=]() { on_down(equalizer_ui); }));

  connections.emplace_back(reverb_ui->plugin_up->signal_clicked().connect([=]() { on_up(reverb_ui); }));
  connections.emplace_back(reverb_ui->plugin_down->signal_clicked().connect([=]() { on_down(reverb_ui); }));

  connections.emplace_back(gate_ui->plugin_up->signal_clicked().connect([=]() { on_up(gate_ui); }));
  connections.emplace_back(gate_ui->plugin_down->signal_clicked().connect([=]() { on_down(gate_ui); }));

  connections.emplace_back(deesser_ui->plugin_up->signal_clicked().connect([=]() { on_up(deesser_ui); }));
  connections.emplace_back(deesser_ui->plugin_down->signal_clicked().connect([=]() { on_down(deesser_ui); }));

  connections.emplace_back(pitch_ui->plugin_up->signal_clicked().connect([=]() { on_up(pitch_ui); }));
  connections.emplace_back(pitch_ui->plugin_down->signal_clicked().connect([=]() { on_down(pitch_ui); }));

  connections.emplace_back(webrtc_ui->plugin_up->signal_clicked().connect([=]() { on_up(webrtc_ui); }));
  connections.emplace_back(webrtc_ui->plugin_down->signal_clicked().connect([=]() { on_down(webrtc_ui); }));

  connections.emplace_back(
      multiband_compressor_ui->plugin_up->signal_clicked().connect([=]() { on_up(multiband_compressor_ui); }));
  connections.emplace_back(
      multiband_compressor_ui->plugin_down->signal_clicked().connect([=]() { on_down(multiband_compressor_ui); }));

  connections.emplace_back(multiband_gate_ui->plugin_up->signal_clicked().connect([=]() { on_up(multiband_gate_ui); }));
  connections.emplace_back(
      multiband_gate_ui->plugin_down->signal_clicked().connect([=]() { on_down(multiband_gate_ui); }));

  connections.emplace_back(stereo_tools_ui->plugin_up->signal_clicked().connect([=]() { on_up(stereo_tools_ui); }));
  connections.emplace_back(stereo_tools_ui->plugin_down->signal_clicked().connect([=]() { on_down(stereo_tools_ui); }));
}
