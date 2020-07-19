use crate::presets::autogain::AutoGain;
use crate::presets::bass_enhancer::BassEnhancer;
use crate::presets::compressor;
use crate::presets::convolver::Convolver;
use crate::presets::crossfeed::Crossfeed;
use crate::presets::crystalizer::Crystalizer;
use crate::presets::deesser;
use crate::presets::delay::Delay;
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
use crate::presets::webrtc::WebRTC;
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
