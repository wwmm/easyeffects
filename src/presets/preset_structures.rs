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
    compressor: Compressor,
    crossfeed: Crossfeed,
    deesser: Deesser,
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
    compressor: Compressor,
    deesser: Deesser,
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
    listen: bool,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Compressor {
    state: bool,
    input_gain: f32,
    output_gain: f32,
    mode: String,
    attack: f32,
    release: f32,
    threshold: f32,
    ratio: f32,
    knee: f32,
    makeup: f32,
    sidechain: CompressorSidechain,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct CompressorSidechain {
    listen: bool,
    style: String,
    mode: String,
    source: String,
    preamp: f32,
    reactivity: f32,
    lookahead: f32,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Crossfeed {
    state: bool,
    fcut: i32,
    feed: f32,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Deesser {
    state: bool,
    detection: String,
    mode: String,
    threshold: f32,
    ratio: f32,
    laxity: i32,
    makeup: f32,
    f1_freq: f32,
    f2_freq: f32,
    f1_level: f32,
    f2_level: f32,
    f2_q: f32,
    sc_listen: bool,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Exciter {
    state: bool,
    input_gain: f32,
    output_gain: f32,
    amount: f32,
    harmonics: f32,
    scope: f32,
    ceil: f32,
    blend: f32,
    ceil_active: bool,
    listen: bool,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Filter {
    state: bool,
    input_gain: f32,
    output_gain: f32,
    frequency: f32,
    resonance: f32,
    mode: String,
    inertia: f32,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Gate {
    state: bool,
    detection: String,
    stereo_link: String,
    range: f32,
    attack: f32,
    release: f32,
    threshold: f32,
    ratio: f32,
    knee: f32,
    makeup: f32,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Limiter {
    state: bool,
    input_gain: f32,
    limit: f32,
    lookahead: f32,
    release: f32,
    asc: bool,
    asc_level: f32,
    oversampling: i32,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Maximizer {
    state: bool,
    release: f32,
    ceiling: f32,
    threshold: f32,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Pitch {
    state: bool,
    input_gain: f32,
    output_gain: f32,
    cents: f32,
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
    input_gain: f32,
    output_gain: f32,
    room_size: String,
    decay_time: f32,
    hf_damp: f32,
    diffusion: f32,
    amount: f32,
    dry: f32,
    predelay: f32,
    bass_cut: f32,
    treble_cut: f32,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct MultibandCompressor {
    state: bool,
    input_gain: f32,
    output_gain: f32,
    freq0: f32,
    freq1: f32,
    freq2: f32,
    mode: String,
    subband: MultibandCompressorBand,
    lowband: MultibandCompressorBand,
    midband: MultibandCompressorBand,
    highband: MultibandCompressorBand,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct MultibandCompressorBand {
    threshold: f32,
    ratio: f32,
    attack: f32,
    release: f32,
    makeup: f32,
    knee: f32,
    detection: String,
    bypass: bool,
    solo: bool,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Loudness {
    state: bool,
    loudness: f32,
    output: f32,
    link: f32,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct MultibandGate {
    state: bool,
    input_gain: f32,
    output_gain: f32,
    freq0: f32,
    freq1: f32,
    freq2: f32,
    mode: String,
    subband: MultibandGateBand,
    lowband: MultibandGateBand,
    midband: MultibandGateBand,
    highband: MultibandGateBand,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct MultibandGateBand {
    reduction: f32,
    threshold: f32,
    ratio: f32,
    attack: f32,
    release: f32,
    makeup: f32,
    knee: f32,
    detection: String,
    bypass: bool,
    solo: bool,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct StereoTools {
    state: bool,
    input_gain: f32,
    output_gain: f32,
    balance_in: f32,
    balance_out: f32,
    softclip: bool,
    mutel: bool,
    muter: bool,
    phasel: bool,
    phaser: bool,
    mode: String,
    side_level: f32,
    side_balance: f32,
    middle_level: f32,
    middle_panorama: f32,
    stereo_base: f32,
    delay: f32,
    sc_level: f32,
    stereo_phase: f32,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Convolver {
    state: bool,
    input_gain: f32,
    output_gain: f32,
    kernel_path: String,
    ir_width: i32,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Crystalizer {
    state: bool,
    aggressive: bool,
    input_gain: f32,
    output_gain: f32,
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
    intensity: f32,
    mute: bool,
    bypass: bool,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct AutoGain {
    state: bool,
    detect_silence: bool,
    use_geometric_mean: bool,
    input_gain: f32,
    output_gain: f32,
    target: f32,
    weight_m: i32,
    weight_s: i32,
    weight_i: i32,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "kebab-case")]
struct Delay {
    state: bool,
    input_gain: f32,
    output_gain: f32,
    time_l: f32,
    time_r: f32,
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