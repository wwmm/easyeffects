/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "presets_menu_ui.hpp"

PresetsMenuUi::PresetsMenuUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& builder,
                             Glib::RefPtr<Gio::Settings> refSettings,
                             Application* application)
    : Gtk::Popover(cobject),
      settings(std::move(refSettings)),
      app(application),
      output_string_list(Gtk::StringList::create({"initial_value"})),
      input_string_list(Gtk::StringList::create({"initial_value"})) {
  // loading builder widgets

  stack = builder->get_widget<Gtk::Stack>("stack");

  toggle_output = builder->get_widget<Gtk::ToggleButton>("toggle_output");
  toggle_input = builder->get_widget<Gtk::ToggleButton>("toggle_input");

  output_listview = builder->get_widget<Gtk::ListView>("output_listview");
  output_scrolled_window = builder->get_widget<Gtk::ScrolledWindow>("output_scrolled_window");
  output_name = builder->get_widget<Gtk::Text>("output_name");
  add_output = builder->get_widget<Gtk::Button>("add_output");
  import_output = builder->get_widget<Gtk::Button>("import_output");
  output_search = builder->get_widget<Gtk::SearchEntry>("output_search");
  last_used_output = builder->get_widget<Gtk::Label>("last_used_output");

  input_listview = builder->get_widget<Gtk::ListView>("input_listview");
  input_scrolled_window = builder->get_widget<Gtk::ScrolledWindow>("input_scrolled_window");
  input_name = builder->get_widget<Gtk::Text>("input_name");
  add_input = builder->get_widget<Gtk::Button>("add_input");
  import_input = builder->get_widget<Gtk::Button>("import_input");
  input_search = builder->get_widget<Gtk::SearchEntry>("input_search");
  last_used_input = builder->get_widget<Gtk::Label>("last_used_input");

  // widgets configuration

  setup_listview(output_listview, PresetType::output, output_string_list);
  setup_listview(input_listview, PresetType::input, input_string_list);

  last_used_output->set_label(settings->get_string("last-used-output-preset"));
  last_used_input->set_label(settings->get_string("last-used-input-preset"));

  // signals connection

  stack_model = stack->get_pages();

  toggle_output->signal_toggled().connect([&, this]() {
    if (toggle_output->get_active()) {
      stack->get_pages()->select_item(0, true);
    }
  });

  toggle_input->signal_toggled().connect([&, this]() {
    if (toggle_input->get_active()) {
      stack->get_pages()->select_item(1, true);
    }
  });

  add_output->signal_clicked().connect([=, this]() { create_preset(PresetType::output); });

  add_input->signal_clicked().connect([=, this]() { create_preset(PresetType::input); });

  import_output->signal_clicked().connect([=, this]() { import_preset(PresetType::output); });

  import_input->signal_clicked().connect([=, this]() { import_preset(PresetType::input); });

  settings->signal_changed("last-used-output-preset").connect([=, this](const auto& key) {
    last_used_output->set_label(settings->get_string("last-used-output-preset"));
  });

  settings->signal_changed("last-used-input-preset").connect([=, this](const auto& key) {
    last_used_input->set_label(settings->get_string("last-used-input-preset"));
  });

  app->presets_manager->user_output_preset_created.connect([=, this](const Glib::RefPtr<Gio::File>& file) {
    output_string_list->append(util::remove_filename_extension(file->get_basename()));
  });

  app->presets_manager->user_output_preset_removed.connect([=, this](const Glib::RefPtr<Gio::File>& file) {
    int count = 0;

    for (auto name = output_string_list->get_string(count); name.c_str() != nullptr;) {
      if (util::remove_filename_extension(file->get_basename()) == std::string(name)) {
        output_string_list->remove(count);
        return;
      }

      name = output_string_list->get_string(++count);
    }
  });

  app->presets_manager->user_input_preset_created.connect([=, this](const Glib::RefPtr<Gio::File>& file) {
    input_string_list->append(util::remove_filename_extension(file->get_basename()));
  });

  app->presets_manager->user_input_preset_removed.connect([=, this](const Glib::RefPtr<Gio::File>& file) {
    int count = 0;

    for (auto name = input_string_list->get_string(count); name.c_str() != nullptr;) {
      if (util::remove_filename_extension(file->get_basename()) == std::string(name)) {
        input_string_list->remove(count);
        return;
      }

      name = input_string_list->get_string(++count);
    }
  });

  reset_menu_button_label();

  reference();
}

PresetsMenuUi::~PresetsMenuUi() {
  for (auto& c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

auto PresetsMenuUi::create(Application* app) -> PresetsMenuUi* {
  const auto& builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/presets_menu.ui");

  const auto& settings = Gio::Settings::create("com.github.wwmm.easyeffects");

  return Gtk::Builder::get_widget_derived<PresetsMenuUi>(builder, "PresetsMenuUi", settings, app);
}

void PresetsMenuUi::create_preset(PresetType preset_type) {
  const auto& name = (preset_type == PresetType::output) ? output_name->get_text() : input_name->get_text();

  if (!name.empty()) {
    std::string illegalChars = "\\/";

    for (const auto& c : name) {
      if (illegalChars.find(c) != std::string::npos) {
        switch (preset_type) {
          case PresetType::output:
            output_name->set_text("");
            break;
          case PresetType::input:
            input_name->set_text("");
            break;
        }

        util::debug(log_tag + " name " + name + " has illegal file name characters. Aborting preset creation!");

        return;
      }
    }

    switch (preset_type) {
      case PresetType::output:
        output_name->set_text("");
        break;
      case PresetType::input:
        input_name->set_text("");
        break;
    }

    app->presets_manager->add(preset_type, name);
  }
}

void PresetsMenuUi::import_preset(PresetType preset_type) {
  auto* main_window = dynamic_cast<Gtk::Window*>(app->get_active_window());

  auto dialog = Gtk::FileChooserNative::create(_("Import Preset"), *main_window, Gtk::FileChooser::Action::OPEN,
                                               _("Open"), _("Cancel"));

  auto dialog_filter = Gtk::FileFilter::create();

  dialog_filter->set_name(_("Presets"));
  dialog_filter->add_pattern("*.json");

  dialog->add_filter(dialog_filter);

  dialog->signal_response().connect([=, this](const auto& response_id) {
    switch (response_id) {
      case Gtk::ResponseType::ACCEPT: {
        const auto& f = dialog->get_file();

        app->presets_manager->import(preset_type, f->get_path());

        break;
      }
      default:
        break;
    }
  });

  dialog->set_modal(true);
  dialog->show();
}

void PresetsMenuUi::setup_listview(Gtk::ListView* listview,
                                   PresetType preset_type,
                                   Glib::RefPtr<Gtk::StringList>& string_list) {
  string_list->remove(0);

  auto names = app->presets_manager->get_names(preset_type);

  for (const auto& name : names) {
    string_list->append(name);
  }

  // filter

  auto filter =
      Gtk::StringFilter::create(Gtk::PropertyExpression<Glib::ustring>::create(GTK_TYPE_STRING_OBJECT, "string"));

  auto filter_model = Gtk::FilterListModel::create(string_list, filter);

  filter_model->set_incremental(true);

  switch (preset_type) {
    case PresetType::output: {
      Glib::Binding::bind_property(output_search->property_text(), filter->property_search());
      break;
    }
    case PresetType::input: {
      Glib::Binding::bind_property(input_search->property_text(), filter->property_search());
      break;
    }
  }

  // sorter

  auto sorter =
      Gtk::StringSorter::create(Gtk::PropertyExpression<Glib::ustring>::create(GTK_TYPE_STRING_OBJECT, "string"));

  auto sort_list_model = Gtk::SortListModel::create(filter_model, sorter);

  // setting the listview model and factory

  listview->set_model(Gtk::NoSelection::create(sort_list_model));

  auto factory = Gtk::SignalListItemFactory::create();

  listview->set_factory(factory);

  // setting the factory callbacks

  factory->signal_setup().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    const auto& b = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/preset_row.ui");

    auto* top_box = b->get_widget<Gtk::Box>("top_box");

    list_item->set_data("name", b->get_widget<Gtk::Label>("name"));
    list_item->set_data("apply", b->get_widget<Gtk::Button>("apply"));
    list_item->set_data("save", b->get_widget<Gtk::Button>("save"));
    list_item->set_data("remove", b->get_widget<Gtk::Button>("remove"));

    list_item->set_child(*top_box);
  });

  factory->signal_bind().connect([=, this](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* label = static_cast<Gtk::Label*>(list_item->get_data("name"));
    auto* apply = static_cast<Gtk::Button*>(list_item->get_data("apply"));
    auto* save = static_cast<Gtk::Button*>(list_item->get_data("save"));
    auto* remove = static_cast<Gtk::Button*>(list_item->get_data("remove"));

    const auto& name = list_item->get_item()->get_property<Glib::ustring>("string");

    label->set_text(name);

    auto connection_apply = apply->signal_clicked().connect([=, this]() {
      switch (preset_type) {
        case PresetType::input:
          settings->set_string("last-used-input-preset", name);
          break;
        case PresetType::output:
          settings->set_string("last-used-output-preset", name);
          break;
      }

      app->presets_manager->load_preset_file(preset_type, name);
    });

    auto connection_save =
        save->signal_clicked().connect([=, this]() { app->presets_manager->save_preset_file(preset_type, name); });

    auto connection_remove =
        remove->signal_clicked().connect([=, this]() { app->presets_manager->remove(preset_type, name); });

    list_item->set_data("connection_apply", new sigc::connection(connection_apply),
                        Glib::destroy_notify_delete<sigc::connection>);

    list_item->set_data("connection_save", new sigc::connection(connection_save),
                        Glib::destroy_notify_delete<sigc::connection>);

    list_item->set_data("connection_remove", new sigc::connection(connection_remove),
                        Glib::destroy_notify_delete<sigc::connection>);
  });

  factory->signal_unbind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    for (const auto* conn : {"connection_apply", "connection_save", "connection_remove"}) {
      if (auto* connection = static_cast<sigc::connection*>(list_item->get_data(conn))) {
        connection->disconnect();

        list_item->set_data(conn, nullptr);
      }
    }
  });
}

void PresetsMenuUi::reset_menu_button_label() {
  const auto& names_input = app->presets_manager->get_names(PresetType::input);
  const auto& names_output = app->presets_manager->get_names(PresetType::output);

  if (names_input.empty() && names_output.empty()) {
    settings->set_string("last-used-output-preset", _("Presets"));
    settings->set_string("last-used-input-preset", _("Presets"));

    return;
  }

  for (const auto& name : names_input) {
    if (name == settings->get_string("last-used-input-preset")) {
      return;
    }
  }

  for (const auto& name : names_output) {
    if (name == settings->get_string("last-used-output-preset")) {
      return;
    }
  }

  settings->set_string("last-used-output-preset", _("Presets"));
  settings->set_string("last-used-input-preset", _("Presets"));
}

void PresetsMenuUi::on_show() {
  auto* parent = dynamic_cast<Gtk::ApplicationWindow*>(app->get_active_window());

  int height = static_cast<int>(0.5F * static_cast<float>(parent->get_allocated_height()));

  output_scrolled_window->set_max_content_height(height);
  input_scrolled_window->set_max_content_height(height);

  Gtk::Popover::on_show();
}
