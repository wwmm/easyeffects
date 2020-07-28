use crate::presets::common::{update_key, update_string_key};
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
    input: f64,
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
            input: settings.get_double("input"),
            makeup: settings.get_double("makeup"),
        }
    }
}

impl Output {
    pub fn apply(&self) {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.gate",
            "/com/github/wwmm/pulseeffects/sinkinputs/gate/",
        );

        update_key(&settings, "state", self.state);
        update_string_key(&settings, "detection", &self.detection);
        update_string_key(&settings, "stereo-link", &self.stereo_link);
        update_key(&settings, "range", self.range);
        update_key(&settings, "attack", self.attack);
        update_key(&settings, "release", self.release);
        update_key(&settings, "threshold", self.threshold);
        update_key(&settings, "ratio", self.ratio);
        update_key(&settings, "knee", self.knee);
        update_key(&settings, "input", self.input);
        update_key(&settings, "makeup", self.makeup);
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
    input: f64,
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
            input: settings.get_double("input"),
            makeup: settings.get_double("makeup"),
        }
    }
}

impl Input {
    pub fn apply(&self) {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.gate",
            "/com/github/wwmm/pulseeffects/sourceoutputs/gate/",
        );

        update_key(&settings, "state", self.state);
        update_string_key(&settings, "detection", &self.detection);
        update_string_key(&settings, "stereo-link", &self.stereo_link);
        update_key(&settings, "range", self.range);
        update_key(&settings, "attack", self.attack);
        update_key(&settings, "release", self.release);
        update_key(&settings, "threshold", self.threshold);
        update_key(&settings, "ratio", self.ratio);
        update_key(&settings, "knee", self.knee);
        update_key(&settings, "input", self.input);
        update_key(&settings, "makeup", self.makeup);
    }
}