use crate::presets::common::update_key;
use serde::{Deserialize, Serialize};

use gio::prelude::*;

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
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
        let settings = gio::Settings::with_path(
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

impl Exciter {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.exciter",
            "/com/github/wwmm/pulseeffects/sinkinputs/exciter/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);
        update_key(&settings, "amount", self.amount);
        update_key(&settings, "harmonics", self.harmonics);
        update_key(&settings, "scope", self.scope);
        update_key(&settings, "ceil", self.ceil);
        update_key(&settings, "blend", self.blend);
        update_key(&settings, "ceil-active", self.ceil_active);
        update_key(&settings, "listen", self.listen);
    }
}
