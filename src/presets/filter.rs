use gio::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
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
        let settings = gio::Settings::new_with_path(
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

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
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
        let settings = gio::Settings::new_with_path(
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
