#include "sink_input_effects.hpp"
#include "pipeline_common.hpp"

namespace {

void on_message_element(const GstBus* gst_bus, GstMessage* message, SinkInputEffects* sie) {
  auto src_name = GST_OBJECT_NAME(message->src);

  if (src_name == std::string("pitch_input_level")) {
    sie->pitch_input_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("pitch_output_level")) {
    sie->pitch_output_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("equalizer_input_level")) {
    sie->equalizer_input_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("equalizer_output_level")) {
    sie->equalizer_output_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("bass_enhancer_input_level")) {
    sie->bass_enhancer_input_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("bass_enhancer_output_level")) {
    sie->bass_enhancer_output_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("exciter_input_level")) {
    sie->exciter_input_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("exciter_output_level")) {
    sie->exciter_output_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("crossfeed_input_level")) {
    sie->crossfeed_input_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("crossfeed_output_level")) {
    sie->crossfeed_output_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("maximizer_input_level")) {
    sie->maximizer_input_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("maximizer_output_level")) {
    sie->maximizer_output_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("loudness_input_level")) {
    sie->loudness_input_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("loudness_output_level")) {
    sie->loudness_output_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("gate_input_level")) {
    sie->gate_input_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("gate_output_level")) {
    sie->gate_output_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("deesser_input_level")) {
    sie->deesser_input_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("deesser_output_level")) {
    sie->deesser_output_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("convolver_input_level")) {
    sie->convolver_input_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("convolver_output_level")) {
    sie->convolver_output_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("crystalizer_input_level")) {
    sie->crystalizer_input_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("crystalizer_output_level")) {
    sie->crystalizer_output_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("autogain_input_level")) {
    sie->autogain_input_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("autogain_output_level")) {
    sie->autogain_output_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("delay_input_level")) {
    sie->delay_input_level.emit(sie->get_peak(message));
  } else if (src_name == std::string("delay_output_level")) {
    sie->delay_output_level.emit(sie->get_peak(message));
  }
}

}  // namespace

SinkInputEffects::SinkInputEffects(PulseManager* pulse_manager) : PipelineBase("sie: ", pulse_manager) {
  std::string pulse_props = "application.id=com.github.wwmm.pulseeffects.sinkinputs";

  child_settings = g_settings_new("com.github.wwmm.pulseeffects.sinkinputs");

  set_pulseaudio_props(pulse_props);

  set_source_monitor_name(pm->apps_sink_info->monitor_source_name);
  set_caps(pm->apps_sink_info->rate);

  auto PULSE_SINK = std::getenv("PULSE_SINK");

  if (PULSE_SINK) {
    if (pm->get_sink_info(PULSE_SINK)) {
      set_output_sink_name(PULSE_SINK);
    } else {
      set_output_sink_name(pm->server_info.default_sink_name);
    }
  } else {
    bool use_default_sink = g_settings_get_boolean(settings, "use-default-sink");

    if (use_default_sink) {
      set_output_sink_name(pm->server_info.default_sink_name);
    } else {
      gchar* custom_sink = g_settings_get_string(settings, "custom-sink");

      if (pm->get_sink_info(custom_sink)) {
        set_output_sink_name(custom_sink);
      } else {
        set_output_sink_name(pm->server_info.default_sink_name);
      }

      g_free(custom_sink);
    }
  }

  pm->sink_input_added.connect(sigc::mem_fun(*this, &SinkInputEffects::on_app_added));
  pm->sink_input_changed.connect(sigc::mem_fun(*this, &SinkInputEffects::on_app_changed));
  pm->sink_input_removed.connect(sigc::mem_fun(*this, &SinkInputEffects::on_app_removed));
  pm->sink_changed.connect(sigc::mem_fun(*this, &SinkInputEffects::on_sink_changed));

  g_settings_bind(child_settings, "buffer-pulsesrc", source, "buffer-time", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(child_settings, "latency-pulsesrc", source, "latency-time", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(child_settings, "buffer-pulsesink", sink, "buffer-time", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(child_settings, "latency-pulsesink", sink, "latency-time", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "blocksize-out", adapter, "blocksize", G_SETTINGS_BIND_DEFAULT);

  // element message callback

  g_signal_connect(bus, "message::element", G_CALLBACK(on_message_element), this);

  limiter = std::make_unique<Limiter>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.limiter");
  compressor = std::make_unique<Compressor>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.compressor");
  filter = std::make_unique<Filter>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.filter");
  equalizer = std::make_unique<Equalizer>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.equalizer",
                                          "com.github.wwmm.pulseeffects.sinkinputs.equalizer.leftchannel",
                                          "com.github.wwmm.pulseeffects.sinkinputs.equalizer.rightchannel");
  reverb = std::make_unique<Reverb>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.reverb");
  bass_enhancer = std::make_unique<BassEnhancer>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.bassenhancer");
  exciter = std::make_unique<Exciter>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.exciter");
  crossfeed = std::make_unique<Crossfeed>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.crossfeed");
  maximizer = std::make_unique<Maximizer>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.maximizer");
  multiband_compressor =
      std::make_unique<MultibandCompressor>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.multibandcompressor");
  loudness = std::make_unique<Loudness>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.loudness");
  gate = std::make_unique<Gate>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.gate");
  pitch = std::make_unique<Pitch>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.pitch");
  multiband_gate = std::make_unique<MultibandGate>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.multibandgate");
  deesser = std::make_unique<Deesser>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.deesser");
  stereo_tools = std::make_unique<StereoTools>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.stereotools");
  convolver = std::make_unique<Convolver>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.convolver");
  crystalizer = std::make_unique<Crystalizer>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.crystalizer");
  autogain = std::make_unique<AutoGain>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.autogain");
  delay = std::make_unique<Delay>(log_tag, "com.github.wwmm.pulseeffects.sinkinputs.delay");

  plugins.insert(std::make_pair(limiter->name, limiter->plugin));
  plugins.insert(std::make_pair(compressor->name, compressor->plugin));
  plugins.insert(std::make_pair(filter->name, filter->plugin));
  plugins.insert(std::make_pair(equalizer->name, equalizer->plugin));
  plugins.insert(std::make_pair(reverb->name, reverb->plugin));
  plugins.insert(std::make_pair(bass_enhancer->name, bass_enhancer->plugin));
  plugins.insert(std::make_pair(exciter->name, exciter->plugin));
  plugins.insert(std::make_pair(crossfeed->name, crossfeed->plugin));
  plugins.insert(std::make_pair(maximizer->name, maximizer->plugin));
  plugins.insert(std::make_pair(multiband_compressor->name, multiband_compressor->plugin));
  plugins.insert(std::make_pair(loudness->name, loudness->plugin));
  plugins.insert(std::make_pair(gate->name, gate->plugin));
  plugins.insert(std::make_pair(pitch->name, pitch->plugin));
  plugins.insert(std::make_pair(multiband_gate->name, multiband_gate->plugin));
  plugins.insert(std::make_pair(deesser->name, deesser->plugin));
  plugins.insert(std::make_pair(stereo_tools->name, stereo_tools->plugin));
  plugins.insert(std::make_pair(convolver->name, convolver->plugin));
  plugins.insert(std::make_pair(crystalizer->name, crystalizer->plugin));
  plugins.insert(std::make_pair(autogain->name, autogain->plugin));
  plugins.insert(std::make_pair(delay->name, delay->plugin));

  add_plugins_to_pipeline();

  g_signal_connect(child_settings, "changed::plugins", G_CALLBACK(on_plugins_order_changed<SinkInputEffects>), this);
}

SinkInputEffects::~SinkInputEffects() {
  util::debug(log_tag + "destroyed");
}

void SinkInputEffects::on_app_added(const std::shared_ptr<AppInfo>& app_info) {
  PipelineBase::on_app_added(app_info);

  auto enable_all = g_settings_get_boolean(settings, "enable-all-sinkinputs");

  if (enable_all && !app_info->connected) {
    pm->move_sink_input_to_pulseeffects(app_info->name, app_info->index);
  }
}

void SinkInputEffects::add_plugins_to_pipeline() {
  gchar* name;
  GVariantIter* iter;
  std::vector<std::string> default_order;

  g_settings_get(child_settings, "plugins", "as", &iter);

  while (g_variant_iter_next(iter, "s", &name)) {
    plugins_order.push_back(name);
    g_free(name);
  }

  auto gvariant = g_settings_get_default_value(child_settings, "plugins");

  g_variant_get(gvariant, "as", &iter);

  g_variant_unref(gvariant);

  while (g_variant_iter_next(iter, "s", &name)) {
    default_order.push_back(name);
    g_free(name);
  }

  g_variant_iter_free(iter);

  // updating user list if there is any new plugin

  if (plugins_order.size() != default_order.size()) {
    plugins_order = default_order;

    g_settings_reset(child_settings, "plugins");
  }

  for (auto v : plugins_order) {
    // checking if the plugin exists. If not we reset the list to default

    if (std::find(default_order.begin(), default_order.end(), v) == default_order.end()) {
      plugins_order = default_order;

      g_settings_reset(child_settings, "plugins");

      break;
    }
  }

  // adding plugins to effects_bin

  for (auto& p : plugins) {
    gst_bin_add(GST_BIN(effects_bin), p.second);
  }

  // linking plugins

  gst_element_unlink(identity_in, identity_out);

  gst_element_link(identity_in, plugins[plugins_order[0]]);

  for (long unsigned int n = 1; n < plugins_order.size(); n++) {
    gst_element_link(plugins[plugins_order[n - 1]], plugins[plugins_order[n]]);
  }

  gst_element_link(plugins[plugins_order[plugins_order.size() - 1]], identity_out);
}
