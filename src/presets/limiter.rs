use crate::presets::common::update_key;
use serde::{Deserialize, Serialize};

use gtk::gio;
use gio::prelude::*;

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Output {
    state: bool,
    input_gain: f64,
    limit: f64,
    lookahead: f64,
    release: f64,
    auto_level: bool,
    asc: bool,
    asc_level: f64,
    oversampling: i32,
}

impl Default for Output {
    fn default() -> Self {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.limiter",
            "/com/github/wwmm/pulseeffects/sinkinputs/limiter/",
        );

        Output {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            limit: settings.get_double("limit"),
            lookahead: settings.get_double("lookahead"),
            release: settings.get_double("release"),
            auto_level: settings.get_boolean("auto-level"),
            asc: settings.get_boolean("asc"),
            asc_level: settings.get_double("asc-level"),
            oversampling: settings.get_int("oversampling"),
        }
    }
}

impl Output {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.limiter",
            "/com/github/wwmm/pulseeffects/sinkinputs/limiter/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "limit", self.limit);
        update_key(&settings, "lookahead", self.lookahead);
        update_key(&settings, "release", self.release);
        update_key(&settings, "auto-level", self.auto_level);
        update_key(&settings, "asc", self.asc);
        update_key(&settings, "asc-level", self.asc_level);
        update_key(&settings, "oversampling", self.oversampling);
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Input {
    state: bool,
    input_gain: f64,
    limit: f64,
    lookahead: f64,
    release: f64,
    auto_level: bool,
    asc: bool,
    asc_level: f64,
    oversampling: i32,
}

impl Default for Input {
    fn default() -> Self {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.limiter",
            "/com/github/wwmm/pulseeffects/sourceoutputs/limiter/",
        );

        Input {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            limit: settings.get_double("limit"),
            lookahead: settings.get_double("lookahead"),
            release: settings.get_double("release"),
            auto_level: settings.get_boolean("auto-level"),
            asc: settings.get_boolean("asc"),
            asc_level: settings.get_double("asc-level"),
            oversampling: settings.get_int("oversampling"),
        }
    }
}

impl Input {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.limiter",
            "/com/github/wwmm/pulseeffects/sourceoutputs/limiter/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "limit", self.limit);
        update_key(&settings, "lookahead", self.lookahead);
        update_key(&settings, "release", self.release);
        update_key(&settings, "auto-level", self.auto_level);
        update_key(&settings, "asc", self.asc);
        update_key(&settings, "asc-level", self.asc_level);
        update_key(&settings, "oversampling", self.oversampling);
    }
}
