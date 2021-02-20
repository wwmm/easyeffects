use gtk::prelude::*;

use gtk::Application;
use gtk::gio;

use log::*;
use std::env;

use crate::config::VERSION;
// use crate::ui::application_window::build_ui;

pub fn init() {
    let application = Application::new(
        Some("com.github.wwmm.pulseeffects.rust"),
        gtk::gio::ApplicationFlags::HANDLES_COMMAND_LINE,
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

    application.connect_startup(|app| {
        debug!("{}{}", "PE version: ", VERSION);
        debug!("{}{:?}", "Flags: ", app.get_flags());

        let mut running_as_service = false;

        if (app.get_flags() & gio::ApplicationFlags::IS_SERVICE)
            == gio::ApplicationFlags::IS_SERVICE
        {
            running_as_service = true;
        }

        create_actions(app);
    });

    application.connect_activate(|app| {
        if app.get_active_window() == Option::None {
            // let window = build_ui();

            // app.add_window(&window);

            // window.connect_hide(|obj| {
            //     let (w, h) = obj.get_size();

            //     println!("{}, {}", w, h);
            // });

            // window.show_all();
        }
    });

    application.run(&env::args().collect::<Vec<_>>());
}

fn create_actions(app: &gtk::Application) {
    let about_action = gio::SimpleAction::new("about", None);
    let quit_action = gio::SimpleAction::new("quit", None);
    let help_action = gio::SimpleAction::new("help", None);

    {
        let app = app.clone();

        about_action.connect_activate(move |_action, _parameters| {
            let builder =
                gtk::Builder::from_resource("/com/github/wwmm/pulseeffects/about.glade");

            let dialog: gtk::Dialog = builder.get_object("about_dialog").unwrap();

            dialog.connect_response(|dialog, response| match response {
                gtk::ResponseType::DeleteEvent => {
                    dialog.hide();
                }

                _ => {}
            });

            dialog.set_transient_for(app.get_active_window().as_ref());

            dialog.show();
            dialog.present(); // Bring it to the front, in case it was already shown
        });
    }

    {
        let app = app.clone();

        quit_action.connect_activate(move |_action, _parameters| {
            app.get_active_window().unwrap().hide();
        });
    }

    {
        let app = app.clone();

        // help_action.connect_activate(move |_action, _parameters| {
        //     match gtk::show_uri_on_window(
        //         app.get_active_window().as_ref(),
        //         "help:pulseeffects",
        //         gtk::get_current_event_time(),
        //     ) {
        //         Ok(_) => {}

        //         Err(err) => {
        //             warn!("{}", err);
        //         }
        //     }
        // });
    }

    app.add_action(&about_action);
    app.add_action(&quit_action);
    app.add_action(&help_action);
}
