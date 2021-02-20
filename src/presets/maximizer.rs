use crate::presets::common::update_key;
use serde::{Deserialize, Serialize};

use gtk::gio;
use gio::prelude::*;

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Output {
    state: bool,
    release: f64,
    ceiling: f64,
    threshold: f64,
}

impl Default for Output {
    fn default() -> Self {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.maximizer",
            "/com/github/wwmm/pulseeffects/sinkinputs/maximizer/",
        );

        Output {
            state: settings.get_boolean("state"),
            release: settings.get_double("release"),
            ceiling: settings.get_double("ceiling"),
            threshold: settings.get_double("threshold"),
        }
    }
}

impl Output {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.maximizer",
            "/com/github/wwmm/pulseeffects/sinkinputs/maximizer/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "release", self.release);
        update_key(&settings, "ceiling", self.ceiling);
        update_key(&settings, "threshold", self.threshold);
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Input {
    state: bool,
    release: f64,
    ceiling: f64,
    threshold: f64,
}

impl Default for Input {
    fn default() -> Self {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.maximizer",
            "/com/github/wwmm/pulseeffects/sourceoutputs/maximizer/",
        );

        Input {
            state: settings.get_boolean("state"),
            release: settings.get_double("release"),
            ceiling: settings.get_double("ceiling"),
            threshold: settings.get_double("threshold"),
        }
    }
}

impl Input {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.maximizer",
            "/com/github/wwmm/pulseeffects/sourceoutputs/maximizer/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "release", self.release);
        update_key(&settings, "ceiling", self.ceiling);
        update_key(&settings, "threshold", self.threshold);
    }
}
