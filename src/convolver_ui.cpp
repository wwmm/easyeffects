#include <glibmm/i18n.h>
#include "convolver_ui.hpp"

ConvolverUi::ConvolverUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
    name = "convolver";

    // loading glade widgets

    builder->get_widget("irs_listbox", irs_listbox);
    builder->get_widget("irs_menu_button", irs_menu_button);
    builder->get_widget("irs_scrolled_window", irs_scrolled_window);
    builder->get_widget("import_irs", import_irs);

    get_object(builder, "input_gain", input_gain);
    get_object(builder, "output_gain", output_gain);

    irs_menu_button->signal_clicked().connect(
        sigc::mem_fun(*this, &ConvolverUi::on_irs_menu_button_clicked));

    irs_listbox->set_sort_func(
        sigc::mem_fun(*this, &ConvolverUi::on_listbox_sort));

    irs_listbox->signal_row_activated().connect([&](auto row) {

    });

    import_irs->signal_clicked().connect(
        sigc::mem_fun(*this, &ConvolverUi::on_import_irs_clicked));

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    settings->bind("installed", this, "sensitive", flag);
    settings->bind("input-gain", input_gain.get(), "value", flag);
    settings->bind("output-gain", output_gain.get(), "value", flag);

    // g_settings_bind_with_mapping(settings->gobj(), "mode", mode->gobj(),
    //                              "active", G_SETTINGS_BIND_DEFAULT,
    //                              convolver_enum_to_int,
    //                              int_to_convolver_enum, nullptr, nullptr);

    settings->set_boolean("post-messages", true);
}

ConvolverUi::~ConvolverUi() {
    settings->set_boolean("post-messages", false);

    util::debug(name + " ui destroyed");
}

int ConvolverUi::on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) {
    auto name1 = row1->get_name();
    auto name2 = row2->get_name();

    std::vector<std::string> names = {name1, name2};

    std::sort(names.begin(), names.end());

    if (name1 == names[0]) {
        return -1;
    } else if (name2 == names[0]) {
        return 1;
    } else {
        return 0;
    }
}

void ConvolverUi::on_irs_menu_button_clicked() {
    int height = 0.7 * get_allocated_height();

    irs_scrolled_window->set_max_content_height(height);

    // populate_irs_listbox();
}

void ConvolverUi::on_import_irs_clicked() {
    // gtkmm 3.22 does not have FileChooseNative so we have to use C api :-(

    GtkFileChooserNative* native;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    native = gtk_file_chooser_native_new(_("Import Presets"),
                                         (GtkWindow*)this->gobj(), action,
                                         _("Open"), _("Cancel"));

    res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(native));

    if (res == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser* chooser = GTK_FILE_CHOOSER(native);

        auto file_list = gtk_file_chooser_get_filenames(chooser);

        g_slist_foreach(file_list,
                        [](auto data, auto user_data) {
                            // auto cui = static_cast<ConvolverUi*>(user_data);

                            // auto file_path = static_cast<char*>(data);

                            // aui->app->irs_manager->import(file_path);
                        },
                        this);

        g_slist_free(file_list);
    }

    g_object_unref(native);

    // populate_irs_listbox();
}

void ConvolverUi::reset() {
    settings->reset("state");
    settings->reset("input-gain");
    settings->reset("output-gain");
}
