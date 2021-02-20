use crate::presets::common::update_key;
use serde::{Deserialize, Serialize};

use gtk::gio;
use gio::prelude::*;

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
        let settings = gio::Settings::with_path(
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

impl Delay {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.delay",
            "/com/github/wwmm/pulseeffects/sinkinputs/delay/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);
        update_key(&settings, "time-l", self.time_l);
        update_key(&settings, "time-r", self.time_r);
    }
}
