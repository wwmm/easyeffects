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
#include <glibmm/i18n.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/dialog.h>
#include <gtkmm/filechoosernative.h>
#include <gtkmm/togglebutton.h>
#include "preset_type.hpp"
#include "util.hpp"

PresetsMenuUi::PresetsMenuUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& builder,
                             Glib::RefPtr<Gio::Settings> refSettings,
                             Application* application)
    : Gtk::Grid(cobject), settings(std::move(refSettings)), app(application) {
  // loading glade widgets

  builder->get_widget("output_listbox", output_listbox);
  builder->get_widget("output_scrolled_window", output_scrolled_window);
  builder->get_widget("output_name", output_name);
  builder->get_widget("add_output", add_output);
  builder->get_widget("import_output", import_output);
  builder->get_widget("input_listbox", input_listbox);
  builder->get_widget("input_scrolled_window", input_scrolled_window);
  builder->get_widget("input_name", input_name);
  builder->get_widget("add_input", add_input);
  builder->get_widget("import_input", import_input);

  // signals connection

  output_listbox->set_sort_func(sigc::ptr_fun(&PresetsMenuUi::on_listbox_sort));

  input_listbox->set_sort_func(sigc::ptr_fun(&PresetsMenuUi::on_listbox_sort));

  add_output->signal_clicked().connect([=]() { create_preset(PresetType::output); });

  add_input->signal_clicked().connect([=]() { create_preset(PresetType::input); });

  import_output->signal_clicked().connect([=]() { import_preset(PresetType::output); });

  import_input->signal_clicked().connect([=]() { import_preset(PresetType::input); });

  reset_menu_button_label();
}

PresetsMenuUi::~PresetsMenuUi() {
  for (auto& c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

auto PresetsMenuUi::add_to_popover(Gtk::Popover* popover, Application* app) -> PresetsMenuUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/presets_menu.glade");

  auto settings = Gio::Settings::create("com.github.wwmm.pulseeffects");

  PresetsMenuUi* ui = nullptr;

  builder->get_widget_derived("widgets_grid", ui, settings, app);

  popover->add(*ui);

  return ui;
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
        // app->presets_manager->add(name);
        input_name->set_text("");
        break;
    }

    app->presets_manager->add(preset_type, name);

    populate_listbox(preset_type);
  }
}

void PresetsMenuUi::import_preset(PresetType preset_type) {
  auto* main_window = dynamic_cast<Gtk::Window*>(this->get_toplevel());

  auto dialog = Gtk::FileChooserNative::create(
      _("Import Presets"), *main_window, Gtk::FileChooserAction::FILE_CHOOSER_ACTION_OPEN, _("Open"), _("Cancel"));

  auto dialog_filter = Gtk::FileFilter::create();

  dialog_filter->set_name(_("Presets"));
  dialog_filter->add_pattern("*.json");

  dialog->add_filter(dialog_filter);

  dialog->signal_response().connect([=](auto response_id) {
    switch (response_id) {
      case Gtk::ResponseType::RESPONSE_ACCEPT: {
        for (const auto& file_path : dialog->get_filenames()) {
          app->presets_manager->import(preset_type, file_path);
        }

        populate_listbox(preset_type);

        break;
      }
      default:
        break;
    }
  });

  dialog->set_modal(true);
  dialog->set_select_multiple(true);
  dialog->show();
}

auto PresetsMenuUi::on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) -> int {
  auto name1 = row1->get_name();
  auto name2 = row2->get_name();

  std::vector<std::string> names = {name1, name2};

  std::sort(names.begin(), names.end());

  if (name1 == names[0]) {
    return -1;
  }
  if (name2 == names[0]) {
    return 1;
  }

  return 0;
}

void PresetsMenuUi::on_presets_menu_button_clicked() {
  auto* parent = dynamic_cast<Gtk::ApplicationWindow*>(this->get_toplevel());
  const float scaling_factor = 0.7F;

  int height = static_cast<int>(scaling_factor * static_cast<float>(parent->get_allocated_height()));

  output_scrolled_window->set_max_content_height(height);

  populate_listbox(PresetType::input);
  populate_listbox(PresetType::output);
}

void PresetsMenuUi::populate_listbox(PresetType preset_type) {
  Gtk::ListBox* listbox = (preset_type == PresetType::output) ? output_listbox : input_listbox;

  auto children = listbox->get_children();

  for (const auto& c : children) {
    listbox->remove(*c);
  }

  auto names = app->presets_manager->get_names(preset_type);

  for (const auto& name : names) {
    auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/preset_row.glade");

    Gtk::ListBoxRow* row = nullptr;
    Gtk::Button* apply_btn = nullptr;
    Gtk::Button* save_btn = nullptr;
    Gtk::Button* remove_btn = nullptr;
    Gtk::Label* label = nullptr;
    Gtk::ToggleButton* autoload_btn = nullptr;

    b->get_widget("preset_row", row);
    b->get_widget("apply", apply_btn);
    b->get_widget("save", save_btn);
    b->get_widget("remove", remove_btn);
    b->get_widget("name", label);
    b->get_widget("autoload", autoload_btn);

    row->set_name(name);
    label->set_text(name);

    if (is_autoloaded(preset_type, name)) {
      autoload_btn->set_active(true);
    }

    connections.emplace_back(apply_btn->signal_clicked().connect([=]() {
      switch (preset_type) {
        case PresetType::input:
          settings->set_string("last-used-input-preset", row->get_name());
          break;
        case PresetType::output:
          settings->set_string("last-used-output-preset", row->get_name());
          break;
      }

      app->presets_manager->load(preset_type, row->get_name());
    }));

    connections.emplace_back(
        save_btn->signal_clicked().connect([=]() { app->presets_manager->save(preset_type, name); }));

    connections.emplace_back(autoload_btn->signal_toggled().connect([=]() {
      //   switch (preset_type) {
      //     case PresetType::output: {
      //       auto dev_name = build_device_name(preset_type, app->pm->server_info.default_sink_name);

      //       if (autoload_btn->get_active()) {
      //         app->presets_manager->add_autoload(dev_name, name);
      //       } else {
      //         app->presets_manager->remove_autoload(dev_name, name);
      //       }

      //       break;
      //     }
      //     case PresetType::input: {
      //       auto dev_name = build_device_name(preset_type, app->pm->server_info.default_source_name);

      //       if (autoload_btn->get_active()) {
      //         app->presets_manager->add_autoload(dev_name, name);
      //       } else {
      //         app->presets_manager->remove_autoload(dev_name, name);
      //       }

      //       break;
      //     }
      //   }

      populate_listbox(preset_type);
    }));

    connections.emplace_back(remove_btn->signal_clicked().connect([=]() {
      app->presets_manager->remove(preset_type, name);

      populate_listbox(preset_type);
    }));

    listbox->add(*row);
    listbox->show_all();
  }
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

auto PresetsMenuUi::build_device_name(PresetType preset_type, const std::string& device) -> std::string {
  std::string port;
  std::string dev_name;

  // switch (preset_type) {
  //   case PresetType::output: {
  //     auto info = app->pm->get_sink_info(device);

  //     port = info->active_port;

  //     break;
  //   }
  //   case PresetType::input: {
  //     auto info = app->pm->get_source_info(device);

  //     port = info->active_port;

  //     break;
  //   }
  // }

  // if (port != "null") {
  //   dev_name = device + ":" + port;
  // } else {
  //   dev_name = device;
  // }

  return dev_name;
}

auto PresetsMenuUi::is_autoloaded(PresetType preset_type, const std::string& name) -> bool {
  std::string current_autoload;

  // switch (preset_type) {
  //   case PresetType::output: {
  //     auto dev_name = build_device_name(preset_type, app->pm->server_info.default_sink_name);

  //     current_autoload = app->presets_manager->find_autoload(dev_name);

  //     break;
  //   }
  //   case PresetType::input: {
  //     auto dev_name = build_device_name(preset_type, app->pm->server_info.default_source_name);

  //     current_autoload = app->presets_manager->find_autoload(dev_name);

  //     break;
  //   }
  // }

  return current_autoload == name;
}
