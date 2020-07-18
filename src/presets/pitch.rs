use gio::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
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

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
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