use gio::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Crossfeed {
    state: bool,
    fcut: i32,
    feed: f64,
}

impl Default for Crossfeed {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.sinkinputs.crossfeed",
            "/com/github/wwmm/pulseeffects/sinkinputs/crossfeed/",
        );

        Crossfeed {
            state: settings.get_boolean("state"),
            fcut: settings.get_int("fcut"),
            feed: settings.get_double("feed"),
        }
    }
}
