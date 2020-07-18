use gio::prelude::*;
use serde::{Deserialize, Serialize};

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
        let settings = gio::Settings::new_with_path(
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
        let settings = gio::Settings::new_with_path(
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