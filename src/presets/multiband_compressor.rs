use crate::presets::common::{update_key, update_string_key};
use gio::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Band {
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
            "com.github.wwmm.pulseeffects.multibandcompressor",
            "/com/github/wwmm/pulseeffects/sinkinputs/multibandcompressor/",
        );

        Output {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            freq0: settings.get_double("freq0"),
            freq1: settings.get_double("freq1"),
            freq2: settings.get_double("freq2"),
            mode: settings.get_string("mode").unwrap().to_string(),
            subband: Band {
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
            lowband: Band {
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
            midband: Band {
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
            highband: Band {
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

impl Output {
    pub fn apply(&self) {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.multibandcompressor",
            "/com/github/wwmm/pulseeffects/sinkinputs/multibandcompressor/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);
        update_key(&settings, "freq0", self.freq0);
        update_key(&settings, "freq1", self.freq1);
        update_key(&settings, "freq2", self.freq2);
        update_string_key(&settings, "mode", &self.mode);

        update_key(&settings, "threshold0", self.subband.threshold);
        update_key(&settings, "ratio0", self.subband.ratio);
        update_key(&settings, "attack0", self.subband.attack);
        update_key(&settings, "release0", self.subband.release);
        update_key(&settings, "makeup0", self.subband.makeup);
        update_key(&settings, "knee0", self.subband.knee);
        update_string_key(&settings, "detection0", &self.subband.detection);
        update_key(&settings, "bypass0", self.subband.bypass);
        update_key(&settings, "solo0", self.subband.solo);

        update_key(&settings, "threshold1", self.lowband.threshold);
        update_key(&settings, "ratio1", self.lowband.ratio);
        update_key(&settings, "attack1", self.lowband.attack);
        update_key(&settings, "release1", self.lowband.release);
        update_key(&settings, "makeup1", self.lowband.makeup);
        update_key(&settings, "knee1", self.lowband.knee);
        update_string_key(&settings, "detection1", &self.lowband.detection);
        update_key(&settings, "bypass1", self.lowband.bypass);
        update_key(&settings, "solo1", self.lowband.solo);

        update_key(&settings, "threshold2", self.midband.threshold);
        update_key(&settings, "ratio2", self.midband.ratio);
        update_key(&settings, "attack2", self.midband.attack);
        update_key(&settings, "release2", self.midband.release);
        update_key(&settings, "makeup2", self.midband.makeup);
        update_key(&settings, "knee2", self.midband.knee);
        update_string_key(&settings, "detection2", &self.midband.detection);
        update_key(&settings, "bypass2", self.midband.bypass);
        update_key(&settings, "solo2", self.midband.solo);

        update_key(&settings, "threshold3", self.highband.threshold);
        update_key(&settings, "ratio3", self.highband.ratio);
        update_key(&settings, "attack3", self.highband.attack);
        update_key(&settings, "release3", self.highband.release);
        update_key(&settings, "makeup3", self.highband.makeup);
        update_key(&settings, "knee3", self.highband.knee);
        update_string_key(&settings, "detection3", &self.highband.detection);
        update_key(&settings, "bypass3", self.highband.bypass);
        update_key(&settings, "solo3", self.highband.solo);
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
            "com.github.wwmm.pulseeffects.multibandcompressor",
            "/com/github/wwmm/pulseeffects/sourceoutputs/multibandcompressor/",
        );

        Input {
            state: settings.get_boolean("state"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            freq0: settings.get_double("freq0"),
            freq1: settings.get_double("freq1"),
            freq2: settings.get_double("freq2"),
            mode: settings.get_string("mode").unwrap().to_string(),
            subband: Band {
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
            lowband: Band {
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
            midband: Band {
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
            highband: Band {
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

impl Input {
    pub fn apply(&self) {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.multibandcompressor",
            "/com/github/wwmm/pulseeffects/sourceoutputs/multibandcompressor/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);
        update_key(&settings, "freq0", self.freq0);
        update_key(&settings, "freq1", self.freq1);
        update_key(&settings, "freq2", self.freq2);
        update_string_key(&settings, "mode", &self.mode);

        update_key(&settings, "threshold0", self.subband.threshold);
        update_key(&settings, "ratio0", self.subband.ratio);
        update_key(&settings, "attack0", self.subband.attack);
        update_key(&settings, "release0", self.subband.release);
        update_key(&settings, "makeup0", self.subband.makeup);
        update_key(&settings, "knee0", self.subband.knee);
        update_string_key(&settings, "detection0", &self.subband.detection);
        update_key(&settings, "bypass0", self.subband.bypass);
        update_key(&settings, "solo0", self.subband.solo);

        update_key(&settings, "threshold1", self.lowband.threshold);
        update_key(&settings, "ratio1", self.lowband.ratio);
        update_key(&settings, "attack1", self.lowband.attack);
        update_key(&settings, "release1", self.lowband.release);
        update_key(&settings, "makeup1", self.lowband.makeup);
        update_key(&settings, "knee1", self.lowband.knee);
        update_string_key(&settings, "detection1", &self.lowband.detection);
        update_key(&settings, "bypass1", self.lowband.bypass);
        update_key(&settings, "solo1", self.lowband.solo);

        update_key(&settings, "threshold2", self.midband.threshold);
        update_key(&settings, "ratio2", self.midband.ratio);
        update_key(&settings, "attack2", self.midband.attack);
        update_key(&settings, "release2", self.midband.release);
        update_key(&settings, "makeup2", self.midband.makeup);
        update_key(&settings, "knee2", self.midband.knee);
        update_string_key(&settings, "detection2", &self.midband.detection);
        update_key(&settings, "bypass2", self.midband.bypass);
        update_key(&settings, "solo2", self.midband.solo);

        update_key(&settings, "threshold3", self.highband.threshold);
        update_key(&settings, "ratio3", self.highband.ratio);
        update_key(&settings, "attack3", self.highband.attack);
        update_key(&settings, "release3", self.highband.release);
        update_key(&settings, "makeup3", self.highband.makeup);
        update_key(&settings, "knee3", self.highband.knee);
        update_string_key(&settings, "detection3", &self.highband.detection);
        update_key(&settings, "bypass3", self.highband.bypass);
        update_key(&settings, "solo3", self.highband.solo);
    }
}