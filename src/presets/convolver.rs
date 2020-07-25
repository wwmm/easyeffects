use crate::presets::common::{update_key, update_string_key};
use gio::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Convolver {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    kernel_path: String,
    ir_width: i32,
}

impl Default for Convolver {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.convolver",
            "/com/github/wwmm/pulseeffects/sinkinputs/convolver/",
        );

        Convolver {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            kernel_path: settings.get_string("kernel-path").unwrap().to_string(),
            ir_width: settings.get_int("ir-width"),
        }
    }
}

impl Convolver {
    pub fn apply(&self) {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.convolver",
            "/com/github/wwmm/pulseeffects/sinkinputs/convolver/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);
        update_string_key(&settings, "kernel-path", &self.kernel_path);
        update_key(&settings, "ir-width", self.ir_width);
    }
}