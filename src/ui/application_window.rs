use gio::prelude::*;
use gtk::prelude::*;

use gdk;
use gio;
use glib;

use crate::ui::general_settings;
use crate::ui::presets_menu;
use gtk_resources::UIResource;

#[derive(UIResource, Debug)]
#[resource = "/com/github/wwmm/pulseeffects/ui/application.glade"]
struct WindowResource {
    application_window: gtk::ApplicationWindow,
    stack: gtk::Stack,
    stack_menu_settings: gtk::Stack,
    presets_menu_button: gtk::Button,
    presets_menu: gtk::Popover,
    presets_menu_label: gtk::Label,
    calibration_button: gtk::Button,
    subtitle_grid: gtk::Grid,
    headerbar: gtk::HeaderBar,
    help_button: gtk::Button,
    bypass_button: gtk::Button,
    headerbar_icon1: gtk::Image,
    headerbar_icon2: gtk::Image,
    headerbar_info: gtk::Label,
}

pub fn build_ui() -> gtk::ApplicationWindow {
    // Register resource bundles

    let res_bytes = include_bytes!("resources.gresource");

    let data = glib::Bytes::from(&res_bytes[..]);

    let resource = gio::Resource::new_from_data(&data).expect("Failed to load resources");

    gio::resources_register(&resource);

    let resources = WindowResource::load().unwrap();

    // println!("res: {:?}", resources);

    let window = resources.application_window;

    apply_css_style("custom.css");
    add_path_to_theme_resource();

    resources
        .presets_menu
        .add(&presets_menu::build_ui(&resources.presets_menu_button));

    resources.stack_menu_settings.add_titled(
        &general_settings::build_ui(&window),
        "general_spectrum",
        "General",
    );

    {
        let window = window.clone();

        resources.help_button.connect_clicked(move |_btn| {
            let app = window.get_application().unwrap();

            app.activate_action("help", None);
        });
    }

    return window;
}

fn apply_css_style(css_file_name: &str) {
    let provider = gtk::CssProvider::new();

    let path = String::from("/com/github/wwmm/pulseeffects/ui/") + css_file_name;

    provider.load_from_resource(&path);

    let screen = &gdk::Screen::get_default().expect("Failed to get gdk default screen");
    let priority = gtk::STYLE_PROVIDER_PRIORITY_APPLICATION;

    gtk::StyleContext::add_provider_for_screen(screen, &provider, priority);
}

fn add_path_to_theme_resource() {
    let default_theme = gtk::IconTheme::get_default().expect("Could not get default icon theme");

    default_theme.add_resource_path("/com/github/wwmm/pulseeffects/icons");
}
