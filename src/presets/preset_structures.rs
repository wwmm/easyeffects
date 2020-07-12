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
