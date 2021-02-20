use gtk::prelude::*;
use gtk::{gdk, gio, glib, CompositeTemplate};

// use crate::ui::general_settings;
// use crate::ui::presets_menu;

mod imp {
    use super::*;
    use glib::subclass;
    use gtk::subclass::prelude::*;

    #[derive(Debug, CompositeTemplate)]
    #[template(file = "application_window.ui")]
    pub struct ExApplicationWindow {
        #[template_child]
        pub headerbar: TemplateChild<gtk::HeaderBar>,

        #[template_child]
        pub headerbar_info: TemplateChild<gtk::Label>,

        #[template_child]
        pub headerbar_icon1: TemplateChild<gtk::Image>,

        #[template_child]
        pub headerbar_icon2: TemplateChild<gtk::Image>,
    }

    impl ObjectSubclass for ExApplicationWindow {
        const NAME: &'static str = "ExApplicationWindow";
        type Type = super::ExApplicationWindow;
        type ParentType = gtk::ApplicationWindow;
        type Interfaces = ();
        type Instance = subclass::simple::InstanceStruct<Self>;
        type Class = subclass::simple::ClassStruct<Self>;

        glib::object_subclass!();

        fn new() -> Self {
            Self {
                headerbar: TemplateChild::default(),
                headerbar_info: TemplateChild::default(),
                headerbar_icon1: TemplateChild::default(),
                headerbar_icon2: TemplateChild::default(),
            }
        }

        fn class_init(klass: &mut Self::Class) {
            Self::bind_template(klass);
        }

        fn instance_init(obj: &glib::subclass::InitializingObject<Self::Type>) {
            // let display_manager = gdk::DisplayManagerBuilder::new().build();

            // println!("{}", display_manager.list_displays().len());

            // let default_display = display_manager.get_default_display();

            // let default_theme = gtk::IconTheme::get_for_display(&default_display.unwrap()).unwrap();

            // default_theme.add_resource_path("/com/github/wwmm/pulseeffects/icons");

            obj.init_template();
        }
    }

    impl ObjectImpl for ExApplicationWindow {
        fn constructed(&self, obj: &Self::Type) {
            self.parent_constructed(obj);
        }
    }

    impl WidgetImpl for ExApplicationWindow {}
    impl WindowImpl for ExApplicationWindow {}
    impl ApplicationWindowImpl for ExApplicationWindow {}
}

glib::wrapper! {
    pub struct ExApplicationWindow(ObjectSubclass<imp::ExApplicationWindow>)
        @extends gtk::Widget, gtk::Window, gtk::ApplicationWindow, @implements gio::ActionMap, gio::ActionGroup;
}

impl ExApplicationWindow {
    pub fn new<P: glib::IsA<gtk::Application>>(app: &P) -> Self {
        glib::Object::new(&[("application", app)]).expect("Failed to create ApplicationWindow")
    }
}

// #[derive(UIResource, Debug)]
// #[resource = "/com/github/wwmm/pulseeffects/ui/application.glade"]
// struct WindowResource {
//     application_window: gtk::ApplicationWindow,
//     stack: gtk::Stack,
//     stack_menu_settings: gtk::Stack,
//     presets_menu_button: gtk::Button,
//     presets_menu: gtk::Popover,
//     presets_menu_label: gtk::Label,
//     calibration_button: gtk::Button,
//     subtitle_grid: gtk::Grid,
//     headerbar: gtk::HeaderBar,
//     help_button: gtk::Button,
//     bypass_button: gtk::Button,
//     headerbar_icon1: gtk::Image,
//     headerbar_icon2: gtk::Image,
//     headerbar_info: gtk::Label,
// }

// pub fn build_ui() -> gtk::ApplicationWindow {
//     // Register resource bundles

//     let res_bytes = include_bytes!("resources.gresource");

//     let data = glib::Bytes::from(&res_bytes[..]);

//     let resource = gio::Resource::new_from_data(&data).expect("Failed to load resources");

//     gio::resources_register(&resource);

//     let resources = WindowResource::load().unwrap();

//     // println!("res: {:?}", resources);

//     let window = resources.application_window;

//     apply_css_style("custom.css");
//     add_path_to_theme_resource();

//     resources
//         .presets_menu
//         .add(&presets_menu::build_ui(&resources.presets_menu_button));

//     resources.stack_menu_settings.add_titled(
//         &general_settings::build_ui(&window),
//         "general_spectrum",
//         "General",
//     );

//     {
//         let window = window.clone();

//         resources.help_button.connect_clicked(move |_btn| {
//             let app = window.get_application().unwrap();

//             app.activate_action("help", None);
//         });
//     }

//     let settings = gio::Settings::new("com.github.wwmm.pulseeffects");
//     let flag = gio::SettingsBindFlags::DEFAULT;

//     settings.bind(
//         "use-dark-theme",
//         &gtk::Settings::get_default().unwrap(),
//         "gtk_application_prefer_dark_theme",
//         flag,
//     );

//     settings.bind("bypass", &resources.bypass_button, "active", flag);

//     let window_width = settings.get_int("window-width");
//     let window_height = settings.get_int("window-height");

//     if window_width > 0 && window_height > 0 {
//         window.set_default_size(window_width, window_height);
//     }

//     return window;
// }

// fn apply_css_style(css_file_name: &str) {
//     let provider = gtk::CssProvider::new();

//     let path = String::from("/com/github/wwmm/pulseeffects/ui/") + css_file_name;

//     provider.load_from_resource(&path);

//     let screen = &gdk::Screen::get_default().expect("Failed to get gdk default screen");
//     let priority = gtk::STYLE_PROVIDER_PRIORITY_APPLICATION;

//     gtk::StyleContext::add_provider_for_screen(screen, &provider, priority);
// }

fn add_path_to_theme_resource() {
    let default_theme = gtk::IconTheme::default();

    default_theme.add_resource_path("/com/github/wwmm/pulseeffects/icons");
}
