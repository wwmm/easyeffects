use gtk::gio;
use gtk::glib;

use gio::prelude::*;

pub fn update_key<
    T: glib::variant::FromVariant + std::cmp::PartialEq + glib::variant::ToVariant,
>(
    settings: &gio::Settings,
    key: &str,
    new_value: T,
) {
    let current_value = settings.get_value(key).get::<T>().unwrap();

    if current_value != new_value {
        settings
            .set_value(key, &new_value.to_variant())
            .expect("Could not write value to the gsettings database");
    }
}

pub fn update_string_key(settings: &gio::Settings, key: &str, new_value: &String) {
    let current_value = settings.get_string(key).unwrap().to_string();

    if current_value != *new_value {
        settings
            .set_string(key, &new_value)
            .expect("Could not write value to the gsettings database");
    }
}
