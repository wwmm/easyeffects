use crate::presets::bass_enhancer::BassEnhancer;
use crate::presets::compressor;
use crate::presets::crossfeed::Crossfeed;
use crate::presets::deesser;
use crate::presets::exciter::Exciter;
use crate::presets::spectrum::Spectrum;
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
    filter: Filter,
    gate: Gate,
    limiter: Limiter,
    maximizer: Maximizer,
    pitch: Pitch,
    reverb: Reverb,
    multiband_compressor: MultibandCompressor,
    loudness: Loudness,
    multiband_gate: MultibandGate,
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
    filter: Filter,
    gate: Gate,
    limiter: Limiter,
    pitch: Pitch,
    reverb: Reverb,
    webrtc: WebRTC,
    multiband_compressor: MultibandCompressor,
    multiband_gate: MultibandGate,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Filter {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    frequency: f64,
    resonance: f64,
    mode: String,
    inertia: f64,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Gate {
    state: bool,
    detection: String,
    stereo_link: String,
    range: f64,
    attack: f64,
    release: f64,
    threshold: f64,
    ratio: f64,
    knee: f64,
    makeup: f64,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Limiter {
    state: bool,
    input_gain: f64,
    limit: f64,
    lookahead: f64,
    release: f64,
    asc: bool,
    asc_level: f64,
    oversampling: i32,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Maximizer {
    state: bool,
    release: f64,
    ceiling: f64,
    threshold: f64,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Pitch {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    cents: f64,
    semitones: i32,
    octaves: i32,
    crispness: i32,
    formant_preserving: bool,
    faster: bool,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Reverb {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    room_size: String,
    decay_time: f64,
    hf_damp: f64,
    diffusion: f64,
    amount: f64,
    dry: f64,
    predelay: f64,
    bass_cut: f64,
    treble_cut: f64,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct MultibandCompressor {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    freq0: f64,
    freq1: f64,
    freq2: f64,
    mode: String,
    subband: MultibandCompressorBand,
    lowband: MultibandCompressorBand,
    midband: MultibandCompressorBand,
    highband: MultibandCompressorBand,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct MultibandCompressorBand {
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
#[serde(rename_all = "kebab-case")]
struct Loudness {
    state: bool,
    loudness: f64,
    output: f64,
    link: f64,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct MultibandGate {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    freq0: f64,
    freq1: f64,
    freq2: f64,
    mode: String,
    subband: MultibandGateBand,
    lowband: MultibandGateBand,
    midband: MultibandGateBand,
    highband: MultibandGateBand,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct MultibandGateBand {
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
#[serde(rename_all = "kebab-case")]
struct StereoTools {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    balance_in: f64,
    balance_out: f64,
    softclip: bool,
    mutel: bool,
    muter: bool,
    phasel: bool,
    phaser: bool,
    mode: String,
    side_level: f64,
    side_balance: f64,
    middle_level: f64,
    middle_panorama: f64,
    stereo_base: f64,
    delay: f64,
    sc_level: f64,
    stereo_phase: f64,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Convolver {
    state: bool,
    input_gain: f64,
    output_gain: f64,
    kernel_path: String,
    ir_width: i32,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Crystalizer {
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

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct CrystalizerBand {
    intensity: f64,
    mute: bool,
    bypass: bool,
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
