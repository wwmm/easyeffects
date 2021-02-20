use crate::presets::common::{update_key, update_string_key};
use serde::{Deserialize, Serialize};

use gtk::gio;
use gio::prelude::*;

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
        let settings = gio::Settings::with_path(
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

impl StereoTools {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.stereotools",
            "/com/github/wwmm/pulseeffects/sinkinputs/stereotools/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);
        update_key(&settings, "balance-in", self.balance_in);
        update_key(&settings, "balance-out", self.balance_out);
        update_key(&settings, "softclip", self.softclip);
        update_key(&settings, "mutel", self.mutel);
        update_key(&settings, "muter", self.muter);
        update_key(&settings, "phasel", self.phasel);
        update_key(&settings, "phaser", self.phaser);
        update_string_key(&settings, "mode", &self.mode);
        update_key(&settings, "slev", self.side_level);
        update_key(&settings, "sbal", self.side_balance);
        update_key(&settings, "mlev", self.middle_level);
        update_key(&settings, "mpan", self.middle_panorama);
        update_key(&settings, "stereo-base", self.stereo_base);
        update_key(&settings, "delay", self.delay);
        update_key(&settings, "sc-level", self.sc_level);
        update_key(&settings, "stereo-phase", self.stereo_phase);
    }
}
