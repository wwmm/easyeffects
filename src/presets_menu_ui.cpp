#include "presets_menu_ui.hpp"
#include <glibmm/i18n.h>
#include <gtkmm/applicationwindow.h>
#include "util.hpp"

PresetsMenuUi::PresetsMenuUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& builder,
                             const Glib::RefPtr<Gio::Settings>& refSettings,
                             Application* application)
    : Gtk::Grid(cobject), settings(refSettings), app(application) {
  // loading glade widgets

  builder->get_widget("presets_listbox", presets_listbox);
  builder->get_widget("presets_scrolled_window", presets_scrolled_window);
  builder->get_widget("preset_name", preset_name);
  builder->get_widget("add_preset", add_preset);
  builder->get_widget("import_preset", import_preset);

  // signals connection

  presets_listbox->set_sort_func(
      sigc::mem_fun(*this, &PresetsMenuUi::on_listbox_sort));

  add_preset->signal_clicked().connect([=]() {
    auto name = preset_name->get_text();
    if (!name.empty()) {
      std::string illegalChars = "\\/\0";

      for (auto it = name.begin(); it < name.end(); ++it) {
        bool found = illegalChars.find(*it) != std::string::npos;
        if (found) {
          preset_name->set_text("");
          return;
        }
      }

      app->presets_manager->add(name);
      preset_name->set_text("");
      populate_presets_listbox();
    }
  });

  import_preset->signal_clicked().connect(
      sigc::mem_fun(*this, &PresetsMenuUi::on_import_preset_clicked));
}

PresetsMenuUi::~PresetsMenuUi() {
  for (auto c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

PresetsMenuUi* PresetsMenuUi::add_to_popover(Gtk::Popover* popover,
                                             Application* app) {
  auto builder = Gtk::Builder::create_from_resource(
      "/com/github/wwmm/pulseeffects/ui/presets_menu.glade");

  auto settings = Gio::Settings::create("com.github.wwmm.pulseeffects");

  PresetsMenuUi* ui;

  builder->get_widget_derived("widgets_grid", ui, settings, app);

  popover->add(*ui);

  return ui;
}

int PresetsMenuUi::on_listbox_sort(Gtk::ListBoxRow* row1,
                                   Gtk::ListBoxRow* row2) {
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

void PresetsMenuUi::on_presets_menu_button_clicked() {
  Gtk::ApplicationWindow* parent =
      dynamic_cast<Gtk::ApplicationWindow*>(this->get_toplevel());

  int height = 0.7 * parent->get_allocated_height();

  presets_scrolled_window->set_max_content_height(height);

  populate_presets_listbox();
}

void PresetsMenuUi::on_import_preset_clicked() {
  // gtkmm 3.22 does not have FileChooseNative so we have to use C api :-(

  gint res;

  auto main_window = gtk_widget_get_toplevel((GtkWidget*)this->gobj());

  auto dialog = gtk_file_chooser_native_new(
      _("Import Presets"), (GtkWindow*)main_window,
      GTK_FILE_CHOOSER_ACTION_OPEN, _("Open"), _("Cancel"));

  auto filter = gtk_file_filter_new();

  gtk_file_filter_set_name(filter, _("Presets"));
  gtk_file_filter_add_pattern(filter, "*.json");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

  res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(dialog));

  if (res == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser* chooser = GTK_FILE_CHOOSER(dialog);

    auto file_list = gtk_file_chooser_get_filenames(chooser);

    g_slist_foreach(file_list,
                    [](auto data, auto user_data) {
                      auto aui = static_cast<PresetsMenuUi*>(user_data);

                      auto file_path = static_cast<char*>(data);

                      aui->app->presets_manager->import(file_path);
                    },
                    this);

    g_slist_free(file_list);
  }

  g_object_unref(dialog);

  populate_presets_listbox();
}

void PresetsMenuUi::populate_presets_listbox() {
  auto children = presets_listbox->get_children();

  for (auto c : children) {
    presets_listbox->remove(*c);
  }

  bool reset_menu_button_label = true;

  auto names = app->presets_manager->get_names();

  for (auto name : names) {
    auto b = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/ui/preset_row.glade");

    Gtk::ListBoxRow* row;
    Gtk::Button *apply_btn, *save_btn, *remove_btn;
    Gtk::Label* label;

    b->get_widget("preset_row", row);
    b->get_widget("apply", apply_btn);
    b->get_widget("save", save_btn);
    b->get_widget("remove", remove_btn);
    b->get_widget("name", label);

    row->set_name(name);

    label->set_text(name);

    connections.push_back(apply_btn->signal_clicked().connect([=]() {
      settings->set_string("last-used-preset", row->get_name());
      app->presets_manager->load(row->get_name());
    }));

    connections.push_back(save_btn->signal_clicked().connect(
        [=]() { app->presets_manager->save(name); }));

    connections.push_back(remove_btn->signal_clicked().connect([=]() {
      app->presets_manager->remove(name);
      populate_presets_listbox();
    }));

    presets_listbox->add(*row);
    presets_listbox->show_all();

    /*if the preset with the name in the button label still exists we do
    not reset the label to "Presets"
    */

    if (name == settings->get_string("last-used-preset")) {
      reset_menu_button_label = false;
    }
  }

  if (reset_menu_button_label) {
    settings->set_string("last-used-preset", _("Presets"));
  }
}
