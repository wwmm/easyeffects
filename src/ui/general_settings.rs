use gtk::prelude::*;

use gtk::glib;
use gtk::gio;
use std::fs;
use std::io::prelude::*;
use std::path::Path;

// use gtk_resources::UIResource;

// #[derive(UIResource, Debug)]
// #[resource = "/com/github/wwmm/pulseeffects/ui/general_settings.glade"]
// struct WindowResource {
//     widgets_grid: gtk::Grid,
//     theme_switch: gtk::Switch,
//     enable_autostart: gtk::Switch,
//     enable_all_sinkinputs: gtk::Switch,
//     enable_all_sourceoutputs: gtk::Switch,
//     reset_settings: gtk::Button,
//     about_button: gtk::Button,
//     realtime_priority: gtk::SpinButton,
//     niceness: gtk::SpinButton,
//     priority_type: gtk::ComboBoxText,
//     adjustment_priority: gtk::Adjustment,
//     adjustment_niceness: gtk::Adjustment,
// }

// pub fn build_ui(app_window: &gtk::ApplicationWindow) -> gtk::Grid {
//     let resources = WindowResource::load().unwrap();

//     resources
//         .enable_autostart
//         .connect_state_set(on_enable_autostart);

//     {
//         let window = app_window.clone();

//         resources.about_button.connect_clicked(move |_btn| {
//             let app = window.get_application().unwrap();

//             app.activate_action("about", None);
//         });
//     }

//     let settings = gio::Settings::new("com.github.wwmm.pulseeffects");

//     {
//         let settings = settings.clone();

//         resources.reset_settings.connect_clicked(move |_btn| {
//             settings.reset("");
//         });
//     }

//     let flag = gio::SettingsBindFlags::DEFAULT;

//     settings.bind("use-dark-theme", &resources.theme_switch, "active", flag);

//     settings.bind(
//         "enable-all-sinkinputs",
//         &resources.enable_all_sinkinputs,
//         "active",
//         flag,
//     );

//     settings.bind(
//         "enable-all-sourceoutputs",
//         &resources.enable_all_sourceoutputs,
//         "active",
//         flag,
//     );

//     settings.bind(
//         "realtime-priority",
//         &resources.adjustment_priority,
//         "value",
//         flag,
//     );

//     settings.bind("niceness", &resources.adjustment_niceness, "value", flag);

//     return resources.widgets_grid;
// }

// fn on_enable_autostart(_switch: &gtk::Switch, state: bool) -> gtk::Inhibit {
//     let user_config_directory =
//         String::from(glib::get_user_config_dir().unwrap().to_str().unwrap());

//     let autostart_directory = user_config_directory + "/autostart/";

//     fs::create_dir_all(&autostart_directory).expect("Could not create autostart directory");

//     let autostart_file = autostart_directory + "/pulseeffects-service.desktop";

//     let autostart_file = Path::new(&autostart_file);

//     if state {
//         if !autostart_file.is_file() {
//             let mut body = String::from("[Desktop Entry]\n");

//             body.push_str("Name=PulseEffects\n");
//             body.push_str("Comment=PulseEffects Service\n");
//             body.push_str("Exec=pulseeffects --gapplication-service\n");
//             body.push_str("Icon=pulseeffects\n");
//             body.push_str("StartupNotify=false\n");
//             body.push_str("Terminal=false\n");
//             body.push_str("Type=Application\n");

//             let mut output = fs::File::create(autostart_file).unwrap();

//             output
//                 .write_all(body.as_bytes())
//                 .expect("Could not save autostart file");
//         }
//     } else if autostart_file.is_file() {
//         fs::remove_file(autostart_file).expect("Could not remove the autostart file");
//     }

//     gtk::Inhibit(false)
// }
