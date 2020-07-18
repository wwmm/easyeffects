use gio::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Output {
    state: bool,
    detection: String,
    stereo_link: String,
    range: f64,
    attack: f64,
    release: f64,
    threshold: f64,
    ratio: f64,
    knee: f64,
    makeup: f64,
}

impl Default for Output {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.gate",
            "/com/github/wwmm/pulseeffects/sinkinputs/gate/",
        );

        Output {
            state: settings.get_boolean("state"),
            detection: settings.get_string("detection").unwrap().to_string(),
            stereo_link: settings.get_string("stereo-link").unwrap().to_string(),
            range: settings.get_double("range"),
            attack: settings.get_double("attack"),
            release: settings.get_double("release"),
            threshold: settings.get_double("threshold"),
            ratio: settings.get_double("ratio"),
            knee: settings.get_double("knee"),
            makeup: settings.get_double("makeup"),
        }
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Input {
    state: bool,
    detection: String,
    stereo_link: String,
    range: f64,
    attack: f64,
    release: f64,
    threshold: f64,
    ratio: f64,
    knee: f64,
    makeup: f64,
}

impl Default for Input {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.gate",
            "/com/github/wwmm/pulseeffects/sourceoutputs/gate/",
        );

        Input {
            state: settings.get_boolean("state"),
            detection: settings.get_string("detection").unwrap().to_string(),
            stereo_link: settings.get_string("stereo-link").unwrap().to_string(),
            range: settings.get_double("range"),
            attack: settings.get_double("attack"),
            release: settings.get_double("release"),
            threshold: settings.get_double("threshold"),
            ratio: settings.get_double("ratio"),
            knee: settings.get_double("knee"),
            makeup: settings.get_double("makeup"),
        }
    }
}