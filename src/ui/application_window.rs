use gio::prelude::*;
use gtk::prelude::*;
use gtk::{gdk, CompositeTemplate};

// use crate::ui::general_settings;
use crate::ui::presets_menu::ExPresetsMenu;

mod imp {
    use super::*;
    use glib::subclass;
    use gtk::subclass::prelude::*;

    #[derive(Debug, CompositeTemplate)]
    #[template(resource = "/com/github/wwmm/pulseeffects/ui/application_window.ui")]
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
        pub presets_menu_button: TemplateChild<gtk::MenuButton>,

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
            self.parent_constructed(obj);

            obj.add_resource_icons_to_theme_path();

            // let presets_menu = ExPresetsMenu::new();
            self.presets_menu_button.set_popover(Some(&ExPresetsMenu::new()));
            // self.presets_menu.set_child(Some(&ExPresetsMenu::new()));
            // ExPresetsMenu::new2(&self.presets_menu.get());

            let settings = gio::Settings::new("com.github.wwmm.pulseeffects");

            settings
                .bind(
                    "use-dark-theme",
                    &gtk::Settings::get_default().unwrap(),
                    "gtk_application_prefer_dark_theme",
                )
                .build();

            settings
                .bind("bypass", &self.bypass_button.get(), "active")
                .build();

            let window_width = settings.get_int("window-width");
            let window_height = settings.get_int("window-height");

            if window_width > 0 && window_height > 0 {
                self.get_instance()
                    .set_default_size(window_width, window_height);
            }

            {
                // let app = self.get_instance().get_application().unwrap();

                self.help_button.connect_clicked(move |_btn| {
                    // app.activate_action("help", None);
                    println!("oi");
                });
            }
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

    pub fn add_resource_icons_to_theme_path(&self) {
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

//     return window;
// }
