use crate::presets::common::{update_key, update_string_key};
use serde::{Deserialize, Serialize};

use gtk::gio;
use gio::prelude::*;

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct WebRTC {
    state: bool,
    high_pass_filter: bool,
    echo_cancel: bool,
    echo_suppression_level: String,
    noise_suppression: bool,
    noise_suppression_level: String,
    gain_control: bool,
    extended_filter: bool,
    delay_agnostic: bool,
    target_level_dbfs: i32,
    compression_gain_db: i32,
    limiter: bool,
    gain_control_mode: String,
    voice_detection: bool,
    voice_detection_frame_size_ms: i32,
    voice_detection_likelihood: String,
}

impl Default for WebRTC {
    fn default() -> Self {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.webrtc",
            "/com/github/wwmm/pulseeffects/sourceoutputs/webrtc/",
        );

        WebRTC {
            state: settings.get_boolean("state"),
            high_pass_filter: settings.get_boolean("high-pass-filter"),
            echo_cancel: settings.get_boolean("echo-cancel"),
            echo_suppression_level: settings
                .get_string("echo-suppression-level")
                .unwrap()
                .to_string(),
            noise_suppression: settings.get_boolean("noise-suppression"),
            noise_suppression_level: settings
                .get_string("noise-suppression-level")
                .unwrap()
                .to_string(),
            gain_control: settings.get_boolean("gain-control"),
            extended_filter: settings.get_boolean("extended-filter"),
            delay_agnostic: settings.get_boolean("delay-agnostic"),
            target_level_dbfs: settings.get_int("target-level-dbfs"),
            compression_gain_db: settings.get_int("compression-gain-db"),
            limiter: settings.get_boolean("limiter"),
            gain_control_mode: settings
                .get_string("gain-control-mode")
                .unwrap()
                .to_string(),
            voice_detection: settings.get_boolean("voice-detection"),
            voice_detection_frame_size_ms: settings.get_int("voice-detection-frame-size-ms"),
            voice_detection_likelihood: settings
                .get_string("voice-detection-likelihood")
                .unwrap()
                .to_string(),
        }
    }
}

impl WebRTC {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.webrtc",
            "/com/github/wwmm/pulseeffects/sourceoutputs/webrtc/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "high-pass-filter", self.high_pass_filter);
        update_key(&settings, "echo-cancel", self.echo_cancel);
        update_string_key(
            &settings,
            "echo-suppression-level",
            &self.echo_suppression_level,
        );
        update_key(&settings, "noise-suppression", self.noise_suppression);
        update_string_key(
            &settings,
            "noise-suppression-level",
            &self.noise_suppression_level,
        );
        update_key(&settings, "gain-control", self.gain_control);
        update_key(&settings, "extended-filter", self.extended_filter);
        update_key(&settings, "delay-agnostic", self.delay_agnostic);
        update_key(&settings, "target-level-dbfs", self.target_level_dbfs);
        update_key(&settings, "compression-gain-db", self.compression_gain_db);
        update_key(&settings, "limiter", self.limiter);
        update_string_key(&settings, "gain-control-mode", &self.gain_control_mode);
        update_key(&settings, "voice-detection", self.voice_detection);
        update_key(
            &settings,
            "voice-detection-frame-size-ms",
            self.voice_detection_frame_size_ms,
        );
        update_string_key(
            &settings,
            "voice-detection-likelihood",
            &self.voice_detection_likelihood,
        );
    }
}
