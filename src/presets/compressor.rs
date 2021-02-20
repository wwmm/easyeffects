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
    mode: String,
    attack: f64,
    release: f64,
    release_threshold: f64,
    threshold: f64,
    ratio: f64,
    knee: f64,
    makeup: f64,
    boost_threshold: f64,
    sidechain: Sidechain,
    hpf_mode: String,
    hpf_frequency: f64,
    lpf_mode: String,
    lpf_frequency: f64,
}

impl Default for Output {
    fn default() -> Self {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.compressor",
            "/com/github/wwmm/pulseeffects/sinkinputs/compressor/",
        );

        Output {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            mode: settings.get_string("mode").unwrap().to_string(),
            attack: settings.get_double("attack"),
            release: settings.get_double("release"),
            release_threshold: settings.get_double("release-threshold"),
            threshold: settings.get_double("threshold"),
            ratio: settings.get_double("ratio"),
            knee: settings.get_double("knee"),
            makeup: settings.get_double("makeup"),
            boost_threshold: settings.get_double("boost-threshold"),
            sidechain: Sidechain {
                listen: settings.get_boolean("sidechain-listen"),
                style: settings.get_string("sidechain-type").unwrap().to_string(),
                mode: settings.get_string("sidechain-mode").unwrap().to_string(),
                source: settings.get_string("sidechain-source").unwrap().to_string(),
                preamp: settings.get_double("sidechain-preamp"),
                reactivity: settings.get_double("sidechain-reactivity"),
                lookahead: settings.get_double("sidechain-lookahead"),
            },
            hpf_mode: settings.get_string("hpf-mode").unwrap().to_string(),
            hpf_frequency: settings.get_double("hpf-frequency"),
            lpf_mode: settings.get_string("lpf-mode").unwrap().to_string(),
            lpf_frequency: settings.get_double("lpf-frequency"),
        }
    }
}

impl Output {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.compressor",
            "/com/github/wwmm/pulseeffects/sinkinputs/compressor/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);
        update_string_key(&settings, "mode", &self.mode);
        update_key(&settings, "attack", self.attack);
        update_key(&settings, "release", self.release);
        update_key(&settings, "release-threshold", self.release_threshold);
        update_key(&settings, "ratio", self.ratio);
        update_key(&settings, "knee", self.knee);
        update_key(&settings, "makeup", self.makeup);
        update_key(&settings, "boost-threshold", self.boost_threshold);
        update_key(&settings, "sidechain-listen", self.sidechain.listen);
        update_string_key(&settings, "sidechain-type", &self.sidechain.style);
        update_string_key(&settings, "sidechain-mode", &self.sidechain.mode);
        update_string_key(&settings, "sidechain-source", &self.sidechain.source);
        update_key(&settings, "sidechain-preamp", self.sidechain.preamp);
        update_key(&settings, "sidechain-reactivity", self.sidechain.reactivity);
        update_key(&settings, "sidechain-lookahead", self.sidechain.lookahead);
        update_string_key(&settings, "hpf-mode", &self.hpf_mode);
        update_key(&settings, "hpf-frequency", self.hpf_frequency);
        update_string_key(&settings, "lpf-mode", &self.lpf_mode);
        update_key(&settings, "lpf-frequency", self.lpf_frequency);
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Input {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    mode: String,
    attack: f64,
    release: f64,
    release_threshold: f64,
    threshold: f64,
    ratio: f64,
    knee: f64,
    makeup: f64,
    boost_threshold: f64,
    sidechain: Sidechain,
    hpf_mode: String,
    hpf_frequency: f64,
    lpf_mode: String,
    lpf_frequency: f64,
}

impl Default for Input {
    fn default() -> Self {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.compressor",
            "/com/github/wwmm/pulseeffects/sourceoutputs/compressor/",
        );

        Input {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            mode: settings.get_string("mode").unwrap().to_string(),
            attack: settings.get_double("attack"),
            release: settings.get_double("release"),
            release_threshold: settings.get_double("release-threshold"),
            threshold: settings.get_double("threshold"),
            ratio: settings.get_double("ratio"),
            knee: settings.get_double("knee"),
            makeup: settings.get_double("makeup"),
            boost_threshold: settings.get_double("boost-threshold"),
            sidechain: Sidechain {
                listen: settings.get_boolean("sidechain-listen"),
                style: settings.get_string("sidechain-type").unwrap().to_string(),
                mode: settings.get_string("sidechain-mode").unwrap().to_string(),
                source: settings.get_string("sidechain-source").unwrap().to_string(),
                preamp: settings.get_double("sidechain-preamp"),
                reactivity: settings.get_double("sidechain-reactivity"),
                lookahead: settings.get_double("sidechain-lookahead"),
            },
            hpf_mode: settings.get_string("hpf-mode").unwrap().to_string(),
            hpf_frequency: settings.get_double("hpf-frequency"),
            lpf_mode: settings.get_string("lpf-mode").unwrap().to_string(),
            lpf_frequency: settings.get_double("lpf-frequency"),
        }
    }
}

impl Input {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.compressor",
            "/com/github/wwmm/pulseeffects/sourceoutputs/compressor/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);
        update_string_key(&settings, "mode", &self.mode);
        update_key(&settings, "attack", self.attack);
        update_key(&settings, "release", self.release);
        update_key(&settings, "release-threshold", self.release_threshold);
        update_key(&settings, "ratio", self.ratio);
        update_key(&settings, "knee", self.knee);
        update_key(&settings, "makeup", self.makeup);
        update_key(&settings, "boost-threshold", self.boost_threshold);
        update_key(&settings, "sidechain-listen", self.sidechain.listen);
        update_string_key(&settings, "sidechain-type", &self.sidechain.style);
        update_string_key(&settings, "sidechain-mode", &self.sidechain.mode);
        update_string_key(&settings, "sidechain-source", &self.sidechain.source);
        update_key(&settings, "sidechain-preamp", self.sidechain.preamp);
        update_key(&settings, "sidechain-reactivity", self.sidechain.reactivity);
        update_key(&settings, "sidechain-lookahead", self.sidechain.lookahead);
        update_string_key(&settings, "hpf-mode", &self.hpf_mode);
        update_key(&settings, "hpf-frequency", self.hpf_frequency);
        update_string_key(&settings, "lpf-mode", &self.lpf_mode);
        update_key(&settings, "lpf-frequency", self.lpf_frequency);
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Sidechain {
    listen: bool,
    style: String,
    mode: String,
    source: String,
    preamp: f64,
    reactivity: f64,
    lookahead: f64,
}
