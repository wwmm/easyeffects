use crate::presets::common::update_key;
use serde::{Deserialize, Serialize};

use gio::prelude::*;

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Loudness {
    state: bool,
    loudness: f64,
    output: f64,
    link: f64,
}

impl Default for Loudness {
    fn default() -> Self {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.loudness",
            "/com/github/wwmm/pulseeffects/sinkinputs/loudness/",
        );

        Loudness {
            state: settings.get_boolean("state"),
            loudness: settings.get_double("loudness"),
            output: settings.get_double("output"),
            link: settings.get_double("link"),
        }
    }
}

impl Loudness {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.loudness",
            "/com/github/wwmm/pulseeffects/sinkinputs/loudness/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "loudness", self.loudness);
        update_key(&settings, "output", self.output);
        update_key(&settings, "link", self.link);
    }
}