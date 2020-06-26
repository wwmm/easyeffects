use gtk::prelude::*;

use crate::presets::manager;

use gtk_resources::UIResource;

#[derive(UIResource, Debug)]
#[resource = "/com/github/wwmm/pulseeffects/ui/presets_menu.glade"]
struct WindowResource {
    widgets_grid: gtk::Grid,
    stack: gtk::Stack,
    output_listbox: gtk::ListBox,
    output_scrolled_window: gtk::ScrolledWindow,
    output_name: gtk::Entry,
    add_output: gtk::Button,
    import_output: gtk::Button,
    input_listbox: gtk::ListBox,
    input_scrolled_window: gtk::ScrolledWindow,
    input_name: gtk::Entry,
    add_input: gtk::Button,
    import_input: gtk::Button,
}

pub fn build_ui(button: &gtk::Button) -> gtk::Grid {
    let resources = WindowResource::load().unwrap();

    let output_scrolled_window = resources.output_scrolled_window;
    let input_listbox = resources.input_listbox;
    let output_listbox = resources.output_listbox;

    output_listbox.set_sort_func(Some(Box::new(on_listbox_sort)));
    input_listbox.set_sort_func(Some(Box::new(on_listbox_sort)));

    let presets_manager = manager::Manager::new();

    button.connect_clicked(move |obj| {
        let top_widget = obj
            .get_toplevel()
            .expect("Could not get presets menu top level widget");
        let height = top_widget.get_allocated_height() as f32;

        output_scrolled_window.set_max_content_height((0.7 * height) as i32);

        populate_listbox(&presets_manager, manager::PresetType::Input, &input_listbox);

        populate_listbox(
            &presets_manager,
            manager::PresetType::Output,
            &output_listbox,
        );
    });

    return resources.widgets_grid;
}

fn on_listbox_sort(row1: &gtk::ListBoxRow, row2: &gtk::ListBoxRow) -> i32 {
    let mut names = Vec::new();
    let name1 = row1.get_widget_name().expect("Could not get widget name");
    let name2 = row2.get_widget_name().expect("Could not get widget name");

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
    presets_manager: &manager::Manager,
    preset_type: manager::PresetType,
    listbox: &gtk::ListBox,
) {
    let children = listbox.get_children();

    for child in children {
        listbox.remove(&child);
    }

    let names = presets_manager.get_names(preset_type);
}
