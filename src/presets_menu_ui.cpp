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
    : Gtk::Popover(cobject), settings(std::move(refSettings)), app(application) {
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

  reset_menu_button_label();
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

    populate_listbox(preset_type);
  }
}

void PresetsMenuUi::import_preset(PresetType preset_type) {
  auto* main_window = dynamic_cast<Gtk::Window*>(app->get_active_window());

  auto dialog = Gtk::FileChooserNative::create(_("Import Presets"), *main_window, Gtk::FileChooser::Action::OPEN,
                                               _("Open"), _("Cancel"));

  auto dialog_filter = Gtk::FileFilter::create();

  dialog_filter->set_name(_("Presets"));
  dialog_filter->add_pattern("*.json");

  dialog->add_filter(dialog_filter);

  dialog->signal_response().connect([=](auto response_id) {
    switch (response_id) {
      case Gtk::ResponseType::ACCEPT: {
        auto* model = dialog->get_files().get();

        for (guint n = 0; n < model->get_n_items(); n++) {
          auto f = std::dynamic_pointer_cast<Gio::File>(model->get_object(n));

          util::warning(f->get_path());

          // app->presets_manager->import(preset_type, file_path);
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

void PresetsMenuUi::populate_listbox(PresetType preset_type) {
  Gtk::ListView* listview = (preset_type == PresetType::output) ? output_listview : input_listview;

  auto names = app->presets_manager->get_names(preset_type);

  for (const auto& name : names) {
    auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/preset_row.glade");

    Gtk::ListBoxRow* row = nullptr;
    Gtk::Button* apply_btn = nullptr;
    Gtk::Button* save_btn = nullptr;
    Gtk::Button* remove_btn = nullptr;
    Gtk::Label* label = nullptr;
    Gtk::ToggleButton* autoload_btn = nullptr;

    // b->get_widget("preset_row", row);
    // b->get_widget("apply", apply_btn);
    // b->get_widget("save", save_btn);
    // b->get_widget("remove", remove_btn);
    // b->get_widget("name", label);
    // b->get_widget("autoload", autoload_btn);

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
      switch (preset_type) {
        case PresetType::output: {
          auto dev_name = app->pm->default_sink.name;

          if (autoload_btn->get_active()) {
            app->presets_manager->add_autoload(dev_name, name);
          } else {
            app->presets_manager->remove_autoload(dev_name, name);
          }

          break;
        }
        case PresetType::input: {
          auto dev_name = app->pm->default_source.name;

          if (autoload_btn->get_active()) {
            app->presets_manager->add_autoload(dev_name, name);
          } else {
            app->presets_manager->remove_autoload(dev_name, name);
          }

          break;
        }
      }

      populate_listbox(preset_type);
    }));

    connections.emplace_back(remove_btn->signal_clicked().connect([=]() {
      app->presets_manager->remove(preset_type, name);

      populate_listbox(preset_type);
    }));
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
