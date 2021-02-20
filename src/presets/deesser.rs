use crate::presets::common::{update_key, update_string_key};
use serde::{Deserialize, Serialize};

use gtk::gio;
use gio::prelude::*;

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Output {
    state: bool,
    detection: String,
    mode: String,
    threshold: f64,
    ratio: f64,
    laxity: i32,
    makeup: f64,
    f1_freq: f64,
    f2_freq: f64,
    f1_level: f64,
    f2_level: f64,
    f2_q: f64,
    sc_listen: bool,
}

impl Default for Output {
    fn default() -> Self {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.deesser",
            "/com/github/wwmm/pulseeffects/sinkinputs/deesser/",
        );

        Output {
            state: settings.get_boolean("state"),
            detection: settings.get_string("detection").unwrap().to_string(),
            mode: settings.get_string("mode").unwrap().to_string(),
            threshold: settings.get_double("threshold"),
            ratio: settings.get_double("ratio"),
            laxity: settings.get_int("laxity"),
            makeup: settings.get_double("makeup"),
            f1_freq: settings.get_double("f1-freq"),
            f2_freq: settings.get_double("f2-freq"),
            f1_level: settings.get_double("f1-level"),
            f2_level: settings.get_double("f2-level"),
            f2_q: settings.get_double("f2-q"),
            sc_listen: settings.get_boolean("sc-listen"),
        }
    }
}

impl Output {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.deesser",
            "/com/github/wwmm/pulseeffects/sinkinputs/deesser/",
        );

        update_key(&settings, "state", self.state);
        update_string_key(&settings, "detection", &self.detection);
        update_string_key(&settings, "mode", &self.mode);
        update_key(&settings, "threshold", self.threshold);
        update_key(&settings, "ratio", self.ratio);
        update_key(&settings, "laxity", self.laxity);
        update_key(&settings, "makeup", self.makeup);
        update_key(&settings, "f1-freq", self.f1_freq);
        update_key(&settings, "f1-level", self.f1_level);
        update_key(&settings, "f2-freq", self.f2_freq);
        update_key(&settings, "f2-level", self.f2_level);
        update_key(&settings, "ratio", self.ratio);
        update_key(&settings, "f2-q", self.f2_q);
        update_key(&settings, "sc-listen", self.sc_listen);
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Input {
    state: bool,
    detection: String,
    mode: String,
    threshold: f64,
    ratio: f64,
    laxity: i32,
    makeup: f64,
    f1_freq: f64,
    f2_freq: f64,
    f1_level: f64,
    f2_level: f64,
    f2_q: f64,
    sc_listen: bool,
}

impl Default for Input {
    fn default() -> Self {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.deesser",
            "/com/github/wwmm/pulseeffects/sourceoutputs/deesser/",
        );

        Input {
            state: settings.get_boolean("state"),
            detection: settings.get_string("detection").unwrap().to_string(),
            mode: settings.get_string("mode").unwrap().to_string(),
            threshold: settings.get_double("threshold"),
            ratio: settings.get_double("ratio"),
            laxity: settings.get_int("laxity"),
            makeup: settings.get_double("makeup"),
            f1_freq: settings.get_double("f1-freq"),
            f2_freq: settings.get_double("f2-freq"),
            f1_level: settings.get_double("f1-level"),
            f2_level: settings.get_double("f2-level"),
            f2_q: settings.get_double("f2-q"),
            sc_listen: settings.get_boolean("sc-listen"),
        }
    }
}

impl Input {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.deesser",
            "/com/github/wwmm/pulseeffects/sourceoutputs/deesser/",
        );

        update_key(&settings, "state", self.state);
        update_string_key(&settings, "detection", &self.detection);
        update_string_key(&settings, "mode", &self.mode);
        update_key(&settings, "threshold", self.threshold);
        update_key(&settings, "ratio", self.ratio);
        update_key(&settings, "laxity", self.laxity);
        update_key(&settings, "makeup", self.makeup);
        update_key(&settings, "f1-freq", self.f1_freq);
        update_key(&settings, "f1-level", self.f1_level);
        update_key(&settings, "f2-freq", self.f2_freq);
        update_key(&settings, "f2-level", self.f2_level);
        update_key(&settings, "ratio", self.ratio);
        update_key(&settings, "f2-q", self.f2_q);
        update_key(&settings, "sc-listen", self.sc_listen);
    }
}