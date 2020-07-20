use gio::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Output {
    state: bool,
    mode: String,
    num_bands: i32,
    input_gain: f64,
    output_gain: f64,
    split_channels: bool,
    left: Left,
}

impl Default for Output {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.equalizer",
            "/com/github/wwmm/pulseeffects/sinkinputs/equalizer/",
        );

        Output {
            state: settings.get_boolean("state"),
            mode: settings.get_string("mode").unwrap().to_string(),
            num_bands: settings.get_int("num-bands"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            split_channels: settings.get_boolean("split-channels"),
        }
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Band {
    style: String,
    mode: String,
    slope: String,
    solo: bool,
    mute: bool,
    gain: f64,
    frequency: f64,
    q: f64,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
struct Left {
    band0: Band,
    band1: Band,
    band2: Band,
    band3: Band,
    band4: Band,
    band5: Band,
    band6: Band,
    band7: Band,
    band8: Band,
    band9: Band,
    band10: Band,
    band11: Band,
    band12: Band,
    band13: Band,
    band14: Band,
    band15: Band,
    band16: Band,
    band17: Band,
    band18: Band,
    band19: Band,
    band20: Band,
    band21: Band,
    band22: Band,
    band23: Band,
    band24: Band,
    band25: Band,
    band26: Band,
    band27: Band,
    band28: Band,
    band29: Band,
    band30: Band,
    band31: Band,
}

impl Default for Left {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.equalizer",
            "/com/github/wwmm/pulseeffects/sinkinputs/equalizer/leftchannel",
        );

        Left {
            band0: Band{
                style: settings.get_string("band0-type").unwrap().to_string(),
                mode: settings.get_string("band0-mode").unwrap().to_string(),
                slope: settings.get_string("band0-slope").unwrap().to_string(),
                solo: settings.get_boolean("band0-solo"),
                mute: settings.get_boolean("band0-mute"),
                gain: settings.get_double("band0-gain"),
                frequency: settings.get_double("band0-frequency"),
                q: settings.get_double("band0-q"),
            },
            band1: Band{
                style: settings.get_string("band1-type").unwrap().to_string(),
                mode: settings.get_string("band1-mode").unwrap().to_string(),
                slope: settings.get_string("band1-slope").unwrap().to_string(),
                solo: settings.get_boolean("band1-solo"),
                mute: settings.get_boolean("band1-mute"),
                gain: settings.get_double("band1-gain"),
                frequency: settings.get_double("band1-frequency"),
                q: settings.get_double("band1-q"),
            },
            band2: Band{
                style: settings.get_string("band2-type").unwrap().to_string(),
                mode: settings.get_string("band2-mode").unwrap().to_string(),
                slope: settings.get_string("band2-slope").unwrap().to_string(),
                solo: settings.get_boolean("band2-solo"),
                mute: settings.get_boolean("band2-mute"),
                gain: settings.get_double("band2-gain"),
                frequency: settings.get_double("band2-frequency"),
                q: settings.get_double("band2-q"),
            },
            band3: Band{
                style: settings.get_string("band3-type").unwrap().to_string(),
                mode: settings.get_string("band3-mode").unwrap().to_string(),
                slope: settings.get_string("band3-slope").unwrap().to_string(),
                solo: settings.get_boolean("band3-solo"),
                mute: settings.get_boolean("band3-mute"),
                gain: settings.get_double("band3-gain"),
                frequency: settings.get_double("band3-frequency"),
                q: settings.get_double("band3-q"),
            },
            band4: Band{
                style: settings.get_string("band4-type").unwrap().to_string(),
                mode: settings.get_string("band4-mode").unwrap().to_string(),
                slope: settings.get_string("band4-slope").unwrap().to_string(),
                solo: settings.get_boolean("band4-solo"),
                mute: settings.get_boolean("band4-mute"),
                gain: settings.get_double("band4-gain"),
                frequency: settings.get_double("band4-frequency"),
                q: settings.get_double("band4-q"),
            },
            band5: Band{
                style: settings.get_string("band5-type").unwrap().to_string(),
                mode: settings.get_string("band5-mode").unwrap().to_string(),
                slope: settings.get_string("band5-slope").unwrap().to_string(),
                solo: settings.get_boolean("band5-solo"),
                mute: settings.get_boolean("band5-mute"),
                gain: settings.get_double("band5-gain"),
                frequency: settings.get_double("band5-frequency"),
                q: settings.get_double("band5-q"),
            },
        }
    }
}