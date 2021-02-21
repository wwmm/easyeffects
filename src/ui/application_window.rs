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

        #[template_child]
        pub stack: TemplateChild<gtk::Stack>,

        #[template_child]
        pub stack_menu_settings: TemplateChild<gtk::Stack>,

        #[template_child]
        pub presets_menu_button: TemplateChild<gtk::Button>,

        #[template_child]
        pub presets_menu: TemplateChild<gtk::Popover>,

        #[template_child]
        pub calibration_button: TemplateChild<gtk::Button>,

        #[template_child]
        pub subtitle_grid: TemplateChild<gtk::Grid>,

        #[template_child]
        pub help_button: TemplateChild<gtk::Button>,

        #[template_child]
        pub bypass_button: TemplateChild<gtk::Button>,
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
                stack: TemplateChild::default(),
                stack_menu_settings: TemplateChild::default(),
                presets_menu_button: TemplateChild::default(),
                presets_menu: TemplateChild::default(),
                calibration_button: TemplateChild::default(),
                subtitle_grid: TemplateChild::default(),
                help_button: TemplateChild::default(),
                bypass_button: TemplateChild::default(),
            }
        }

        fn class_init(klass: &mut Self::Class) {
            Self::bind_template(klass);
        }

        fn instance_init(obj: &glib::subclass::InitializingObject<Self::Type>) {
            obj.init_template();
        }
    }

    impl ObjectImpl for ExApplicationWindow {
        fn constructed(&self, obj: &Self::Type) {
            obj.register_resources();
            obj.add_resource_icons_to_theme_path();

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

    pub fn register_resources(&self){
        let res_bytes = include_bytes!("resources.gresource");

        let data = glib::Bytes::from(&res_bytes[..]);

        let resource = gio::Resource::from_data(&data).expect("Failed to load resources");

        gio::resources_register(&resource);
    }

    pub fn add_resource_icons_to_theme_path(&self){
        let default_display = gdk::Display::get_default();

        let default_theme = gtk::IconTheme::get_for_display(&default_display.unwrap()).unwrap();

        default_theme.add_resource_path("/com/github/wwmm/pulseeffects/icons");
    }
}

// pub fn build_ui() -> gtk::ApplicationWindow {
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
