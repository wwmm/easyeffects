use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
pub struct Band {
    pub style: String,
    pub mode: String,
    pub slope: String,
    pub solo: bool,
    pub mute: bool,
    pub gain: f64,
    pub frequency: f64,
    pub q: f64,
}

#[macro_export]
macro_rules! update_band {
    ($settings:expr, $band:expr, $index:expr) => {{
        crate::presets::common::update_string_key(&$settings, concat!("band", $index, "-type"), &$band.style);
        crate::presets::common::update_string_key(&$settings, concat!("band", $index, "-mode"), &$band.mode);
        crate::presets::common::update_string_key(&$settings, concat!("band", $index, "-slope"), &$band.slope);
        crate::presets::common::update_key(&$settings, concat!("band", $index, "-solo"), $band.solo);
        crate::presets::common::update_key(&$settings, concat!("band", $index, "-mute"), $band.mute);
        crate::presets::common::update_key(&$settings, concat!("band", $index, "-gain"), $band.gain);
        crate::presets::common::update_key(&$settings, concat!("band", $index, "-frequency"), $band.frequency);
        crate::presets::common::update_key(&$settings, concat!("band", $index, "-q"), $band.q);
    }};
}

#[macro_export]
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
