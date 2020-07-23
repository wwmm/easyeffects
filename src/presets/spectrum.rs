use gio::prelude::*;
use serde::{Deserialize, Serialize};
use crate::presets::common::update_key;

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case", default)]
pub struct Spectrum {
    show: bool,
    n_points: i32,
    height: i32,
    use_custom_color: bool,
    fill: bool,
    show_bar_border: bool,
    scale: f64,
    exponent: f64,
    sampling_freq: i32,
    line_width: f64,
    style: String,
    color: Vec<f32>,
    gradient_color: Vec<f32>,
}

impl Default for Spectrum {
    fn default() -> Self {
        let settings = gio::Settings::new("com.github.wwmm.pulseeffects.spectrum");

        // color vec

        let mut color_string = settings
            .get_value("color")
            .to_string()
            .replace("[", "")
            .replace("]", "")
            .replace(" ", "");

        let color_str_vec: Vec<&str> = color_string.split(",").collect();

        let mut color: Vec<f32> = Vec::new();

        for pixel in color_str_vec {
            color.push(pixel.parse().unwrap());
        }

        // gradient color vec

        color_string = settings
            .get_value("gradient-color")
            .to_string()
            .replace("[", "")
            .replace("]", "")
            .replace(" ", "");

        let color_str_vec: Vec<&str> = color_string.split(",").collect();

        let mut gradient_color: Vec<f32> = Vec::new();

        for pixel in color_str_vec {
            gradient_color.push(pixel.parse().unwrap());
        }

        Spectrum {
            show: settings.get_boolean("show"),
            n_points: settings.get_int("n-points"),
            height: settings.get_int("height"),
            use_custom_color: settings.get_boolean("use-custom-color"),
            fill: settings.get_boolean("fill"),
            show_bar_border: settings.get_boolean("show-bar-border"),
            scale: settings.get_double("scale"),
            exponent: settings.get_double("exponent"),
            sampling_freq: settings.get_int("sampling-freq"),
            line_width: settings.get_double("line-width"),
            style: settings.get_string("type").unwrap().to_string(),
            color: color,
            gradient_color: gradient_color,
        }
    }
}

impl Spectrum {
    pub fn apply(&self){
        let settings = gio::Settings::new("com.github.wwmm.pulseeffects.spectrum");

        update_key(&settings, "show", self.show);
        update_key(&settings, "n-points", self.n_points);
        update_key(&settings, "height", self.height);
    }
}