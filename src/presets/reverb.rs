use gio::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Output {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    room_size: String,
    decay_time: f64,
    hf_damp: f64,
    diffusion: f64,
    amount: f64,
    dry: f64,
    predelay: f64,
    bass_cut: f64,
    treble_cut: f64,
}

impl Default for Output {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.reverb",
            "/com/github/wwmm/pulseeffects/sinkinputs/reverb/",
        );

        Output {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            room_size: settings.get_string("room-size").unwrap().to_string(),
            decay_time: settings.get_double("decay-time"),
            hf_damp: settings.get_double("hf-damp"),
            diffusion: settings.get_double("diffusion"),
            amount: settings.get_double("amount"),
            dry: settings.get_double("dry"),
            predelay: settings.get_double("predelay"),
            bass_cut: settings.get_double("bass-cut"),
            treble_cut: settings.get_double("treble-cut"),
        }
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Input {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    room_size: String,
    decay_time: f64,
    hf_damp: f64,
    diffusion: f64,
    amount: f64,
    dry: f64,
    predelay: f64,
    bass_cut: f64,
    treble_cut: f64,
}

impl Default for Input {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.reverb",
            "/com/github/wwmm/pulseeffects/sourceoutputs/reverb/",
        );

        Input {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            room_size: settings.get_string("room-size").unwrap().to_string(),
            decay_time: settings.get_double("decay-time"),
            hf_damp: settings.get_double("hf-damp"),
            diffusion: settings.get_double("diffusion"),
            amount: settings.get_double("amount"),
            dry: settings.get_double("dry"),
            predelay: settings.get_double("predelay"),
            bass_cut: settings.get_double("bass-cut"),
            treble_cut: settings.get_double("treble-cut"),
        }
    }
}
