use gio::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Output {
    state: bool,
    detection: String,
    mode: String,
    threshold: f64,
    ratio: f64,
    laxity: i32,
    makeup: f64,
    f1_freq: f64,
    f2_freq: f64,
    f1_level: f64,
    f2_level: f64,
    f2_q: f64,
    sc_listen: bool,
}

impl Default for Output {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.deesser",
            "/com/github/wwmm/pulseeffects/sinkinputs/deesser/",
        );

        Output {
            state: settings.get_boolean("state"),
            detection: settings.get_string("detection").unwrap().to_string(),
            mode: settings.get_string("mode").unwrap().to_string(),
            threshold: settings.get_double("threshold"),
            ratio: settings.get_double("ratio"),
            laxity: settings.get_int("laxity"),
            makeup: settings.get_double("makeup"),
            f1_freq: settings.get_double("f1-freq"),
            f2_freq: settings.get_double("f2-freq"),
            f1_level: settings.get_double("f1-level"),
            f2_level: settings.get_double("f2-level"),
            f2_q: settings.get_double("f2-q"),
            sc_listen: settings.get_boolean("sc-listen"),
        }
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Input {
    state: bool,
    detection: String,
    mode: String,
    threshold: f64,
    ratio: f64,
    laxity: i32,
    makeup: f64,
    f1_freq: f64,
    f2_freq: f64,
    f1_level: f64,
    f2_level: f64,
    f2_q: f64,
    sc_listen: bool,
}

impl Default for Input {
    fn default() -> Self {
        let settings = gio::Settings::new_with_path(
            "com.github.wwmm.pulseeffects.deesser",
            "/com/github/wwmm/pulseeffects/sourceoutputs/deesser/",
        );

        Input {
            state: settings.get_boolean("state"),
            detection: settings.get_string("detection").unwrap().to_string(),
            mode: settings.get_string("mode").unwrap().to_string(),
            threshold: settings.get_double("threshold"),
            ratio: settings.get_double("ratio"),
            laxity: settings.get_int("laxity"),
            makeup: settings.get_double("makeup"),
            f1_freq: settings.get_double("f1-freq"),
            f2_freq: settings.get_double("f2-freq"),
            f1_level: settings.get_double("f1-level"),
            f2_level: settings.get_double("f2-level"),
            f2_q: settings.get_double("f2-q"),
            sc_listen: settings.get_boolean("sc-listen"),
        }
    }
}
