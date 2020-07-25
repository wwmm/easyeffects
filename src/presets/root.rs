use crate::presets::autogain::AutoGain;
use crate::presets::bass_enhancer::BassEnhancer;
use crate::presets::compressor;
use crate::presets::convolver::Convolver;
use crate::presets::crossfeed::Crossfeed;
use crate::presets::crystalizer::Crystalizer;
use crate::presets::deesser;
use crate::presets::delay::Delay;
use crate::presets::equalizer;
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
pub struct Output {
    spectrum: Spectrum,
    output: OutputChildren,
}

impl Default for Output {
    fn default() -> Self {
        Output {
            spectrum: Spectrum::default(),
            output: OutputChildren::default(),
        }
    }
}

impl Output {
    pub fn apply(&self){
        self.spectrum.apply();
        self.output.apply();
    }
}

#[derive(Serialize, Deserialize, Debug)]
pub struct Input {
    spectrum: Spectrum,
    input: InputChildren,
}

impl Default for Input {
    fn default() -> Self {
        Input {
            spectrum: Spectrum::default(),
            input: InputChildren::default(),
        }
    }
}

impl Input {
    pub fn apply(&self){
        self.spectrum.apply();
        self.input.apply();
    }
}

#[derive(Serialize, Deserialize, Debug)]
struct OutputChildren {
    plugins_order: Vec<String>,
    bass_enhancer: BassEnhancer,
    compressor: compressor::Output,
    crossfeed: Crossfeed,
    deesser: deesser::Output,
    equalizer: equalizer::Output,
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

impl Default for OutputChildren {
    fn default() -> Self {
        OutputChildren {
            plugins_order: Vec::new(),
            bass_enhancer: BassEnhancer::default(),
            compressor: compressor::Output::default(),
            crossfeed: Crossfeed::default(),
            deesser: deesser::Output::default(),
            equalizer: equalizer::Output::default(),
            exciter: Exciter::default(),
            filter: filter::Output::default(),
            gate: gate::Output::default(),
            limiter: limiter::Output::default(),
            maximizer: maximizer::Output::default(),
            pitch: pitch::Output::default(),
            reverb: reverb::Output::default(),
            multiband_compressor: multiband_compressor::Output::default(),
            loudness: Loudness::default(),
            multiband_gate: multiband_gate::Output::default(),
            stereo_tools: StereoTools::default(),
            convolver: Convolver::default(),
            crystalizer: Crystalizer::default(),
            autogain: AutoGain::default(),
            delay: Delay::default(),
        }
    }
}

impl OutputChildren {
    pub fn apply(&self){
        self.autogain.apply();
        self.bass_enhancer.apply();
        self.compressor.apply();
        self.convolver.apply();
    }
}

#[derive(Serialize, Deserialize, Debug)]
struct InputChildren {
    plugins_order: Vec<String>,
    compressor: compressor::Input,
    deesser: deesser::Input,
    equalizer: equalizer::Input,
    filter: filter::Input,
    gate: gate::Input,
    limiter: limiter::Input,
    pitch: pitch::Input,
    reverb: reverb::Input,
    webrtc: WebRTC,
    multiband_compressor: multiband_compressor::Input,
    multiband_gate: multiband_gate::Input,
}

impl Default for InputChildren {
    fn default() -> Self {
        InputChildren {
            plugins_order: Vec::new(),
            compressor: compressor::Input::default(),
            deesser: deesser::Input::default(),
            equalizer: equalizer::Input::default(),
            filter: filter::Input::default(),
            gate: gate::Input::default(),
            limiter: limiter::Input::default(),
            pitch: pitch::Input::default(),
            reverb: reverb::Input::default(),
            webrtc: WebRTC::default(),
            multiband_compressor: multiband_compressor::Input::default(),
            multiband_gate: multiband_gate::Input::default(),
        }
    }
}

impl InputChildren {
    pub fn apply(&self){
        self.compressor.apply();
    }
}
