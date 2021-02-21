use crate::presets::common::{update_key, update_string_key};
use crate::presets::{equalizer_left, equalizer_right};
use serde::{Deserialize, Serialize};

use gio::prelude::*;

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Output {
    state: bool,
    mode: String,
    num_bands: i32,
    input_gain: f64,
    output_gain: f64,
    split_channels: bool,
    left: equalizer_left::LeftOutput,
    right: equalizer_right::RightOutput,
}

impl Default for Output {
    fn default() -> Self {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.equalizer",
            "/com/github/wwmm/pulseeffects/sinkinputs/equalizer/",
        );

        Output {
            state: settings.get_boolean("state"),
            mode: settings.get_string("mode").unwrap().to_string(),
            num_bands: settings.get_int("num-bands"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            split_channels: settings.get_boolean("split-channels"),
            left: equalizer_left::LeftOutput::default(),
            right: equalizer_right::RightOutput::default(),
        }
    }
}

impl Output {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.equalizer",
            "/com/github/wwmm/pulseeffects/sinkinputs/equalizer/",
        );

        update_key(&settings, "state", self.state);
        update_string_key(&settings, "mode", &self.mode);
        update_key(&settings, "num-bands", self.num_bands);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);
        update_key(&settings, "split-channels", self.split_channels);

        self.left.apply();
        self.right.apply();
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Input {
    state: bool,
    mode: String,
    num_bands: i32,
    input_gain: f64,
    output_gain: f64,
    split_channels: bool,
    left: equalizer_left::LeftInput,
    right: equalizer_right::RightInput,
}

impl Default for Input {
    fn default() -> Self {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.equalizer",
            "/com/github/wwmm/pulseeffects/sourceoutputs/equalizer/",
        );

        Input {
            state: settings.get_boolean("state"),
            mode: settings.get_string("mode").unwrap().to_string(),
            num_bands: settings.get_int("num-bands"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            split_channels: settings.get_boolean("split-channels"),
            left: equalizer_left::LeftInput::default(),
            right: equalizer_right::RightInput::default(),
        }
    }
}

impl Input {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.equalizer",
            "/com/github/wwmm/pulseeffects/sourceoutputs/equalizer/",
        );

        update_key(&settings, "state", self.state);
        update_string_key(&settings, "mode", &self.mode);
        update_key(&settings, "num-bands", self.num_bands);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);
        update_key(&settings, "split-channels", self.split_channels);

        self.left.apply();
        self.right.apply();
    }
}
