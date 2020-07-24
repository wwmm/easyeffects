use crate::presets::common::update_key;
use gio::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct BassEnhancer {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    amount: f64,
    harmonics: f64,
    scope: f64,
    floor: f64,
    blend: f64,
    floor_active: bool,
    listen: bool,
}

impl Default for BassEnhancer {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.bassenhancer",
            "/com/github/wwmm/pulseeffects/sinkinputs/bassenhancer/",
        );

        BassEnhancer {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            amount: settings.get_double("amount"),
            harmonics: settings.get_double("harmonics"),
            scope: settings.get_double("scope"),
            floor: settings.get_double("floor"),
            blend: settings.get_double("blend"),
            floor_active: settings.get_boolean("floor-active"),
            listen: settings.get_boolean("listen"),
        }
    }
}

impl BassEnhancer {
    pub fn apply(&self) {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.bassenhancer",
            "/com/github/wwmm/pulseeffects/sinkinputs/bassenhancer/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);
        update_key(&settings, "amount", self.amount);
        update_key(&settings, "harmonics", self.harmonics);
        update_key(&settings, "scope", self.scope);
        update_key(&settings, "floor", self.floor);
        update_key(&settings, "blend", self.blend);
        update_key(&settings, "floor-active", self.floor_active);
        update_key(&settings, "listen", self.listen);
    }
}