use gio::prelude::*;
use gtk::prelude::*;

use gtk::Application;
use std::env;

use crate::config::VERSION;
use crate::ui::application_window::build_ui;

pub fn init() {
    let application = Application::new(
        Some("com.github.wwmm.pulseeffects.rust"),
        gio::ApplicationFlags::HANDLES_COMMAND_LINE,
    )
    .expect("failed to initialize GTK application");

    application.connect_command_line(|app, command_line| {
        for arg in command_line.get_arguments() {
            match arg.to_str() {
                Some("-v") | Some("--version") => {
                    println!("{}", VERSION);
                    app.quit();
                }

                Some("-q") | Some("--quit") => {
                    println!("quitting");
                    app.quit();
                }

                Some("-l") | Some("--load-preset") => {
                    println!("loading preset...");
                }

                Some("-r") | Some("--reset") => {
                    println!("resetting...");
                }

                _ => app.activate(),
            };
        }

        0
    });

    application.connect_activate(|app| {
        if app.get_active_window() == Option::None {
            let window = build_ui();

            app.add_window(&window);

            window.connect_hide(|obj| {
                let (w, h) = obj.get_size();

                println!("{}, {}", w, h);
            });

            window.show_all();
        }
    });

    application.run(&env::args().collect::<Vec<_>>());
}
