#include "webrtc_preset.hpp"

WebrtcPreset::WebrtcPreset()
    : input_settings(Gio::Settings::create(
          "com.github.wwmm.pulseeffects.sourceoutputs.webrtc")) {}

void WebrtcPreset::save(boost::property_tree::ptree& root,
                        const std::string& section,
                        const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".webrtc.state", settings->get_boolean("state"));

  root.put(section + ".webrtc.high-pass-filter",
           settings->get_boolean("high-pass-filter"));

  root.put(section + ".webrtc.echo-cancel",
           settings->get_boolean("echo-cancel"));

  root.put(section + ".webrtc.echo-suppression-level",
           settings->get_string("echo-suppression-level"));

  root.put(section + ".webrtc.noise-suppression",
           settings->get_boolean("noise-suppression"));

  root.put(section + ".webrtc.noise-suppression-level",
           settings->get_string("noise-suppression-level"));

  root.put(section + ".webrtc.gain-control",
           settings->get_boolean("gain-control"));

  root.put(section + ".webrtc.extended-filter",
           settings->get_boolean("extended-filter"));

  root.put(section + ".webrtc.delay-agnostic",
           settings->get_boolean("delay-agnostic"));

  root.put(section + ".webrtc.target-level-dbfs",
           settings->get_int("target-level-dbfs"));

  root.put(section + ".webrtc.compression-gain-db",
           settings->get_int("compression-gain-db"));

  root.put(section + ".webrtc.limiter", settings->get_boolean("limiter"));

  root.put(section + ".webrtc.gain-control-mode",
           settings->get_string("gain-control-mode"));

  root.put(section + ".webrtc.voice-detection",
           settings->get_boolean("voice-detection"));

  root.put(section + ".webrtc.voice-detection-frame-size-ms",
           settings->get_int("voice-detection-frame-size-ms"));

  root.put(section + ".webrtc.voice-detection-likelihood",
           settings->get_string("voice-detection-likelihood"));
}

void WebrtcPreset::load(boost::property_tree::ptree& root,
                        const std::string& section,
                        const Glib::RefPtr<Gio::Settings>& settings) {
  settings->set_boolean("state",
                        root.get<bool>(section + ".webrtc.state",
                                       get_default<bool>(settings, "state")));

  settings->set_boolean(
      "high-pass-filter",
      root.get<bool>(section + ".webrtc.high-pass-filter",
                     get_default<bool>(settings, "high-pass-filter")));

  settings->set_boolean(
      "echo-cancel",
      root.get<bool>(section + ".webrtc.echo-cancel",
                     get_default<bool>(settings, "echo-cancel")));

  settings->set_string(
      "echo-suppression-level",
      root.get<std::string>(
          section + ".webrtc.echo-suppression-level",
          get_default<std::string>(settings, "echo-suppression-level")));

  settings->set_boolean(
      "noise-suppression",
      root.get<bool>(section + ".webrtc.noise-suppression",
                     get_default<bool>(settings, "noise-suppression")));

  settings->set_string(
      "noise-suppression-level",
      root.get<std::string>(
          section + ".webrtc.noise-suppression-level",
          get_default<std::string>(settings, "noise-suppression-level")));

  settings->set_boolean(
      "gain-control",
      root.get<bool>(section + ".webrtc.gain-control",
                     get_default<bool>(settings, "gain-control")));

  settings->set_boolean(
      "extended-filter",
      root.get<bool>(section + ".webrtc.extended-filter",
                     get_default<bool>(settings, "extended-filter")));

  settings->set_boolean(
      "delay-agnostic",
      root.get<bool>(section + ".webrtc.delay-agnostic",
                     get_default<bool>(settings, "delay-agnostic")));

  settings->set_int(
      "target-level-dbfs",
      root.get<int>(section + ".webrtc.target-level-dbfs",
                    get_default<int>(settings, "target-level-dbfs")));

  settings->set_int(
      "compression-gain-db",
      root.get<int>(section + ".webrtc.compression-gain-db",
                    get_default<int>(settings, "compression-gain-db")));

  settings->set_boolean("limiter",
                        root.get<bool>(section + ".webrtc.limiter",
                                       get_default<bool>(settings, "limiter")));

  settings->set_string(
      "gain-control-mode",
      root.get<std::string>(
          section + ".webrtc.gain-control-mode",
          get_default<std::string>(settings, "gain-control-mode")));

  settings->set_boolean(
      "voice-detection",
      root.get<bool>(section + ".webrtc.voice-detection",
                     get_default<bool>(settings, "voice-detection")));

  settings->set_int(
      "voice-detection-frame-size-ms",
      root.get<int>(
          section + ".webrtc.voice-detection-frame-size-ms",
          get_default<int>(settings, "voice-detection-frame-size-ms")));

  settings->set_string(
      "voice-detection-likelihood",
      root.get<std::string>(
          section + ".webrtc.voice-detection-likelihood",
          get_default<std::string>(settings, "voice-detection-likelihood")));
}

void WebrtcPreset::write(boost::property_tree::ptree& root) {
  save(root, "input", input_settings);
}

void WebrtcPreset::read(boost::property_tree::ptree& root) {
  load(root, "input", input_settings);
}
