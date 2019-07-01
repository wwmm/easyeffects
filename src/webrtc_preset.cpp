#include "webrtc_preset.hpp"

WebrtcPreset::WebrtcPreset()
    : input_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sourceoutputs.webrtc")) {}

void WebrtcPreset::save(boost::property_tree::ptree& root,
                        const std::string& section,
                        const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".webrtc.state", settings->get_boolean("state"));

  root.put(section + ".webrtc.high-pass-filter", settings->get_boolean("high-pass-filter"));

  root.put(section + ".webrtc.echo-cancel", settings->get_boolean("echo-cancel"));

  root.put(section + ".webrtc.echo-suppression-level", settings->get_string("echo-suppression-level"));

  root.put(section + ".webrtc.noise-suppression", settings->get_boolean("noise-suppression"));

  root.put(section + ".webrtc.noise-suppression-level", settings->get_string("noise-suppression-level"));

  root.put(section + ".webrtc.gain-control", settings->get_boolean("gain-control"));

  root.put(section + ".webrtc.extended-filter", settings->get_boolean("extended-filter"));

  root.put(section + ".webrtc.delay-agnostic", settings->get_boolean("delay-agnostic"));

  root.put(section + ".webrtc.target-level-dbfs", settings->get_int("target-level-dbfs"));

  root.put(section + ".webrtc.compression-gain-db", settings->get_int("compression-gain-db"));

  root.put(section + ".webrtc.limiter", settings->get_boolean("limiter"));

  root.put(section + ".webrtc.gain-control-mode", settings->get_string("gain-control-mode"));

  root.put(section + ".webrtc.voice-detection", settings->get_boolean("voice-detection"));

  root.put(section + ".webrtc.voice-detection-frame-size-ms", settings->get_int("voice-detection-frame-size-ms"));

  root.put(section + ".webrtc.voice-detection-likelihood", settings->get_string("voice-detection-likelihood"));
}

void WebrtcPreset::load(boost::property_tree::ptree& root,
                        const std::string& section,
                        const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(root, settings, "state", section + ".webrtc.state");

  update_key<bool>(root, settings, "high-pass-filter", section + ".webrtc.high-pass-filter");

  update_key<bool>(root, settings, "echo-cancel", section + ".webrtc.echo-cancel");

  update_string_key(root, settings, "echo-suppression-level", section + ".webrtc.echo-suppression-level");

  update_key<bool>(root, settings, "noise-suppression", section + ".webrtc.noise-suppression");

  update_string_key(root, settings, "noise-suppression-level", section + ".webrtc.noise-suppression-level");

  update_key<bool>(root, settings, "gain-control", section + ".webrtc.gain-control");

  update_key<bool>(root, settings, "extended-filter", section + ".webrtc.extended-filter");

  update_key<bool>(root, settings, "delay-agnostic", section + ".webrtc.delay-agnostic");

  update_key<int>(root, settings, "target-level-dbfs", section + ".webrtc.target-level-dbfs");

  update_key<int>(root, settings, "compression-gain-db", section + ".webrtc.compression-gain-db");

  update_key<bool>(root, settings, "limiter", section + ".webrtc.limiter");

  update_string_key(root, settings, "gain-control-mode", section + ".webrtc.gain-control-mode");

  update_key<bool>(root, settings, "voice-detection", section + ".webrtc.voice-detection");

  update_key<int>(root, settings, "voice-detection-frame-size-ms", section + ".webrtc.voice-detection-frame-size-ms");

  update_string_key(root, settings, "voice-detection-likelihood", section + ".webrtc.voice-detection-likelihood");
}

void WebrtcPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::input) {
    save(root, "input", input_settings);
  }
}

void WebrtcPreset::read(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::input) {
    load(root, "input", input_settings);
  }
}
