use gio::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct CrystalizerBand {
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
    band0: CrystalizerBand,
    band1: CrystalizerBand,
    band2: CrystalizerBand,
    band3: CrystalizerBand,
    band4: CrystalizerBand,
    band5: CrystalizerBand,
    band6: CrystalizerBand,
    band7: CrystalizerBand,
    band8: CrystalizerBand,
    band9: CrystalizerBand,
    band10: CrystalizerBand,
    band11: CrystalizerBand,
    band12: CrystalizerBand,
}

impl Default for Crystalizer {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.crystalizer",
            "/com/github/wwmm/pulseeffects/sinkinputs/crystalizer/",
        );

        Crystalizer {
            state: settings.get_boolean("state"),
            aggressive: settings.get_boolean("aggressive"),
            input_gain: settings.get_double("input-gain"),
            output_gain: settings.get_double("output-gain"),
            band0: CrystalizerBand {
                intensity: settings.get_double("intensity-band0"),
                mute: settings.get_boolean("mute-band0"),
                bypass: settings.get_boolean("bypass-band0"),
            },
            band1: CrystalizerBand {
                intensity: settings.get_double("intensity-band1"),
                mute: settings.get_boolean("mute-band1"),
                bypass: settings.get_boolean("bypass-band1"),
            },
            band2: CrystalizerBand {
                intensity: settings.get_double("intensity-band2"),
                mute: settings.get_boolean("mute-band2"),
                bypass: settings.get_boolean("bypass-band2"),
            },
            band3: CrystalizerBand {
                intensity: settings.get_double("intensity-band3"),
                mute: settings.get_boolean("mute-band3"),
                bypass: settings.get_boolean("bypass-band3"),
            },
            band4: CrystalizerBand {
                intensity: settings.get_double("intensity-band4"),
                mute: settings.get_boolean("mute-band4"),
                bypass: settings.get_boolean("bypass-band4"),
            },
            band5: CrystalizerBand {
                intensity: settings.get_double("intensity-band5"),
                mute: settings.get_boolean("mute-band5"),
                bypass: settings.get_boolean("bypass-band5"),
            },
            band6: CrystalizerBand {
                intensity: settings.get_double("intensity-band6"),
                mute: settings.get_boolean("mute-band6"),
                bypass: settings.get_boolean("bypass-band6"),
            },
            band7: CrystalizerBand {
                intensity: settings.get_double("intensity-band7"),
                mute: settings.get_boolean("mute-band7"),
                bypass: settings.get_boolean("bypass-band7"),
            },
            band8: CrystalizerBand {
                intensity: settings.get_double("intensity-band8"),
                mute: settings.get_boolean("mute-band8"),
                bypass: settings.get_boolean("bypass-band8"),
            },
            band9: CrystalizerBand {
                intensity: settings.get_double("intensity-band9"),
                mute: settings.get_boolean("mute-band9"),
                bypass: settings.get_boolean("bypass-band9"),
            },
            band10: CrystalizerBand {
                intensity: settings.get_double("intensity-band10"),
                mute: settings.get_boolean("mute-band10"),
                bypass: settings.get_boolean("bypass-band10"),
            },
            band11: CrystalizerBand {
                intensity: settings.get_double("intensity-band11"),
                mute: settings.get_boolean("mute-band11"),
                bypass: settings.get_boolean("bypass-band11"),
            },
            band12: CrystalizerBand{
                intensity: settings.get_double("intensity-band12"),
                mute: settings.get_boolean("mute-band12"),
                bypass: settings.get_boolean("bypass-band12"),
            },
        }
    }
}
