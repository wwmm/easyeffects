use crate::presets::common::update_key;
use serde::{Deserialize, Serialize};

use gio::prelude::*;

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Band {
    intensity: f64,
    mute: bool,
    bypass: bool,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Crystalizer {
    state: bool,
    aggressive: bool,
    input_gain: f64,
    output_gain: f64,
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
}

impl Default for Crystalizer {
    fn default() -> Self {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.crystalizer",
            "/com/github/wwmm/pulseeffects/sinkinputs/crystalizer/",
        );

        Crystalizer {
            state: settings.get_boolean("state"),
            aggressive: settings.get_boolean("aggressive"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            band0: Band {
                intensity: settings.get_double("intensity-band0"),
                mute: settings.get_boolean("mute-band0"),
                bypass: settings.get_boolean("bypass-band0"),
            },
            band1: Band {
                intensity: settings.get_double("intensity-band1"),
                mute: settings.get_boolean("mute-band1"),
                bypass: settings.get_boolean("bypass-band1"),
            },
            band2: Band {
                intensity: settings.get_double("intensity-band2"),
                mute: settings.get_boolean("mute-band2"),
                bypass: settings.get_boolean("bypass-band2"),
            },
            band3: Band {
                intensity: settings.get_double("intensity-band3"),
                mute: settings.get_boolean("mute-band3"),
                bypass: settings.get_boolean("bypass-band3"),
            },
            band4: Band {
                intensity: settings.get_double("intensity-band4"),
                mute: settings.get_boolean("mute-band4"),
                bypass: settings.get_boolean("bypass-band4"),
            },
            band5: Band {
                intensity: settings.get_double("intensity-band5"),
                mute: settings.get_boolean("mute-band5"),
                bypass: settings.get_boolean("bypass-band5"),
            },
            band6: Band {
                intensity: settings.get_double("intensity-band6"),
                mute: settings.get_boolean("mute-band6"),
                bypass: settings.get_boolean("bypass-band6"),
            },
            band7: Band {
                intensity: settings.get_double("intensity-band7"),
                mute: settings.get_boolean("mute-band7"),
                bypass: settings.get_boolean("bypass-band7"),
            },
            band8: Band {
                intensity: settings.get_double("intensity-band8"),
                mute: settings.get_boolean("mute-band8"),
                bypass: settings.get_boolean("bypass-band8"),
            },
            band9: Band {
                intensity: settings.get_double("intensity-band9"),
                mute: settings.get_boolean("mute-band9"),
                bypass: settings.get_boolean("bypass-band9"),
            },
            band10: Band {
                intensity: settings.get_double("intensity-band10"),
                mute: settings.get_boolean("mute-band10"),
                bypass: settings.get_boolean("bypass-band10"),
            },
            band11: Band {
                intensity: settings.get_double("intensity-band11"),
                mute: settings.get_boolean("mute-band11"),
                bypass: settings.get_boolean("bypass-band11"),
            },
            band12: Band {
                intensity: settings.get_double("intensity-band12"),
                mute: settings.get_boolean("mute-band12"),
                bypass: settings.get_boolean("bypass-band12"),
            },
        }
    }
}

impl Crystalizer {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.crystalizer",
            "/com/github/wwmm/pulseeffects/sinkinputs/crystalizer/",
        );

        update_key(&settings, "state", self.state);
        update_key(&settings, "aggressive", self.aggressive);
        update_key(&settings, "input-gain", self.input_gain);
        update_key(&settings, "output-gain", self.output_gain);

        update_key(&settings, "intensity-band0", self.band0.intensity);
        update_key(&settings, "mute-band0", self.band0.mute);
        update_key(&settings, "bypass-band0", self.band0.bypass);

        update_key(&settings, "intensity-band1", self.band1.intensity);
        update_key(&settings, "mute-band1", self.band1.mute);
        update_key(&settings, "bypass-band1", self.band1.bypass);

        update_key(&settings, "intensity-band2", self.band2.intensity);
        update_key(&settings, "mute-band2", self.band2.mute);
        update_key(&settings, "bypass-band2", self.band2.bypass);

        update_key(&settings, "intensity-band3", self.band3.intensity);
        update_key(&settings, "mute-band3", self.band3.mute);
        update_key(&settings, "bypass-band3", self.band3.bypass);

        update_key(&settings, "intensity-band4", self.band4.intensity);
        update_key(&settings, "mute-band4", self.band4.mute);
        update_key(&settings, "bypass-band4", self.band4.bypass);

        update_key(&settings, "intensity-band5", self.band5.intensity);
        update_key(&settings, "mute-band5", self.band5.mute);
        update_key(&settings, "bypass-band5", self.band5.bypass);

        update_key(&settings, "intensity-band6", self.band6.intensity);
        update_key(&settings, "mute-band6", self.band6.mute);
        update_key(&settings, "bypass-band6", self.band6.bypass);

        update_key(&settings, "intensity-band7", self.band7.intensity);
        update_key(&settings, "mute-band7", self.band7.mute);
        update_key(&settings, "bypass-band7", self.band7.bypass);
        update_key(&settings, "intensity-band8", self.band8.intensity);
        update_key(&settings, "mute-band8", self.band8.mute);
        update_key(&settings, "bypass-band8", self.band8.bypass);

        update_key(&settings, "intensity-band9", self.band9.intensity);
        update_key(&settings, "mute-band9", self.band9.mute);
        update_key(&settings, "bypass-band9", self.band9.bypass);

        update_key(&settings, "intensity-band10", self.band10.intensity);
        update_key(&settings, "mute-band10", self.band10.mute);
        update_key(&settings, "bypass-band10", self.band10.bypass);

        update_key(&settings, "intensity-band11", self.band11.intensity);
        update_key(&settings, "mute-band11", self.band11.mute);
        update_key(&settings, "bypass-band11", self.band11.bypass);

        update_key(&settings, "intensity-band12", self.band12.intensity);
        update_key(&settings, "mute-band12", self.band12.mute);
        update_key(&settings, "bypass-band12", self.band12.bypass);
    }
}
