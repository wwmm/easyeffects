use gio::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Output {
    state: bool,
    input_gain: f64,
    limit: f64,
    lookahead: f64,
    release: f64,
    asc: bool,
    asc_level: f64,
    oversampling: i32,
}

impl Default for Output {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.limiter",
            "/com/github/wwmm/pulseeffects/sinkinputs/limiter/",
        );

        Output {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            limit: settings.get_double("limit"),
            lookahead: settings.get_double("lookahead"),
            release: settings.get_double("release"),
            asc: settings.get_boolean("asc"),
            asc_level: settings.get_double("asc-level"),
            oversampling: settings.get_int("oversampling"),
        }
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
    asc: bool,
    asc_level: f64,
    oversampling: i32,
}

impl Default for Input {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.limiter",
            "/com/github/wwmm/pulseeffects/sourceoutputs/limiter/",
        );

        Input {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            limit: settings.get_double("limit"),
            lookahead: settings.get_double("lookahead"),
            release: settings.get_double("release"),
            asc: settings.get_boolean("asc"),
            asc_level: settings.get_double("asc-level"),
            oversampling: settings.get_int("oversampling"),
        }
    }
}
