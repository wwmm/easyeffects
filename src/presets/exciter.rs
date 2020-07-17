use gio::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
pub struct Exciter {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    amount: f64,
    harmonics: f64,
    scope: f64,
    ceil: f64,
    blend: f64,
    ceil_active: bool,
    listen: bool,
}

impl Default for Exciter {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.exciter",
            "/com/github/wwmm/pulseeffects/sinkinputs/exciter/",
        );

        Exciter {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            amount: settings.get_double("amount"),
            harmonics: settings.get_double("harmonics"),
            scope: settings.get_double("scope"),
            ceil: settings.get_double("ceil"),
            blend: settings.get_double("blend"),
            ceil_active: settings.get_boolean("ceil-active"),
            listen: settings.get_boolean("listen"),
        }
    }
}