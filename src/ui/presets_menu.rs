use gtk::prelude::*;
use gtk::subclass::widget::WidgetImplExt;
use gtk::CompositeTemplate;

use crate::presets::manager;
use std::sync::{Arc, Mutex};

mod imp {
    use super::*;
    use glib::subclass;
    use gtk::subclass::prelude::*;

    #[derive(Debug, CompositeTemplate)]
    #[template(resource = "/com/github/wwmm/pulseeffects/ui/presets_menu.ui")]
    pub struct ExPresetsMenu {
        #[template_child]
        pub stack: TemplateChild<gtk::Stack>,

        #[template_child]
        pub import_output: TemplateChild<gtk::Button>,

        #[template_child]
        pub add_output: TemplateChild<gtk::Button>,

        #[template_child]
        pub output_name: TemplateChild<gtk::Entry>,

        #[template_child]
        pub output_scrolled_window: TemplateChild<gtk::ScrolledWindow>,

        #[template_child]
        pub output_listview: TemplateChild<gtk::ListView>,

        #[template_child]
        pub import_input: TemplateChild<gtk::Button>,

        #[template_child]
        pub add_input: TemplateChild<gtk::Button>,

        #[template_child]
        pub input_name: TemplateChild<gtk::Entry>,

        #[template_child]
        pub input_scrolled_window: TemplateChild<gtk::ScrolledWindow>,

        #[template_child]
        pub input_listview: TemplateChild<gtk::ListView>,

        pub presets_manager: manager::Manager,

        pub output_string_list: gtk::StringList,
    }

    impl ObjectSubclass for ExPresetsMenu {
        const NAME: &'static str = "ExPresetsMenu";
        type Type = super::ExPresetsMenu;
        type ParentType = gtk::Popover;
        type Interfaces = ();
        type Instance = subclass::simple::InstanceStruct<Self>;
        type Class = subclass::simple::ClassStruct<Self>;

        glib::object_subclass!();

        fn new() -> Self {
            Self {
                stack: TemplateChild::default(),
                import_output: TemplateChild::default(),
                add_output: TemplateChild::default(),
                output_name: TemplateChild::default(),
                output_scrolled_window: TemplateChild::default(),
                output_listview: TemplateChild::default(),
                import_input: TemplateChild::default(),
                add_input: TemplateChild::default(),
                input_name: TemplateChild::default(),
                input_scrolled_window: TemplateChild::default(),
                input_listview: TemplateChild::default(),
                presets_manager: manager::Manager::new(),
                output_string_list: gtk::StringList::new(&[""]),
            }
        }

        fn class_init(klass: &mut Self::Class) {
            Self::bind_template(klass);
        }

        fn instance_init(obj: &glib::subclass::InitializingObject<Self::Type>) {
            obj.init_template();
        }
    }

    impl ObjectImpl for ExPresetsMenu {
        fn constructed(&self, obj: &Self::Type) {
            self.parent_constructed(obj);

            self.configure_output_listview();
        }
    }

    impl WidgetImpl for ExPresetsMenu {
        fn show(&self, widget: &Self::Type) {
            self.parent_show(widget);

            println!("oi");

            self.populate_listview(manager::PresetType::Output);
        }
    }

    impl PopoverImpl for ExPresetsMenu {}

    impl ExPresetsMenu {
        pub fn configure_output_listview(&self) {
            self.output_string_list.remove(0);

            let no_selection_model = gtk::NoSelectionBuilder::new().build();

            no_selection_model.set_model(Some(&self.output_string_list));

            self.output_listview.set_model(Some(&no_selection_model));

            let output_factory = gtk::BuilderListItemFactoryBuilder::new()
                .resource("/com/github/wwmm/pulseeffects/ui/preset_row.ui")
                .build();

            // let factory = gtk::SignalListItemFactory::new();

            // factory.connect_setup(|_factory, list_item| {});

            // factory.connect_bind(|_factory, list_item| {});

            self.output_listview.set_factory(Some(&output_factory));
        }

        pub fn populate_listview(&self, preset_type: manager::PresetType) {
            let mut n_items = self.output_string_list.get_n_items();

            while n_items > 0 {
                self.output_string_list.remove(n_items - 1);

                n_items = self.output_string_list.get_n_items();
            }

            let names = self.presets_manager.get_names(&preset_type);

            for name in names {
                self.output_string_list.append(&name);
            }
        }
    }
}

glib::wrapper! {
    pub struct ExPresetsMenu(ObjectSubclass<imp::ExPresetsMenu>) @extends gtk::Widget, gtk::Popover;
}

impl ExPresetsMenu {
    pub fn new() -> Self {
        glib::Object::new(&[]).expect("Failed to create the presets menu")
    }
}

// pub fn build_ui(button: &gtk::Button) -> gtk::Grid {

//         button.connect_clicked(move |obj| {
//             let top_widget = obj
//                 .get_toplevel()
//                 .expect("Could not get presets menu top level widget");

//             let height = top_widget.get_allocated_height() as f32;

//             output_scrolled_window.set_max_content_height((0.7 * height) as i32);

//             populate_listbox(
//                 &presets_manager,
//                 &manager::PresetType::Output,
//                 &output_listbox,
//             );
//         });
//     }

//     {
//         let output_name = resources.output_name.clone();
//         let output_listbox = resources.output_listbox.clone();
//         let presets_manager = presets_manager.clone();

//         resources.add_output.connect_clicked(move |_btn| {
//             create_preset(
//                 &presets_manager,
//                 &manager::PresetType::Output,
//                 &output_name,
//                 &output_listbox,
//             );

//             populate_listbox(
//                 &presets_manager,
//                 &manager::PresetType::Output,
//                 &output_listbox,
//             );
//         });
//     }

//     {
//         let input_name = resources.input_name.clone();
//         let input_listbox = resources.input_listbox.clone();
//         let presets_manager = presets_manager.clone();

//         resources.add_input.connect_clicked(move |_btn| {
//             create_preset(
//                 &presets_manager,
//                 &manager::PresetType::Input,
//                 &input_name,
//                 &input_listbox,
//             );

//             populate_listbox(
//                 &presets_manager,
//                 &manager::PresetType::Input,
//                 &input_listbox,
//             );
//         });
//     }

//     return resources.widgets_grid;
// }

fn create_preset(
    presets_manager: &std::sync::Arc<std::sync::Mutex<manager::Manager>>,
    preset_type: &manager::PresetType,
    entry: &gtk::Entry,
    listbox: &gtk::ListBox,
) {
    let name = entry.get_text().to_string();

    if name.chars().all(char::is_alphanumeric) {
        presets_manager.lock().unwrap().add(preset_type, &name);

        populate_listbox(&presets_manager, &preset_type, &listbox);
    }

    entry.set_text("");
}

fn populate_listbox(
    presets_manager: &std::sync::Arc<std::sync::Mutex<manager::Manager>>,
    preset_type: &manager::PresetType,
    listbox: &gtk::ListBox,
) {
    let names = presets_manager.lock().unwrap().get_names(preset_type);

    for name in names {
        let builder =
            gtk::Builder::from_resource("/com/github/wwmm/pulseeffects/ui/preset_row.glade");

        let row: gtk::ListBoxRow = builder
            .get_object("preset_row")
            .expect("builder could not get the widget: preset_row");

        let apply_btn: gtk::Button = builder
            .get_object("apply")
            .expect("builder could not get the widget: apply");

        let save_btn: gtk::Button = builder
            .get_object("save")
            .expect("builder could not get the widget: save");

        let remove_btn: gtk::Button = builder
            .get_object("remove")
            .expect("builder could not get the widget: remove");

        let label: gtk::Label = builder
            .get_object("name")
            .expect("builder could not get the widget: name");

        let autoload_btn: gtk::Button = builder
            .get_object("autoload")
            .expect("builder could not get the widget: autoload");

        row.set_widget_name(name.as_str());

        label.set_text(name.as_str());

        // if (is_autoloaded(preset_type, name)) {
        //     autoload_btn->set_active(true);
        // }

        let presets_manager = presets_manager.clone();

        {
            let presets_manager = presets_manager.clone();
            let preset_type = (*preset_type).clone();
            let name = name.clone();

            apply_btn.connect_clicked(move |_btn| {
                // settings->set_string("last-used-preset", row->get_name());

                presets_manager.lock().unwrap().load(&preset_type, &name);
            });
        }

        {
            let presets_manager = presets_manager.clone();
            let preset_type = (*preset_type).clone();
            let name = name.clone();

            save_btn.connect_clicked(move |_btn| {
                presets_manager.lock().unwrap().save(&preset_type, &name);
            });
        }

        {
            let presets_manager = presets_manager.clone();
            let preset_type = (*preset_type).clone();
            let name = name.clone();
            let listbox = listbox.clone();

            remove_btn.connect_clicked(move |_btn| {
                presets_manager.lock().unwrap().remove(&preset_type, &name);

                populate_listbox(&presets_manager, &preset_type, &listbox);
            });
        }

        autoload_btn.connect_clicked(|obj| {
            // if (preset_type == PresetType::output) {
            //     auto dev_name = build_device_name(preset_type, app->pm->server_info.default_sink_name);
            //     if (autoload_btn->get_active()) {
            //       app->presets_manager->add_autoload(dev_name, name);
            //     } else {
            //       app->presets_manager->remove_autoload(dev_name, name);
            //     }
            //   } else {
            //     auto dev_name = build_device_name(preset_type, app->pm->server_info.default_source_name);
            //     if (autoload_btn->get_active()) {
            //       app->presets_manager->add_autoload(dev_name, name);
            //     } else {
            //       app->presets_manager->remove_autoload(dev_name, name);
            //     }
            //   }
            //   populate_listbox(preset_type);
        });
    }
}
