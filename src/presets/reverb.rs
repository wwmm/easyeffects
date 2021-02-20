use crate::presets::common::{update_key, update_string_key};
use serde::{Deserialize, Serialize};

use gtk::gio;
use gio::prelude::*;

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
        let settings = gio::Settings::with_path(
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

impl Output {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.reverb",
            "/com/github/wwmm/pulseeffects/sinkinputs/reverb/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);
        update_string_key(&settings, "room-size", &self.room_size);
        update_key(&settings, "decay-time", self.decay_time);
        update_key(&settings, "hf-damp", self.hf_damp);
        update_key(&settings, "diffusion", self.diffusion);
        update_key(&settings, "amount", self.amount);
        update_key(&settings, "dry", self.dry);
        update_key(&settings, "predelay", self.predelay);
        update_key(&settings, "bass-cut", self.bass_cut);
        update_key(&settings, "treble-cut", self.treble_cut);
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
        let settings = gio::Settings::with_path(
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

impl Input {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.reverb",
            "/com/github/wwmm/pulseeffects/sourceoutputs/reverb/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);
        update_string_key(&settings, "room-size", &self.room_size);
        update_key(&settings, "decay-time", self.decay_time);
        update_key(&settings, "hf-damp", self.hf_damp);
        update_key(&settings, "diffusion", self.diffusion);
        update_key(&settings, "amount", self.amount);
        update_key(&settings, "dry", self.dry);
        update_key(&settings, "predelay", self.predelay);
        update_key(&settings, "bass-cut", self.bass_cut);
        update_key(&settings, "treble-cut", self.treble_cut);
    }
}