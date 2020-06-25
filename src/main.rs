// use log::*;
use gettextrs::*;

mod config;
mod application;
mod ui;

fn main() {
    env_logger::init();

    // debug!("this is a debug {}", "message");
    // error!("this is printed by default");

    bindtextdomain(config::GETTEXT_PACKAGE, config::LOCALE_DIR);
    textdomain(config::GETTEXT_PACKAGE);

    // Initializing GTK

    gtk::init().expect("Unable to initialize GTK3");

    // Setting glibproperties

    glib::set_application_name("PulseEffects");

    glib::setenv(
        "PULSE_PROP_application.id",
        "com.github.wwmm.pulseeffects",
        false,
    )
    .expect("setenv failed to set the application id");

    glib::setenv("PULSE_PROP_application.icon_name", "pulseeffects", false)
        .expect("setenv failed to set the application icon name");

    // Creating the gtk application

    application::init();
}
