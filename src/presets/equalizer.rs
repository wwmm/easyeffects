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
    left: LeftOutput,
    right: RightOutput,
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
            left: LeftOutput::default(),
            right: RightOutput::default(),
        }
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Input {
    state: bool,
    mode: String,
    num_bands: i32,
    input_gain: f64,
    output_gain: f64,
    split_channels: bool,
    left: LeftInput,
    right: RightInput,
}

impl Default for Input {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.equalizer",
            "/com/github/wwmm/pulseeffects/sourceoutputs/equalizer/",
        );

        Input {
            state: settings.get_boolean("state"),
            mode: settings.get_string("mode").unwrap().to_string(),
            num_bands: settings.get_int("num-bands"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            split_channels: settings.get_boolean("split-channels"),
            left: LeftInput::default(),
            right: RightInput::default(),
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

macro_rules! band {
    ($settings:expr, $index:expr) => {
        Band {
            style: $settings
                .get_string(concat!("band", $index, "-type"))
                .unwrap()
                .to_string(),
            mode: $settings
                .get_string(concat!("band", $index, "-mode"))
                .unwrap()
                .to_string(),
            slope: $settings
                .get_string(concat!("band", $index, "-slope"))
                .unwrap()
                .to_string(),
            solo: $settings.get_boolean(concat!("band", $index, "-solo")),
            mute: $settings.get_boolean(concat!("band", $index, "-mute")),
            gain: $settings.get_double(concat!("band", $index, "-gain")),
            frequency: $settings.get_double(concat!("band", $index, "-frequency")),
            q: $settings.get_double(concat!("band", $index, "-q")),
        }
    };
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
struct LeftOutput {
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

impl Default for LeftOutput {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.equalizer.channel",
            "/com/github/wwmm/pulseeffects/sinkinputs/equalizer/leftchannel/",
        );

        LeftOutput {
            band0: band!(settings, "0"),
            band1: band!(settings, "1"),
            band2: band!(settings, "2"),
            band3: band!(settings, "3"),
            band4: band!(settings, "4"),
            band5: band!(settings, "5"),
            band6: band!(settings, "6"),
            band7: band!(settings, "7"),
            band8: band!(settings, "8"),
            band9: band!(settings, "9"),
            band10: band!(settings, "10"),
            band11: band!(settings, "11"),
            band12: band!(settings, "12"),
            band13: band!(settings, "13"),
            band14: band!(settings, "14"),
            band15: band!(settings, "15"),
            band16: band!(settings, "16"),
            band17: band!(settings, "17"),
            band18: band!(settings, "18"),
            band19: band!(settings, "19"),
            band20: band!(settings, "20"),
            band21: band!(settings, "21"),
            band22: band!(settings, "22"),
            band23: band!(settings, "23"),
            band24: band!(settings, "24"),
            band25: band!(settings, "25"),
            band26: band!(settings, "26"),
            band27: band!(settings, "27"),
            band28: band!(settings, "28"),
            band29: band!(settings, "29"),
            band30: band!(settings, "30"),
            band31: band!(settings, "31"),
        }
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
struct LeftInput {
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

impl Default for LeftInput {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.equalizer.channel",
            "/com/github/wwmm/pulseeffects/sourceoutputs/equalizer/leftchannel/",
        );

        LeftInput {
            band0: band!(settings, "0"),
            band1: band!(settings, "1"),
            band2: band!(settings, "2"),
            band3: band!(settings, "3"),
            band4: band!(settings, "4"),
            band5: band!(settings, "5"),
            band6: band!(settings, "6"),
            band7: band!(settings, "7"),
            band8: band!(settings, "8"),
            band9: band!(settings, "9"),
            band10: band!(settings, "10"),
            band11: band!(settings, "11"),
            band12: band!(settings, "12"),
            band13: band!(settings, "13"),
            band14: band!(settings, "14"),
            band15: band!(settings, "15"),
            band16: band!(settings, "16"),
            band17: band!(settings, "17"),
            band18: band!(settings, "18"),
            band19: band!(settings, "19"),
            band20: band!(settings, "20"),
            band21: band!(settings, "21"),
            band22: band!(settings, "22"),
            band23: band!(settings, "23"),
            band24: band!(settings, "24"),
            band25: band!(settings, "25"),
            band26: band!(settings, "26"),
            band27: band!(settings, "27"),
            band28: band!(settings, "28"),
            band29: band!(settings, "29"),
            band30: band!(settings, "30"),
            band31: band!(settings, "31"),
        }
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
struct RightOutput {
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

impl Default for RightOutput {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.equalizer.channel",
            "/com/github/wwmm/pulseeffects/sinkinputs/equalizer/rightchannel/",
        );

        RightOutput {
            band0: band!(settings, "0"),
            band1: band!(settings, "1"),
            band2: band!(settings, "2"),
            band3: band!(settings, "3"),
            band4: band!(settings, "4"),
            band5: band!(settings, "5"),
            band6: band!(settings, "6"),
            band7: band!(settings, "7"),
            band8: band!(settings, "8"),
            band9: band!(settings, "9"),
            band10: band!(settings, "10"),
            band11: band!(settings, "11"),
            band12: band!(settings, "12"),
            band13: band!(settings, "13"),
            band14: band!(settings, "14"),
            band15: band!(settings, "15"),
            band16: band!(settings, "16"),
            band17: band!(settings, "17"),
            band18: band!(settings, "18"),
            band19: band!(settings, "19"),
            band20: band!(settings, "20"),
            band21: band!(settings, "21"),
            band22: band!(settings, "22"),
            band23: band!(settings, "23"),
            band24: band!(settings, "24"),
            band25: band!(settings, "25"),
            band26: band!(settings, "26"),
            band27: band!(settings, "27"),
            band28: band!(settings, "28"),
            band29: band!(settings, "29"),
            band30: band!(settings, "30"),
            band31: band!(settings, "31"),
        }
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
struct RightInput {
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

impl Default for RightInput {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.equalizer.channel",
            "/com/github/wwmm/pulseeffects/sourceoutputs/equalizer/rightchannel/",
        );

        RightInput {
            band0: band!(settings, "0"),
            band1: band!(settings, "1"),
            band2: band!(settings, "2"),
            band3: band!(settings, "3"),
            band4: band!(settings, "4"),
            band5: band!(settings, "5"),
            band6: band!(settings, "6"),
            band7: band!(settings, "7"),
            band8: band!(settings, "8"),
            band9: band!(settings, "9"),
            band10: band!(settings, "10"),
            band11: band!(settings, "11"),
            band12: band!(settings, "12"),
            band13: band!(settings, "13"),
            band14: band!(settings, "14"),
            band15: band!(settings, "15"),
            band16: band!(settings, "16"),
            band17: band!(settings, "17"),
            band18: band!(settings, "18"),
            band19: band!(settings, "19"),
            band20: band!(settings, "20"),
            band21: band!(settings, "21"),
            band22: band!(settings, "22"),
            band23: band!(settings, "23"),
            band24: band!(settings, "24"),
            band25: band!(settings, "25"),
            band26: band!(settings, "26"),
            band27: band!(settings, "27"),
            band28: band!(settings, "28"),
            band29: band!(settings, "29"),
            band30: band!(settings, "30"),
            band31: band!(settings, "31"),
        }
    }
}
