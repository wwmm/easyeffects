use gio::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Band {
    reduction: f64,
    threshold: f64,
    ratio: f64,
    attack: f64,
    release: f64,
    makeup: f64,
    knee: f64,
    detection: String,
    bypass: bool,
    solo: bool,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Output {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    freq0: f64,
    freq1: f64,
    freq2: f64,
    mode: String,
    subband: Band,
    lowband: Band,
    midband: Band,
    highband: Band,
}

impl Default for Output {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.multibandgate",
            "/com/github/wwmm/pulseeffects/sinkinputs/multibandgate/",
        );

        Output {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            freq0: settings.get_double("freq0"),
            freq1: settings.get_double("freq1"),
            freq2: settings.get_double("freq2"),
            mode: settings.get_string("mode").unwrap().to_string(),
            subband: Band{
                reduction: settings.get_double("range0"),
                threshold: settings.get_double("threshold0"),
                ratio: settings.get_double("ratio0"),
                attack: settings.get_double("attack0"),
                release: settings.get_double("release0"),
                makeup: settings.get_double("makeup0"),
                knee: settings.get_double("knee0"),
                detection: settings.get_string("detection0").unwrap().to_string(),
                bypass: settings.get_boolean("bypass0"),
                solo: settings.get_boolean("solo0"),
            },
            lowband: Band{
                reduction: settings.get_double("range1"),
                threshold: settings.get_double("threshold1"),
                ratio: settings.get_double("ratio1"),
                attack: settings.get_double("attack1"),
                release: settings.get_double("release1"),
                makeup: settings.get_double("makeup1"),
                knee: settings.get_double("knee1"),
                detection: settings.get_string("detection1").unwrap().to_string(),
                bypass: settings.get_boolean("bypass1"),
                solo: settings.get_boolean("solo1"),
            },
            midband: Band{
                reduction: settings.get_double("range2"),
                threshold: settings.get_double("threshold2"),
                ratio: settings.get_double("ratio2"),
                attack: settings.get_double("attack2"),
                release: settings.get_double("release2"),
                makeup: settings.get_double("makeup2"),
                knee: settings.get_double("knee2"),
                detection: settings.get_string("detection2").unwrap().to_string(),
                bypass: settings.get_boolean("bypass2"),
                solo: settings.get_boolean("solo2"),
            },
            highband: Band{
                reduction: settings.get_double("range3"),
                threshold: settings.get_double("threshold3"),
                ratio: settings.get_double("ratio3"),
                attack: settings.get_double("attack3"),
                release: settings.get_double("release3"),
                makeup: settings.get_double("makeup3"),
                knee: settings.get_double("knee3"),
                detection: settings.get_string("detection3").unwrap().to_string(),
                bypass: settings.get_boolean("bypass3"),
                solo: settings.get_boolean("solo3"),
            },
        }
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Input {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    freq0: f64,
    freq1: f64,
    freq2: f64,
    mode: String,
    subband: Band,
    lowband: Band,
    midband: Band,
    highband: Band,
}

impl Default for Input {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.multibandgate",
            "/com/github/wwmm/pulseeffects/sourceoutputs/multibandgate/",
        );

        Input {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            freq0: settings.get_double("freq0"),
            freq1: settings.get_double("freq1"),
            freq2: settings.get_double("freq2"),
            mode: settings.get_string("mode").unwrap().to_string(),
            subband: Band{
                reduction: settings.get_double("range0"),
                threshold: settings.get_double("threshold0"),
                ratio: settings.get_double("ratio0"),
                attack: settings.get_double("attack0"),
                release: settings.get_double("release0"),
                makeup: settings.get_double("makeup0"),
                knee: settings.get_double("knee0"),
                detection: settings.get_string("detection0").unwrap().to_string(),
                bypass: settings.get_boolean("bypass0"),
                solo: settings.get_boolean("solo0"),
            },
            lowband: Band{
                reduction: settings.get_double("range1"),
                threshold: settings.get_double("threshold1"),
                ratio: settings.get_double("ratio1"),
                attack: settings.get_double("attack1"),
                release: settings.get_double("release1"),
                makeup: settings.get_double("makeup1"),
                knee: settings.get_double("knee1"),
                detection: settings.get_string("detection1").unwrap().to_string(),
                bypass: settings.get_boolean("bypass1"),
                solo: settings.get_boolean("solo1"),
            },
            midband: Band{
                reduction: settings.get_double("range2"),
                threshold: settings.get_double("threshold2"),
                ratio: settings.get_double("ratio2"),
                attack: settings.get_double("attack2"),
                release: settings.get_double("release2"),
                makeup: settings.get_double("makeup2"),
                knee: settings.get_double("knee2"),
                detection: settings.get_string("detection2").unwrap().to_string(),
                bypass: settings.get_boolean("bypass2"),
                solo: settings.get_boolean("solo2"),
            },
            highband: Band{
                reduction: settings.get_double("range3"),
                threshold: settings.get_double("threshold3"),
                ratio: settings.get_double("ratio3"),
                attack: settings.get_double("attack3"),
                release: settings.get_double("release3"),
                makeup: settings.get_double("makeup3"),
                knee: settings.get_double("knee3"),
                detection: settings.get_string("detection3").unwrap().to_string(),
                bypass: settings.get_boolean("bypass3"),
                solo: settings.get_boolean("solo3"),
            },
        }
    }
}