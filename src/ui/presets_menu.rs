use gtk::prelude::*;

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
    import_input: gtk::Button
}

pub fn build_ui(button: &gtk::Button) -> gtk::Grid {
    let res = WindowResource::load().unwrap();
    let output_scrolled_window = res.output_scrolled_window;

    button.connect_clicked(move |obj|{
        let top_widget = obj.get_toplevel().expect("Could not get presets menu top level widget");
        let height = top_widget.get_allocated_height() as f32;

        output_scrolled_window.set_max_content_height((0.7 * height) as i32);

        // populate_listbox(PresetType::input);
        // populate_listbox(PresetType::output);
    });

    return res.widgets_grid;
}