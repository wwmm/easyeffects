use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
pub struct OutputRoot {
    spectrum: Spectrum,
    output: Output,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct InputRoot {
    spectrum: Spectrum,
    output: Output,
}

#[derive(Serialize, Deserialize, Debug)]
struct Output {
    plugins_order: Vec<String>,
    bass_enhancer: BassEnhancer
}

#[derive(Serialize, Deserialize)]
struct Input {}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Spectrum {
    show: bool,
    n_points: i32,
    height: i32,
    use_custom_color: bool,
    fill: bool,
    show_bar_border: bool,
    scale: f32,
    exponent: f32,
    sampling_freq: i32,
    line_width: i32,
    style: String,
    color: [f32; 4],
    gradient_color: [f32; 4],
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct BassEnhancer {
    state: bool,
    input_gain: f32,
    output_gain: f32,
    amount: f32,
    harmonics: f32,
    scope: f32,
    floor: f32,
    blend: f32,
    floor_active: bool,
    listen: bool
}
