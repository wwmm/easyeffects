/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
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

  output_listview = builder->get_widget<Gtk::ListView>("output_listview");
  output_scrolled_window = builder->get_widget<Gtk::ScrolledWindow>("output_scrolled_window");
  output_name = builder->get_widget<Gtk::Entry>("output_name");
  add_output = builder->get_widget<Gtk::Button>("add_output");
  import_output = builder->get_widget<Gtk::Button>("import_output");

  input_listview = builder->get_widget<Gtk::ListView>("input_listview");
  input_scrolled_window = builder->get_widget<Gtk::ScrolledWindow>("input_scrolled_window");
  input_name = builder->get_widget<Gtk::Entry>("input_name");
  add_input = builder->get_widget<Gtk::Button>("add_input");
  import_input = builder->get_widget<Gtk::Button>("import_input");

  // widgets configuration

  setup_listview(output_listview, PresetType::output, output_string_list);
  setup_listview(input_listview, PresetType::input, input_string_list);

  // setting the maximum menu size

  // auto* parent = dynamic_cast<Gtk::ApplicationWindow*>(app->get_active_window());
  // const float scaling_factor = 0.7F;

  // int height = static_cast<int>(scaling_factor * static_cast<float>(parent->get_allocated_height()));

  // output_scrolled_window->set_max_content_height(height);

  // signals connection

  add_output->signal_clicked().connect([=]() { create_preset(PresetType::output); });

  add_input->signal_clicked().connect([=]() { create_preset(PresetType::input); });

  import_output->signal_clicked().connect([=]() { import_preset(PresetType::output); });

  import_input->signal_clicked().connect([=]() { import_preset(PresetType::input); });

  app->presets_manager->user_output_preset_created.connect([=](const Glib::RefPtr<Gio::File>& file) {
    output_string_list->append(util::remove_filename_extension(file->get_basename()));
  });

  app->presets_manager->user_output_preset_removed.connect([=](const Glib::RefPtr<Gio::File>& file) {
    int count = 0;
    auto name = output_string_list->get_string(count);

    while (name.c_str() != nullptr) {
      if (util::remove_filename_extension(file->get_basename()) == std::string(name)) {
        output_string_list->remove(count);
        return;
      }

      count++;

      name = output_string_list->get_string(count);
    }
  });

  app->presets_manager->user_input_preset_created.connect([=](const Glib::RefPtr<Gio::File>& file) {
    input_string_list->append(util::remove_filename_extension(file->get_basename()));
  });

  app->presets_manager->user_input_preset_removed.connect([=](const Glib::RefPtr<Gio::File>& file) {
    int count = 0;
    auto name = input_string_list->get_string(count);

    while (name.c_str() != nullptr) {
      if (util::remove_filename_extension(file->get_basename()) == std::string(name)) {
        input_string_list->remove(count);
        return;
      }

      count++;

      name = input_string_list->get_string(count);
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
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/presets_menu.ui");

  auto settings = Gio::Settings::create("com.github.wwmm.pulseeffects");

  return builder->get_widget_derived<PresetsMenuUi>(builder, "PresetsMenuUi", settings, app);
}

void PresetsMenuUi::create_preset(PresetType preset_type) {
  std::string name = (preset_type == PresetType::output) ? output_name->get_text() : input_name->get_text();

  if (!name.empty()) {
    std::string illegalChars = "\\/";

    for (auto it = name.begin(); it < name.end(); ++it) {
      bool found = illegalChars.find(*it) != std::string::npos;

      if (found) {
        switch (preset_type) {
          case PresetType::output:
            output_name->set_text("");
            break;
          case PresetType::input:
            input_name->set_text("");
            break;
        }

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

  dialog->signal_response().connect([=](auto response_id) {
    switch (response_id) {
      case Gtk::ResponseType::ACCEPT: {
        auto f = dialog->get_file();

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

  listview->set_model(Gtk::NoSelection::create(string_list));

  auto factory = Gtk::SignalListItemFactory::create();

  listview->set_factory(factory);

  factory->signal_setup().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/preset_row.ui");

    auto* top_box = b->get_widget<Gtk::Box>("top_box");

    list_item->set_child(*top_box);
  });

  factory->signal_bind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    if (auto* label = dynamic_cast<Gtk::Label*>(list_item->get_child()->get_first_child())) {
      if (auto* apply = dynamic_cast<Gtk::Button*>(label->get_next_sibling())) {
        if (auto* save = dynamic_cast<Gtk::Button*>(apply->get_next_sibling())) {
          if (auto* autoload = dynamic_cast<Gtk::ToggleButton*>(save->get_next_sibling())) {
            if (auto* remove = dynamic_cast<Gtk::Button*>(autoload->get_next_sibling())) {
              auto name = list_item->get_item()->get_property<Glib::ustring>("string");

              label->set_text(name);

              if (is_autoloaded(preset_type, name)) {
                autoload->set_active(true);
              }

              apply->signal_clicked().connect([=]() {
                switch (preset_type) {
                  case PresetType::input:
                    settings->set_string("last-used-input-preset", name);
                    break;
                  case PresetType::output:
                    settings->set_string("last-used-output-preset", name);
                    break;
                }

                app->presets_manager->load(preset_type, name);
              });

              save->signal_clicked().connect([=]() { app->presets_manager->save(preset_type, name); });

              autoload->signal_toggled().connect([=]() {
                switch (preset_type) {
                  case PresetType::output: {
                    auto dev_name = app->pm->default_sink.name;

                    if (autoload->get_active()) {
                      app->presets_manager->add_autoload(dev_name, name);
                    } else {
                      app->presets_manager->remove_autoload(dev_name, name);
                    }

                    break;
                  }
                  case PresetType::input: {
                    auto dev_name = app->pm->default_source.name;

                    if (autoload->get_active()) {
                      app->presets_manager->add_autoload(dev_name, name);
                    } else {
                      app->presets_manager->remove_autoload(dev_name, name);
                    }

                    break;
                  }
                }
              });

              remove->signal_clicked().connect([=]() { app->presets_manager->remove(preset_type, name); });
            }
          }
        }
      }
    }
  });

  factory->signal_unbind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    if (auto* label = dynamic_cast<Gtk::Label*>(list_item->get_child()->get_first_child())) {
      if (auto* apply = dynamic_cast<Gtk::Button*>(label->get_next_sibling())) {
        if (auto* save = dynamic_cast<Gtk::Button*>(apply->get_next_sibling())) {
          if (auto* autoload = dynamic_cast<Gtk::ToggleButton*>(save->get_next_sibling())) {
            if (auto* remove = dynamic_cast<Gtk::Button*>(autoload->get_next_sibling())) {
              util::warning("unbind");
            }
          }
        }
      }
    }
  });
}

void PresetsMenuUi::reset_menu_button_label() {
  auto names_input = app->presets_manager->get_names(PresetType::input);
  auto names_output = app->presets_manager->get_names(PresetType::output);

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

auto PresetsMenuUi::is_autoloaded(PresetType preset_type, const std::string& name) -> bool {
  std::string current_autoload;

  switch (preset_type) {
    case PresetType::output: {
      auto dev_name = app->pm->default_sink.name;

      current_autoload = app->presets_manager->find_autoload(dev_name);

      break;
    }
    case PresetType::input: {
      auto dev_name = app->pm->default_source.name;

      current_autoload = app->presets_manager->find_autoload(dev_name);

      break;
    }
  }

  return current_autoload == name;
}
