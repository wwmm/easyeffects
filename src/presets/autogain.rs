use crate::presets::common::update_key;
use serde::{Deserialize, Serialize};

use gtk::gio;
use gio::prelude::*;

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct AutoGain {
    state: bool,
    detect_silence: bool,
    use_geometric_mean: bool,
    input_gain: f64,
    output_gain: f64,
    target: f64,
    weight_m: i32,
    weight_s: i32,
    weight_i: i32,
}

impl Default for AutoGain {
    fn default() -> Self {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.autogain",
            "/com/github/wwmm/pulseeffects/sinkinputs/autogain/",
        );

        AutoGain {
            state: settings.get_boolean("state"),
            detect_silence: settings.get_boolean("detect-silence"),
            use_geometric_mean: settings.get_boolean("use-geometric-mean"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            target: settings.get_double("target"),
            weight_m: settings.get_int("weight-m"),
            weight_s: settings.get_int("weight-s"),
            weight_i: settings.get_int("weight-i"),
        }
    }
}

impl AutoGain {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.autogain",
            "/com/github/wwmm/pulseeffects/sinkinputs/autogain/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "detect-silence", self.detect_silence);
        update_key(&settings, "use-geometric-mean", self.use_geometric_mean);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);
        update_key(&settings, "target", self.target);
        update_key(&settings, "weight-m", self.weight_m);
        update_key(&settings, "weight-s", self.weight_s);
        update_key(&settings, "weight-i", self.weight_i);
    }
}
