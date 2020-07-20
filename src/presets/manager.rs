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
use glib;
use log::*;
use serde::{Deserialize, Serialize};
use serde_json;
use serde_yaml;
use std::fs;
use std::path::{Path, PathBuf};

#[derive(Clone)]
pub enum PresetType {
    Input,
    Output,
}

#[derive(Clone)]
pub struct Manager {
    input_presets_directories: Vec<String>,
    output_presets_directories: Vec<String>,
    json: serde_yaml::Value,
}

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

impl Manager {
    pub fn new() -> Manager {
        // User presets directories

        let user_config_directory =
            String::from(glib::get_user_config_dir().unwrap().to_str().unwrap());

        let user_input_directory = user_config_directory.clone() + "/PulseEffects/input";
        let user_output_directory = user_config_directory.clone() + "/PulseEffects/output";
        let user_autoload_directory = user_config_directory + "/PulseEffects/autoload";

        fs::create_dir_all(&user_input_directory).expect("Could not create preset directory");
        fs::create_dir_all(&user_output_directory).expect("Could not create preset directory");
        fs::create_dir_all(&user_autoload_directory)
            .expect("Could not create preset autoload directory");

        let mut input_directories = Vec::new();
        let mut output_directories = Vec::new();

        input_directories.push(user_input_directory);
        output_directories.push(user_output_directory);

        // appending system presets paths in case they exist

        for directory in glib::get_system_config_dirs() {
            let directory_name = String::from(directory.to_str().unwrap());

            let input_dir = directory_name.clone() + "/PulseEffects/input";
            let output_dir = directory_name + "/PulseEffects/output";

            if Path::new(&input_dir).is_dir() {
                input_directories.push(input_dir);
            }

            if Path::new(&output_dir).is_dir() {
                output_directories.push(output_dir);
            }
        }

        Manager {
            input_presets_directories: input_directories,
            output_presets_directories: output_directories,
            json: serde_json::from_str("{}").unwrap(),
        }
    }

    pub fn get_names(&self, preset_type: &PresetType) -> Vec<String> {
        let mut names = Vec::new();

        match preset_type {
            PresetType::Output => {
                for dir in &self.output_presets_directories {
                    let presets = get_presets_list(dir);

                    for preset in presets {
                        names.push(preset);
                    }
                }

                names.sort();
                names.dedup();

                return names;
            }
            PresetType::Input => {
                for dir in &self.input_presets_directories {
                    let presets = get_presets_list(dir);

                    for preset in presets {
                        names.push(preset);
                    }
                }

                names.sort();
                names.dedup();

                return names;
            }
        }
    }

    pub fn load(&mut self, preset_type: &PresetType, name: &String) {
        let yaml_string = self.preset_file_to_string(preset_type, name);

        if yaml_string.is_empty() {
            return;
        }

        match preset_type {
            PresetType::Output => {
                match serde_yaml::from_str::<OutputRoot>(yaml_string.as_str()) {
                    Ok(root) => {
                        println!("{:?}", root);
                    }

                    Err(err) => {
                        error!("{:?}", err);
                    }
                }

                // println!("{:?}", self.json["output"]["plugins_order"]);
            }
            PresetType::Input => match serde_yaml::from_str::<InputRoot>(yaml_string.as_str()) {
                Ok(root) => {
                    println!("{:?}", root);
                }

                Err(err) => {
                    error!("{:?}", err);
                }
            },
        }
    }

    fn check_file_type(
        &self,
        preset_type: &PresetType,
        name: &String,
        extension: &str,
    ) -> (bool, PathBuf) {
        let mut name = name.clone();

        name.push_str(extension);

        let directory_list = {
            match preset_type {
                PresetType::Output => &self.output_presets_directories,
                PresetType::Input => &self.input_presets_directories,
            }
        };

        for dir in directory_list {
            let file_path = PathBuf::new();
            let f_path = file_path.join(dir).join(&name);

            if f_path.is_file() {
                return (true, f_path);
            }
        }

        return (false, PathBuf::new());
    }

    fn preset_file_to_string(&self, preset_type: &PresetType, name: &String) -> String {
        let (file_exists, yaml_path) = self.check_file_type(preset_type, name, ".yaml");

        if file_exists {
            let file_string =
                fs::read_to_string(yaml_path).expect("could not read yaml file to string");

            return file_string;
        }

        // json is the old format. If the check returns true we generate a new yaml file with its contents

        let (file_exists, json_path) = self.check_file_type(preset_type, name, ".json");

        if file_exists {
            let file_string =
                fs::read_to_string(json_path).expect("could not read json file to string");

            let yaml_value: serde_yaml::Value = serde_json::from_str(file_string.as_str()).unwrap();

            let mut yaml_string = serde_yaml::to_string(&yaml_value).unwrap();

            // Old presets had a key name "type" but this is a protected Rust keyword.

            yaml_string = yaml_string.replace("type", "style");

            // Old presets were generated by C++ Boost. Unfortunately it puts all values between quotes.
            // We have to remove them
            yaml_string = yaml_string.replace("\"", "");

            return yaml_string;
        }

        return String::new();
    }
}

fn get_presets_list(directory: &String) -> Vec<String> {
    let dir_path = Path::new(&directory);
    let mut output = Vec::new();

    if dir_path.is_dir() {
        for entry in dir_path.read_dir().expect("read_dir call failed") {
            let file_path = entry.unwrap().path();

            if file_path.is_file() {
                if file_path.extension().unwrap() == "json" {
                    let name = file_path.file_stem().unwrap().to_str().unwrap();

                    output.push(String::from(name));
                }
            }
        }
    }

    return output;
}
