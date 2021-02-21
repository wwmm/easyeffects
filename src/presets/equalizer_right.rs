use crate::presets::equalizer_band::Band;
use serde::{Deserialize, Serialize};

use gio::prelude::*;

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct RightOutput {
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
        let settings = gio::Settings::with_path(
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

impl RightOutput {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.equalizer.channel",
            "/com/github/wwmm/pulseeffects/sinkinputs/equalizer/rightchannel/",
        );

        update_band!(settings, self.band0, "0");
        update_band!(settings, self.band1, "1");
        update_band!(settings, self.band2, "2");
        update_band!(settings, self.band3, "3");
        update_band!(settings, self.band4, "4");
        update_band!(settings, self.band5, "5");
        update_band!(settings, self.band6, "6");
        update_band!(settings, self.band7, "7");
        update_band!(settings, self.band8, "8");
        update_band!(settings, self.band9, "9");
        update_band!(settings, self.band10, "10");
        update_band!(settings, self.band11, "11");
        update_band!(settings, self.band12, "12");
        update_band!(settings, self.band13, "13");
        update_band!(settings, self.band14, "14");
        update_band!(settings, self.band15, "15");
        update_band!(settings, self.band16, "16");
        update_band!(settings, self.band17, "17");
        update_band!(settings, self.band18, "18");
        update_band!(settings, self.band19, "19");
        update_band!(settings, self.band20, "20");
        update_band!(settings, self.band21, "21");
        update_band!(settings, self.band22, "22");
        update_band!(settings, self.band23, "23");
        update_band!(settings, self.band24, "24");
        update_band!(settings, self.band25, "25");
        update_band!(settings, self.band26, "26");
        update_band!(settings, self.band27, "27");
        update_band!(settings, self.band28, "28");
        update_band!(settings, self.band29, "29");
        update_band!(settings, self.band30, "30");
        update_band!(settings, self.band31, "31");
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct RightInput {
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
        let settings = gio::Settings::with_path(
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

impl RightInput {
    pub fn apply(&self) {
        let settings = gio::Settings::with_path(
            "com.github.wwmm.pulseeffects.equalizer.channel",
            "/com/github/wwmm/pulseeffects/sourceoutputs/equalizer/rightchannel/",
        );

        update_band!(settings, self.band0, "0");
        update_band!(settings, self.band1, "1");
        update_band!(settings, self.band2, "2");
        update_band!(settings, self.band3, "3");
        update_band!(settings, self.band4, "4");
        update_band!(settings, self.band5, "5");
        update_band!(settings, self.band6, "6");
        update_band!(settings, self.band7, "7");
        update_band!(settings, self.band8, "8");
        update_band!(settings, self.band9, "9");
        update_band!(settings, self.band10, "10");
        update_band!(settings, self.band11, "11");
        update_band!(settings, self.band12, "12");
        update_band!(settings, self.band13, "13");
        update_band!(settings, self.band14, "14");
        update_band!(settings, self.band15, "15");
        update_band!(settings, self.band16, "16");
        update_band!(settings, self.band17, "17");
        update_band!(settings, self.band18, "18");
        update_band!(settings, self.band19, "19");
        update_band!(settings, self.band20, "20");
        update_band!(settings, self.band21, "21");
        update_band!(settings, self.band22, "22");
        update_band!(settings, self.band23, "23");
        update_band!(settings, self.band24, "24");
        update_band!(settings, self.band25, "25");
        update_band!(settings, self.band26, "26");
        update_band!(settings, self.band27, "27");
        update_band!(settings, self.band28, "28");
        update_band!(settings, self.band29, "29");
        update_band!(settings, self.band30, "30");
        update_band!(settings, self.band31, "31");
    }
}
