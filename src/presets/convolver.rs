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