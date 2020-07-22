use gio::prelude::*;
use serde::{Deserialize, Serialize};

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
        let settings = gio::Settings::new_with_path(
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
        let settings = gio::Settings::new_with_path(
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
