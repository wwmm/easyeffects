// use log::*;
use gettextrs::*;

mod config;
mod application;
mod ui;
mod presets;

fn main() {
    env_logger::init();

    // debug!("this is a debug {}", "message");
    // error!("this is printed by default");

    bindtextdomain(config::GETTEXT_PACKAGE, config::LOCALE_DIR);
    textdomain(config::GETTEXT_PACKAGE);

    // Creating the gtk application

    application::init();
}
