use gio::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Delay {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    time_l: f64,
    time_r: f64,
}

impl Default for Delay {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.delay",
            "/com/github/wwmm/pulseeffects/sinkinputs/delay/",
        );

        Delay {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            time_l: settings.get_double("time-l"),
            time_r: settings.get_double("time-r"),
        }
    }
}