use crate::presets::root;
use glib;
use log::*;
use serde_json;
use serde_yaml;
use std::fs;
use std::io::prelude::*;
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

    pub fn save(&mut self, preset_type: &PresetType, name: &String) {
        match preset_type {
            PresetType::Output => {
                let o = root::Output::default();

                let yaml_string = serde_yaml::to_string(&o).unwrap();

                self.save_file(preset_type, name, yaml_string);
            }
            PresetType::Input => {
                let o = root::Input::default();

                let yaml_string = serde_yaml::to_string(&o).unwrap();

                self.save_file(preset_type, name, yaml_string);
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
                match serde_yaml::from_str::<root::Output>(yaml_string.as_str()) {
                    Ok(root) => {
                        // println!("{:?}", root);
                        root.apply();
                    }

                    Err(err) => {
                        error!("{:?}", err);
                    }
                }

                // println!("{:?}", self.json["output"]["plugins_order"]);
            }
            PresetType::Input => {
                match serde_yaml::from_str::<root::Input>(yaml_string.as_str()) {
                    Ok(root) => {
                        // println!("{:?}", root);
                        root.apply();
                    }

                    Err(err) => {
                        error!("{:?}", err);
                    }
                }

                // println!("{:?}", self.json["input"]["plugins_order"]);
            }
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

    fn save_file(&self, preset_type: &PresetType, name: &String, body: String) {
        let preset_directory = match preset_type {
            PresetType::Output => {
                let mut p = self.output_presets_directories[0].clone();
                p.push_str("/output");

                p
            }
            PresetType::Input => {
                let mut p = self.input_presets_directories[0].clone();

                p.push_str("/input");

                p
            }
        };

        let preset_directory = Path::new(&preset_directory);

        let preset_file = preset_directory.with_file_name(name.clone() + ".yaml");

        let mut output = fs::File::create(preset_file).unwrap();

        output
            .write_all(body.as_bytes())
            .expect("Could not save preset file");
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
