use crate::presets::common::update_key;
use gio::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Output {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    cents: f64,
    semitones: i32,
    octaves: i32,
    crispness: i32,
    formant_preserving: bool,
    faster: bool,
}

impl Default for Output {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.pitch",
            "/com/github/wwmm/pulseeffects/sinkinputs/pitch/",
        );

        Output {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            cents: settings.get_double("cents"),
            semitones: settings.get_int("semitones"),
            octaves: settings.get_int("octaves"),
            crispness: settings.get_int("crispness"),
            formant_preserving: settings.get_boolean("formant-preserving"),
            faster: settings.get_boolean("faster"),
        }
    }
}

impl Output {
    pub fn apply(&self) {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.pitch",
            "/com/github/wwmm/pulseeffects/sinkinputs/pitch/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);
        update_key(&settings, "cents", self.cents);
        update_key(&settings, "semitones", self.semitones);
        update_key(&settings, "octaves", self.octaves);
        update_key(&settings, "crispness", self.crispness);
        update_key(&settings, "formant-preserving", self.formant_preserving);
        update_key(&settings, "faster", self.faster);
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Input {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    cents: f64,
    semitones: i32,
    octaves: i32,
    crispness: i32,
    formant_preserving: bool,
    faster: bool,
}

impl Default for Input {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.pitch",
            "/com/github/wwmm/pulseeffects/sourceoutputs/pitch/",
        );

        Input {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            cents: settings.get_double("cents"),
            semitones: settings.get_int("semitones"),
            octaves: settings.get_int("octaves"),
            crispness: settings.get_int("crispness"),
            formant_preserving: settings.get_boolean("formant-preserving"),
            faster: settings.get_boolean("faster"),
        }
    }
}

impl Input {
    pub fn apply(&self) {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.pitch",
            "/com/github/wwmm/pulseeffects/sourceoutputs/pitch/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);
        update_key(&settings, "cents", self.cents);
        update_key(&settings, "semitones", self.semitones);
        update_key(&settings, "octaves", self.octaves);
        update_key(&settings, "crispness", self.crispness);
        update_key(&settings, "formant-preserving", self.formant_preserving);
        update_key(&settings, "faster", self.faster);
    }
}
