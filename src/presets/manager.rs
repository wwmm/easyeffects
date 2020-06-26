use glib;
use log::*;
use std::fs;

pub enum PresetType {
    Input,
    Output,
}

pub struct Manager {
    input_presets_directories: Vec<String>,
    output_presets_directories: Vec<String>,
}

impl Manager {
    pub fn new() -> Manager {
        let mut input_directories = Vec::new();
        let mut output_directories = Vec::new();

        for directory in glib::get_system_config_dirs() {
            let directory_name = String::from(directory.to_str().unwrap());

            input_directories.push(directory_name.clone() + "/PulseEffects/input");
            output_directories.push(directory_name.clone() + "/PulseEffects/output");
        }

        let user_config_directory =
            String::from(glib::get_user_config_dir().unwrap().to_str().unwrap());

        let user_input_directory = user_config_directory.clone() + "/PulseEffects/input";
        let user_output_directory = user_config_directory.clone() + "/PulseEffects/output";
        let user_autoload_directory = user_config_directory + "/PulseEffects/autoload";

        fs::create_dir_all(&user_input_directory).expect("Could not create preset directory");
        fs::create_dir_all(&user_output_directory).expect("Could not create preset directory");
        fs::create_dir_all(&user_autoload_directory).expect("Could not create preset autoload directory");

        input_directories.push(user_input_directory);
        output_directories.push(user_output_directory);

        input_directories.sort();
        input_directories.dedup();

        output_directories.sort();
        output_directories.dedup();

        debug!("input presets paths {:?}", input_directories);
        debug!("output presets paths {:?}", output_directories);

        Manager {
            input_presets_directories: input_directories,
            output_presets_directories: output_directories,
        }
    }

    pub fn get_names(&self, preset_type: PresetType) {
        // system directories search
        match preset_type {
            PresetType::Output => {}
            PresetType::Input => {}
        }
    }
}
