#include "source_output_effects.hpp"
#include <cstring>
#include "pipeline_common.hpp"

namespace {

void on_message_element(const GstBus* gst_bus, GstMessage* message, SourceOutputEffects* soe) {
  auto* src_name = GST_OBJECT_NAME(message->src);

  if (std::strcmp(src_name, "global_level_meter") == 0) {
    soe->global_output_level.emit(SourceOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "equalizer_input_level") == 0) {
    soe->equalizer_input_level.emit(SourceOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "equalizer_output_level") == 0) {
    soe->equalizer_output_level.emit(SourceOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "gate_input_level") == 0) {
    soe->gate_input_level.emit(SourceOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "gate_output_level") == 0) {
    soe->gate_output_level.emit(SourceOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "deesser_input_level") == 0) {
    soe->deesser_input_level.emit(SourceOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "deesser_output_level") == 0) {
    soe->deesser_output_level.emit(SourceOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "pitch_input_level") == 0) {
    soe->pitch_input_level.emit(SourceOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "pitch_output_level") == 0) {
    soe->pitch_output_level.emit(SourceOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "webrtc_input_level") == 0) {
    soe->webrtc_input_level.emit(SourceOutputEffects::get_peak(message));
  } else if (std::strcmp(src_name, "webrtc_output_level") == 0) {
    soe->webrtc_output_level.emit(SourceOutputEffects::get_peak(message));
  }
}

}  // namespace

SourceOutputEffects::SourceOutputEffects(PulseManager* pulse_manager) : PipelineBase("soe: ", pulse_manager) {
  std::string pulse_props = "application.id=com.github.wwmm.pulseeffects.sourceoutputs";

  child_settings = g_settings_new("com.github.wwmm.pulseeffects.sourceoutputs");

  set_pulseaudio_props(pulse_props);
  set_output_sink_name("PulseEffects_mic");
  set_caps(pm->mic_sink_info->rate);

  auto* PULSE_SOURCE = std::getenv("PULSE_SOURCE");

  if (PULSE_SOURCE != nullptr) {
    if (pm->get_source_info(PULSE_SOURCE)) {
      set_source_monitor_name(PULSE_SOURCE);
    } else {
      set_source_monitor_name(pm->server_info.default_source_name);
    }
  } else {
    bool use_default_source = g_settings_get_boolean(settings, "use-default-source") != 0;

    if (use_default_source) {
      set_source_monitor_name(pm->server_info.default_source_name);
    } else {
      gchar* custom_source = g_settings_get_string(settings, "custom-source");

      if (pm->get_source_info(custom_source)) {
        set_source_monitor_name(custom_source);
      } else {
        set_source_monitor_name(pm->server_info.default_source_name);
      }

      g_free(custom_source);
    }
  }

  pm->source_output_added.connect(sigc::mem_fun(*this, &SourceOutputEffects::on_app_added));
  pm->source_output_changed.connect(sigc::mem_fun(*this, &SourceOutputEffects::on_app_changed));
  pm->source_output_removed.connect(sigc::mem_fun(*this, &SourceOutputEffects::on_app_removed));
  pm->source_changed.connect(sigc::mem_fun(*this, &SourceOutputEffects::on_source_changed));

  g_settings_bind(child_settings, "buffer-pulsesrc", source, "buffer-time", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(child_settings, "latency-pulsesrc", source, "latency-time", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(child_settings, "buffer-pulsesink", sink, "buffer-time", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(child_settings, "latency-pulsesink", sink, "latency-time", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "blocksize-in", adapter, "blocksize", G_SETTINGS_BIND_DEFAULT);

  // element message callback

  g_signal_connect(bus, "message::element", G_CALLBACK(on_message_element), this);

  limiter = std::make_unique<Limiter>(log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.limiter");
  compressor = std::make_unique<Compressor>(log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.compressor");
  filter = std::make_unique<Filter>(log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.filter");
  equalizer = std::make_unique<Equalizer>(log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.equalizer",
                                          "com.github.wwmm.pulseeffects.sourceoutputs.equalizer.leftchannel",
                                          "com.github.wwmm.pulseeffects.sourceoutputs.equalizer.rightchannel");
  reverb = std::make_unique<Reverb>(log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.reverb");
  gate = std::make_unique<Gate>(log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.gate");
  deesser = std::make_unique<Deesser>(log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.deesser");
  pitch = std::make_unique<Pitch>(log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.pitch");
  webrtc =
      std::make_unique<Webrtc>(log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.webrtc", pm->mic_sink_info->rate);
  multiband_compressor =
      std::make_unique<MultibandCompressor>(log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.multibandcompressor");
  multiband_gate = std::make_unique<MultibandGate>(log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.multibandgate");

  plugins.insert(std::make_pair(limiter->name, limiter->plugin));
  plugins.insert(std::make_pair(compressor->name, compressor->plugin));
  plugins.insert(std::make_pair(filter->name, filter->plugin));
  plugins.insert(std::make_pair(equalizer->name, equalizer->plugin));
  plugins.insert(std::make_pair(reverb->name, reverb->plugin));
  plugins.insert(std::make_pair(gate->name, gate->plugin));
  plugins.insert(std::make_pair(deesser->name, deesser->plugin));
  plugins.insert(std::make_pair(pitch->name, pitch->plugin));
  plugins.insert(std::make_pair(webrtc->name, webrtc->plugin));
  plugins.insert(std::make_pair(multiband_compressor->name, multiband_compressor->plugin));
  plugins.insert(std::make_pair(multiband_gate->name, multiband_gate->plugin));

  add_plugins_to_pipeline();

  g_signal_connect(child_settings, "changed::plugins", G_CALLBACK(on_plugins_order_changed<SourceOutputEffects>), this);
}

SourceOutputEffects::~SourceOutputEffects() {
  util::debug(log_tag + "destroyed");
}

void SourceOutputEffects::on_app_added(const std::shared_ptr<AppInfo>& app_info) {
  PipelineBase::on_app_added(app_info);

  bool forbidden_app = false, success = false;
  auto* blacklist = g_settings_get_strv(settings, "blacklist-in");

  for (std::size_t i = 0; blacklist[i] != nullptr; i++) {
    if (app_info->name == blacklist[i]) {
      forbidden_app = true;
    }

    g_free(blacklist[i]);
  }

  if (app_info->connected) {
    if (forbidden_app) {
      success = pm->remove_source_output_from_pulseeffects(app_info->name, app_info->index);

      if (success) {
        app_info->connected = false;
      }
    }
  } else {
    auto enable_all = g_settings_get_boolean(settings, "enable-all-sourceoutputs");

    if (!forbidden_app && (enable_all != 0)) {
      success = pm->move_source_output_to_pulseeffects(app_info->name, app_info->index);

      if (success) {
        app_info->connected = true;
      }
    }
  }

  g_free(blacklist);
}

void SourceOutputEffects::add_plugins_to_pipeline() {
  gchar* name = nullptr;
  GVariantIter* iter = nullptr;
  std::vector<std::string> default_order;

  g_settings_get(child_settings, "plugins", "as", &iter);

  while (g_variant_iter_next(iter, "s", &name) != 0) {
    plugins_order.emplace_back(name);
    g_free(name);
  }

  auto* gvariant = g_settings_get_default_value(child_settings, "plugins");

  g_variant_get(gvariant, "as", &iter);

  g_variant_unref(gvariant);

  while (g_variant_iter_next(iter, "s", &name) != 0) {
    default_order.emplace_back(name);
    g_free(name);
  }

  g_variant_iter_free(iter);

  // updating user list if there is any new plugin

  if (plugins_order.size() != default_order.size()) {
    plugins_order = default_order;

    g_settings_reset(child_settings, "plugins");
  }

  for (const auto& v : plugins_order) {
    // checking if the plugin exists. If not we reset the list to default

    if (std::find(default_order.begin(), default_order.end(), v) == default_order.end()) {
      plugins_order = default_order;

      g_settings_reset(child_settings, "plugins");

      break;
    }
  }

  // adding plugins to effects_bin

  for (const auto& p : plugins) {
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
