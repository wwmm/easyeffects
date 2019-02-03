#ifndef PIPELINE_BASE_HPP
#define PIPELINE_BASE_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <memory>
#include <mutex>
#include <vector>
#include "pulse_manager.hpp"
#include "realtime_kit.hpp"
#include "util.hpp"

class PipelineBase {
 public:
  PipelineBase(const std::string& tag, const uint& sampling_rate);
  virtual ~PipelineBase();

  bool playing = false;
  std::string log_tag;

  GstElement *pipeline = nullptr, *source = nullptr, *sink = nullptr,
             *spectrum = nullptr, *spectrum_bin = nullptr,
             *spectrum_identity_in = nullptr, *spectrum_identity_out = nullptr,
             *effects_bin = nullptr, *identity_in = nullptr,
             *identity_out = nullptr;

  GstBus* bus = nullptr;

  GSettings* settings = nullptr;

  std::unique_ptr<RealtimeKit> rtkit;

  std::mutex pipeline_mutex;

  GstClockTime state_check_timeout = 5 * GST_SECOND;

  bool resizing_spectrum = false;
  uint min_spectrum_freq = 20;     // Hz
  uint max_spectrum_freq = 20000;  // Hz
  int spectrum_threshold = -120;   // dB
  uint spectrum_nbands = 1600, spectrum_nfreqs;
  float spline_f0, spline_df;
  std::vector<float> spectrum_freqs, spectrum_x_axis;
  std::vector<float> spectrum_mag_tmp, spectrum_mag;

  void enable_spectrum();
  void disable_spectrum();
  std::array<double, 2> get_peak(GstMessage* message);

  void set_source_monitor_name(std::string name);
  void set_output_sink_name(std::string name);
  void set_null_pipeline();
  void update_pipeline_state();
  void get_latency();
  void init_spectrum(const uint& sampling_rate);
  void update_spectrum_interval(const double& value);

  sigc::signal<void, std::vector<float>> new_spectrum;
  sigc::signal<void, int> new_latency;

 protected:
  void set_pulseaudio_props(std::string props);

  void on_app_added(const std::shared_ptr<AppInfo>& app_info);
  void on_app_changed(const std::shared_ptr<AppInfo>& app_info);
  void on_app_removed(uint idx);

 private:
  GstElement* capsfilter = nullptr;

  std::vector<std::shared_ptr<AppInfo>> apps_list;

  void set_caps(const uint& sampling_rate);
  void init_spectrum_bin();
  void init_effects_bin();

  GstElement* get_required_plugin(const gchar* factoryname, const gchar* name);
};

namespace {

template <typename T>
void update_effects_order(gpointer user_data) {
  auto l = static_cast<T>(user_data);

  if (!gst_element_is_locked_state(l->effects_bin)) {
    if (!gst_element_set_locked_state(l->effects_bin, true)) {
      util::debug(l->log_tag + " could not lock state changes");
    }
  }

  // unlinking elements using old plugins order

  gst_element_unlink(l->identity_in, l->plugins[l->plugins_order_old[0]]);

  for (long unsigned int n = 1; n < l->plugins_order_old.size(); n++) {
    gst_element_unlink(l->plugins[l->plugins_order_old[n - 1]],
                       l->plugins[l->plugins_order_old[n]]);
  }

  gst_element_unlink(
      l->plugins[l->plugins_order_old[l->plugins_order_old.size() - 1]],
      l->identity_out);

  // linking elements using the new plugins order

  gst_element_link(l->identity_in, l->plugins[l->plugins_order[0]]);

  for (long unsigned int n = 1; n < l->plugins_order.size(); n++) {
    gst_element_link(l->plugins[l->plugins_order[n - 1]],
                     l->plugins[l->plugins_order[n]]);
  }

  gst_element_link(l->plugins[l->plugins_order[l->plugins_order.size() - 1]],
                   l->identity_out);

  for (auto& p : l->plugins) {
    gst_element_sync_state_with_parent(p.second);
  }

  gst_element_set_locked_state(l->effects_bin, false);

  gst_element_sync_state_with_parent(l->effects_bin);

  std::string list;

  for (auto name : l->plugins_order) {
    list += name + ",";
  }

  util::debug(l->log_tag + "new plugins order: [" + list + "]");
}

template <typename T>
GstPadProbeReturn on_pad_idle(GstPad* pad,
                              GstPadProbeInfo* info,
                              gpointer user_data) {
  auto l = static_cast<T>(user_data);

  std::lock_guard<std::mutex> lock(l->pipeline_mutex);

  update_effects_order<T>(user_data);

  return GST_PAD_PROBE_REMOVE;
}

template <typename T>
void on_plugins_order_changed(GSettings* settings, gchar* key, T* l) {
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

    gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_IDLE, on_pad_idle<T*>, l,
                      nullptr);

    g_object_unref(srcpad);
  }
}

}  // namespace

#endif
