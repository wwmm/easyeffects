use crate::presets::bass_enhancer::BassEnhancer;
use crate::presets::compressor;
use crate::presets::convolver::Convolver;
use crate::presets::crossfeed::Crossfeed;
use crate::presets::crystalizer::Crystalizer;
use crate::presets::deesser;
use crate::presets::exciter::Exciter;
use crate::presets::filter;
use crate::presets::gate;
use crate::presets::limiter;
use crate::presets::loudness::Loudness;
use crate::presets::maximizer;
use crate::presets::multiband_compressor;
use crate::presets::multiband_gate;
use crate::presets::pitch;
use crate::presets::reverb;
use crate::presets::spectrum::Spectrum;
use crate::presets::stereo_tools::StereoTools;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
pub struct OutputRoot {
    spectrum: Spectrum,
    output: Output,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct InputRoot {
    spectrum: Spectrum,
    input: Input,
}

#[derive(Serialize, Deserialize, Debug)]
struct Output {
    plugins_order: Vec<String>,
    bass_enhancer: BassEnhancer,
    compressor: compressor::Output,
    crossfeed: Crossfeed,
    deesser: deesser::Output,
    exciter: Exciter,
    filter: filter::Output,
    gate: gate::Output,
    limiter: limiter::Output,
    maximizer: maximizer::Output,
    pitch: pitch::Output,
    reverb: reverb::Output,
    multiband_compressor: multiband_compressor::Output,
    loudness: Loudness,
    multiband_gate: multiband_gate::Output,
    stereo_tools: StereoTools,
    convolver: Convolver,
    crystalizer: Crystalizer,
    autogain: AutoGain,
    delay: Delay,
}

#[derive(Serialize, Deserialize, Debug)]
struct Input {
    plugins_order: Vec<String>,
    compressor: compressor::Input,
    deesser: deesser::Input,
    filter: filter::Input,
    gate: gate::Input,
    limiter: limiter::Input,
    pitch: pitch::Input,
    reverb: reverb::Input,
    webrtc: WebRTC,
    multiband_compressor: multiband_compressor::Input,
    multiband_gate: multiband_gate::Input,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct AutoGain {
    state: bool,
    detect_silence: bool,
    use_geometric_mean: bool,
    input_gain: f64,
    output_gain: f64,
    target: f64,
    weight_m: i32,
    weight_s: i32,
    weight_i: i32,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Delay {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    time_l: f64,
    time_r: f64,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct WebRTC {
    state: bool,
    high_pass_filter: bool,
    echo_cancel: bool,
    echo_suppression_level: String,
    noise_suppression: bool,
    noise_suppression_level: String,
    gain_control: bool,
    extended_filter: bool,
    delay_agnostic: bool,
    target_level_dbfs: i32,
    compression_gain_db: i32,
    limiter: bool,
    gain_control_mode: String,
    voice_detection: bool,
    voice_detection_frame_size_ms: i32,
    voice_detection_likelihood: String,
}
