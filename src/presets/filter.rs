use crate::presets::common::{update_key, update_string_key};
use serde::{Deserialize, Serialize};

use gio::prelude::*;

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Output {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    frequency: f64,
    resonance: f64,
    mode: String,
    inertia: f64,
}

impl Default for Output {
    fn default() -> Self {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.filter",
            "/com/github/wwmm/pulseeffects/sinkinputs/filter/",
        );

        Output {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            frequency: settings.get_double("frequency"),
            resonance: settings.get_double("resonance"),
            mode: settings.get_string("mode").unwrap().to_string(),
            inertia: settings.get_double("inertia"),
        }
    }
}

impl Output {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.filter",
            "/com/github/wwmm/pulseeffects/sinkinputs/filter/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);
        update_key(&settings, "frequency", self.frequency);
        update_key(&settings, "resonance", self.resonance);
        update_string_key(&settings, "mode", &self.mode);
        update_key(&settings, "inertia", self.inertia);
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Input {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    frequency: f64,
    resonance: f64,
    mode: String,
    inertia: f64,
}

impl Default for Input {
    fn default() -> Self {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.filter",
            "/com/github/wwmm/pulseeffects/sourceoutputs/filter/",
        );

        Input {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            frequency: settings.get_double("frequency"),
            resonance: settings.get_double("resonance"),
            mode: settings.get_string("mode").unwrap().to_string(),
            inertia: settings.get_double("inertia"),
        }
    }
}

impl Input {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.filter",
            "/com/github/wwmm/pulseeffects/sourceoutputs/filter/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);
        update_key(&settings, "frequency", self.frequency);
        update_key(&settings, "resonance", self.resonance);
        update_string_key(&settings, "mode", &self.mode);
        update_key(&settings, "inertia", self.inertia);
    }
}