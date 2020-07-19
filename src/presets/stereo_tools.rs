use gio::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct StereoTools {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    balance_in: f64,
    balance_out: f64,
    softclip: bool,
    mutel: bool,
    muter: bool,
    phasel: bool,
    phaser: bool,
    mode: String,
    side_level: f64,
    side_balance: f64,
    middle_level: f64,
    middle_panorama: f64,
    stereo_base: f64,
    delay: f64,
    sc_level: f64,
    stereo_phase: f64,
}

impl Default for StereoTools {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.stereotools",
            "/com/github/wwmm/pulseeffects/sinkinputs/stereotools/",
        );

        StereoTools {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            balance_in: settings.get_double("balance-in"),
            balance_out: settings.get_double("balance-out"),
            softclip: settings.get_boolean("softclip"),
            mutel: settings.get_boolean("mutel"),
            muter: settings.get_boolean("muter"),
            phasel: settings.get_boolean("phasel"),
            phaser: settings.get_boolean("phaser"),
            mode: settings.get_string("mode").unwrap().to_string(),
            side_level: settings.get_double("slev"),
            side_balance: settings.get_double("sbal"),
            middle_level: settings.get_double("mlev"),
            middle_panorama: settings.get_double("mpan"),
            stereo_base: settings.get_double("stereo-base"),
            delay: settings.get_double("delay"),
            sc_level: settings.get_double("sc-level"),
            stereo_phase: settings.get_double("stereo-phase"),
        }
    }
}