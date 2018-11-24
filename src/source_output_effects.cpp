#include "source_output_effects.hpp"
#include "util.hpp"

extern std::mutex pipeline_mutex;

namespace {

void on_message_element(const GstBus* gst_bus,
                        GstMessage* message,
                        SourceOutputEffects* soe) {
  auto src_name = GST_OBJECT_NAME(message->src);

  if (src_name == std::string("compressor_input_level")) {
    soe->compressor_input_level.emit(soe->get_peak(message));
  } else if (src_name == std::string("compressor_output_level")) {
    soe->compressor_output_level.emit(soe->get_peak(message));
  } else if (src_name == std::string("equalizer_input_level")) {
    soe->equalizer_input_level.emit(soe->get_peak(message));
  } else if (src_name == std::string("equalizer_output_level")) {
    soe->equalizer_output_level.emit(soe->get_peak(message));
  } else if (src_name == std::string("gate_input_level")) {
    soe->gate_input_level.emit(soe->get_peak(message));
  } else if (src_name == std::string("gate_output_level")) {
    soe->gate_output_level.emit(soe->get_peak(message));
  } else if (src_name == std::string("deesser_input_level")) {
    soe->deesser_input_level.emit(soe->get_peak(message));
  } else if (src_name == std::string("deesser_output_level")) {
    soe->deesser_output_level.emit(soe->get_peak(message));
  } else if (src_name == std::string("pitch_input_level")) {
    soe->pitch_input_level.emit(soe->get_peak(message));
  } else if (src_name == std::string("pitch_output_level")) {
    soe->pitch_output_level.emit(soe->get_peak(message));
  } else if (src_name == std::string("webrtc_input_level")) {
    soe->webrtc_input_level.emit(soe->get_peak(message));
  } else if (src_name == std::string("webrtc_output_level")) {
    soe->webrtc_output_level.emit(soe->get_peak(message));
  }
}

void update_order(gpointer user_data) {
  auto l = static_cast<SourceOutputEffects*>(user_data);

  // unlinking elements using old plugins order

  gst_element_unlink(l->identity_in, l->plugins[l->plugins_order_old[0]]);

  for (long unsigned int n = 1; n < l->plugins_order_old.size(); n++) {
    gst_element_unlink(l->plugins[l->plugins_order_old[n - 1]],
                       l->plugins[l->plugins_order_old[n]]);
  }

  gst_element_unlink(
      l->plugins[l->plugins_order_old[l->plugins_order_old.size() - 1]],
      l->identity_out);

  // setting state

  for (auto& p : l->plugins) {
    gst_element_set_state(p.second, GST_STATE_NULL);
    gst_element_sync_state_with_parent(p.second);
  }

  // linking elements using the new plugins order

  gst_element_link(l->identity_in, l->plugins[l->plugins_order[0]]);

  for (long unsigned int n = 1; n < l->plugins_order.size(); n++) {
    gst_element_link(l->plugins[l->plugins_order[n - 1]],
                     l->plugins[l->plugins_order[n]]);
  }

  gst_element_link(l->plugins[l->plugins_order[l->plugins_order.size() - 1]],
                   l->identity_out);

  std::string list;

  for (auto name : l->plugins_order) {
    list += name + ",";
  }

  util::debug(l->log_tag + "new plugins order: [" + list + "]");
}

static GstPadProbeReturn event_probe_cb(GstPad* pad,
                                        GstPadProbeInfo* info,
                                        gpointer user_data) {
  if (GST_EVENT_TYPE(GST_PAD_PROBE_INFO_DATA(info)) != GST_EVENT_EOS) {
    return GST_PAD_PROBE_PASS;
  }

  gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));

  std::lock_guard<std::mutex> lock(pipeline_mutex);

  update_order(user_data);

  return GST_PAD_PROBE_DROP;
}

GstPadProbeReturn on_pad_blocked(GstPad* pad,
                                 GstPadProbeInfo* info,
                                 gpointer user_data) {
  gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));

  auto l = static_cast<SourceOutputEffects*>(user_data);

  auto srcpad = gst_element_get_static_pad(
      l->plugins[l->plugins_order_old[l->plugins_order_old.size() - 1]], "src");

  gst_pad_add_probe(
      srcpad,
      static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BLOCK |
                                   GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM),
      event_probe_cb, user_data, nullptr);

  auto sinkpad =
      gst_element_get_static_pad(l->plugins[l->plugins_order_old[0]], "sink");

  gst_pad_send_event(sinkpad, gst_event_new_eos());

  gst_object_unref(sinkpad);
  gst_object_unref(srcpad);

  return GST_PAD_PROBE_OK;
}

GstPadProbeReturn on_pad_idle(GstPad* pad,
                              GstPadProbeInfo* info,
                              gpointer user_data) {
  gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));

  update_order(user_data);

  return GST_PAD_PROBE_OK;
}
void on_plugins_order_changed(GSettings* settings,
                              gchar* key,
                              SourceOutputEffects* l) {
  bool update = false;
  gchar* name;
  GVariantIter* iter;

  g_settings_get(settings, "plugins", "as", &iter);

  l->plugins_order_old = l->plugins_order;
  l->plugins_order.clear();

  while (g_variant_iter_next(iter, "s", &name)) {
    l->plugins_order.push_back(name);
    g_free(name);
  }

  g_variant_iter_free(iter);

  if (l->plugins_order.size() != l->plugins_order_old.size()) {
    update = true;
  } else if (!std::equal(l->plugins_order.begin(), l->plugins_order.end(),
                         l->plugins_order_old.begin())) {
    update = true;
  }

  if (update) {
    auto srcpad = gst_element_get_static_pad(l->identity_in, "src");

    GstState state, pending;

    gst_element_get_state(l->effects_bin, &state, &pending,
                          l->state_check_timeout);

    if (state != GST_STATE_PLAYING) {
      gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_IDLE, on_pad_idle, l,
                        nullptr);
    } else {
      gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
                        on_pad_blocked, l, nullptr);
    }

    g_object_unref(srcpad);
  }
}

void on_blocksize_changed(GSettings* settings,
                          gchar* key,
                          SourceOutputEffects* l) {
  GstState state, pending;

  gst_element_get_state(l->pipeline, &state, &pending, l->state_check_timeout);

  if (state == GST_STATE_PLAYING || state == GST_STATE_PAUSED) {
    gst_element_set_state(l->pipeline, GST_STATE_READY);

    l->update_pipeline_state();
  }
}

}  // namespace

SourceOutputEffects::SourceOutputEffects(PulseManager* pulse_manager)
    : PipelineBase("soe: ", pulse_manager->mic_sink_info->rate),
      log_tag("soe: "),
      pm(pulse_manager),
      soe_settings(
          g_settings_new("com.github.wwmm.pulseeffects.sourceoutputs")) {
  std::string pulse_props =
      "application.id=com.github.wwmm.pulseeffects.sourceoutputs";

  set_pulseaudio_props(pulse_props);

  set_output_sink_name("PulseEffects_mic");

  auto PULSE_SOURCE = std::getenv("PULSE_SOURCE");

  if (PULSE_SOURCE) {
    if (pm->get_source_info(PULSE_SOURCE)) {
      set_source_monitor_name(PULSE_SOURCE);
    } else {
      set_source_monitor_name(pm->server_info.default_source_name);
    }
  } else {
    bool use_default_source =
        g_settings_get_boolean(settings, "use-default-source");

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

  pm->source_output_added.connect(
      sigc::mem_fun(*this, &SourceOutputEffects::on_app_added));
  pm->source_output_changed.connect(
      sigc::mem_fun(*this, &SourceOutputEffects::on_app_changed));
  pm->source_output_removed.connect(
      sigc::mem_fun(*this, &SourceOutputEffects::on_app_removed));

  g_settings_bind(settings, "buffer-in", source, "buffer-time",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(settings, "latency-in", source, "latency-time",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(settings, "buffer-in", sink, "buffer-time",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(settings, "latency-in", sink, "latency-time",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(settings, "blocksize-in", adapter, "blocksize",
                  G_SETTINGS_BIND_DEFAULT);

  // element message callback

  g_signal_connect(bus, "message::element", G_CALLBACK(on_message_element),
                   this);

  g_signal_connect(settings, "changed::blocksize-in",
                   G_CALLBACK(on_blocksize_changed), this);

  limiter = std::make_unique<Limiter>(
      log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.limiter");
  compressor = std::make_unique<Compressor>(
      log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.compressor");
  filter = std::make_unique<Filter>(
      log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.filter");
  equalizer = std::make_unique<Equalizer>(
      log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.equalizer");
  reverb = std::make_unique<Reverb>(
      log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.reverb");
  gate = std::make_unique<Gate>(
      log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.gate");
  deesser = std::make_unique<Deesser>(
      log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.deesser");
  pitch = std::make_unique<Pitch>(
      log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.pitch");
  webrtc = std::make_unique<Webrtc>(
      log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.webrtc",
      pm->mic_sink_info->rate);
  multiband_compressor = std::make_unique<MultibandCompressor>(
      log_tag,
      "com.github.wwmm.pulseeffects.sourceoutputs.multibandcompressor");
  multiband_gate = std::make_unique<MultibandGate>(
      log_tag, "com.github.wwmm.pulseeffects.sourceoutputs.multibandgate");

  plugins.insert(std::make_pair(limiter->name, limiter->plugin));
  plugins.insert(std::make_pair(compressor->name, compressor->plugin));
  plugins.insert(std::make_pair(filter->name, filter->plugin));
  plugins.insert(std::make_pair(equalizer->name, equalizer->plugin));
  plugins.insert(std::make_pair(reverb->name, reverb->plugin));
  plugins.insert(std::make_pair(gate->name, gate->plugin));
  plugins.insert(std::make_pair(deesser->name, deesser->plugin));
  plugins.insert(std::make_pair(pitch->name, pitch->plugin));
  plugins.insert(std::make_pair(webrtc->name, webrtc->plugin));
  plugins.insert(
      std::make_pair(multiband_compressor->name, multiband_compressor->plugin));
  plugins.insert(std::make_pair(multiband_gate->name, multiband_gate->plugin));

  add_plugins_to_pipeline();

  g_signal_connect(soe_settings, "changed::plugins",
                   G_CALLBACK(on_plugins_order_changed), this);
}

SourceOutputEffects::~SourceOutputEffects() {
  g_object_unref(soe_settings);

  util::debug(log_tag + "destroyed");
}

void SourceOutputEffects::on_app_added(
    const std::shared_ptr<AppInfo>& app_info) {
  PipelineBase::on_app_added(app_info);

  auto enable_all_apps = g_settings_get_boolean(settings, "enable-all-apps");

  if (enable_all_apps && !app_info->connected) {
    pm->move_source_output_to_pulseeffects(app_info->name, app_info->index);
  }
}

void SourceOutputEffects::add_plugins_to_pipeline() {
  gchar* name;
  GVariantIter* iter;
  std::vector<std::string> default_order;

  g_settings_get(soe_settings, "plugins", "as", &iter);

  while (g_variant_iter_next(iter, "s", &name)) {
    plugins_order.push_back(name);
    g_free(name);
  }

  auto gvariant = g_settings_get_default_value(soe_settings, "plugins");

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

    g_settings_reset(soe_settings, "plugins");
  }

  for (auto v : plugins_order) {
    // checking if the plugin exists. If not we reset the list to default

    if (std::find(default_order.begin(), default_order.end(), v) ==
        default_order.end()) {
      plugins_order = default_order;

      g_settings_reset(soe_settings, "plugins");

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

  gst_element_link(plugins[plugins_order[plugins_order.size() - 1]],
                   identity_out);
}
