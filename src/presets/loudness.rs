use gio::prelude::*;
use serde::{Deserialize, Serialize};

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
        let settings = gio::Settings::new_with_path(
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
