use gtk::prelude::*;
use gtk::CompositeTemplate;

use crate::presets::manager;
use std::sync::{Arc, Mutex};

mod imp {
    use super::*;
    use glib::subclass;
    use gtk::subclass::prelude::*;

    #[derive(Debug, CompositeTemplate)]
    #[template(file = "presets_menu.ui")]
    pub struct ExPresetsMenu {
        #[template_child]
        pub stack: TemplateChild<gtk::Stack>,

        #[template_child]
        pub output_name: TemplateChild<gtk::Entry>,
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
                output_name: TemplateChild::default(),
                // headerbar_icon1: TemplateChild::default(),
                // headerbar_icon2: TemplateChild::default(),
                // stack_menu_settings: TemplateChild::default(),
                // presets_menu_button: TemplateChild::default(),
                // presets_menu: TemplateChild::default(),
                // calibration_button: TemplateChild::default(),
                // subtitle_grid: TemplateChild::default(),
                // help_button: TemplateChild::default(),
                // bypass_button: TemplateChild::default(),
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
        }
    }

    impl WidgetImpl for ExPresetsMenu {}
    impl PopoverImpl for ExPresetsMenu {}
}

glib::wrapper! {
    pub struct ExPresetsMenu(ObjectSubclass<imp::ExPresetsMenu>) @extends gtk::Widget, gtk::Popover;
}

impl ExPresetsMenu {
    pub fn new() -> Self {
        glib::Object::new(&[]).expect("Failed to create the presets menu")
    }

    pub fn new2<P: glib::IsA<gtk::Popover>>(parent: &P) -> Self {
        glib::Object::new(&[("parent", parent)]).expect("Failed to create the presets menu")
    }
}

// #[derive(UIResource, Debug)]
// #[resource = "/com/github/wwmm/pulseeffects/ui/presets_menu.glade"]
// struct WindowResource {
//     widgets_grid: gtk::Grid,
//     stack: gtk::Stack,
//     output_listbox: gtk::ListBox,
//     output_scrolled_window: gtk::ScrolledWindow,
//     output_name: gtk::Entry,
//     add_output: gtk::Button,
//     import_output: gtk::Button,
//     input_listbox: gtk::ListBox,
//     input_scrolled_window: gtk::ScrolledWindow,
//     input_name: gtk::Entry,
//     add_input: gtk::Button,
//     import_input: gtk::Button,
// }

// pub fn build_ui(button: &gtk::Button) -> gtk::Grid {
//     let resources = WindowResource::load().unwrap();

//     let output_scrolled_window = resources.output_scrolled_window;

//     resources
//         .output_listbox
//         .set_sort_func(Some(Box::new(on_listbox_sort)));

//     resources
//         .input_listbox
//         .set_sort_func(Some(Box::new(on_listbox_sort)));

//     let presets_manager = Arc::new(Mutex::new(manager::Manager::new()));

//     {
//         let presets_manager = presets_manager.clone();
//         let input_listbox = resources.input_listbox.clone();
//         let output_listbox = resources.output_listbox.clone();

//         button.connect_clicked(move |obj| {
//             let top_widget = obj
//                 .get_toplevel()
//                 .expect("Could not get presets menu top level widget");

//             let height = top_widget.get_allocated_height() as f32;

//             output_scrolled_window.set_max_content_height((0.7 * height) as i32);

//             populate_listbox(
//                 &presets_manager,
//                 &manager::PresetType::Input,
//                 &input_listbox,
//             );

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

fn on_listbox_sort(row1: &gtk::ListBoxRow, row2: &gtk::ListBoxRow) -> i32 {
    let mut names = Vec::new();
    let name1 = row1.get_widget_name();
    let name2 = row2.get_widget_name();

    names.push(&name1);
    names.push(&name2);
    names.sort();

    if name1 == *names[0] {
        return -1;
    }
    if name2 == *names[0] {
        return 1;
    }

    return 0;
}

fn populate_listbox(
    presets_manager: &std::sync::Arc<std::sync::Mutex<manager::Manager>>,
    preset_type: &manager::PresetType,
    listbox: &gtk::ListBox,
) {
    // let children = listbox.get_children();

    // for child in children {
    //     listbox.remove(&child);
    // }

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

        // listbox.add(&row);
        // listbox.show_all();
    }
}
