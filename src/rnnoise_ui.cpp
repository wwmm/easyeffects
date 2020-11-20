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

#include "rnnoise_ui.hpp"

RNNoiseUi::RNNoiseUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& builder,
                     const std::string& schema,
                     const std::string& schema_path)
    : Gtk::Grid(cobject),
      PluginUiBase(builder, schema, schema_path),
      model_dir(Glib::get_user_config_dir() + "/PulseEffects/rnnoise") {
  name = "rnnoise";
  default_model_name = _("Standard RNNoise Model");

  // loading glade widgets

  builder->get_widget("plugin_reset", reset_button);
  builder->get_widget("import_model", import_model);
  builder->get_widget("model_listbox", model_listbox);
  builder->get_widget("active_model_name", active_model_name);

  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);

  // signals connection

  model_listbox->set_sort_func(sigc::ptr_fun(&RNNoiseUi::on_listbox_sort));

  import_model->signal_clicked().connect(sigc::mem_fun(*this, &RNNoiseUi::on_import_model_clicked));

  reset_button->signal_clicked().connect([=]() { reset(); });

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);

  connections.emplace_back(settings->signal_changed("model-path").connect([=](auto key) { set_active_model_label(); }));

  // model dir

  if (!std::filesystem::is_directory(model_dir)) {
    if (std::filesystem::create_directories(model_dir)) {
      util::debug(log_tag + "model directory created: " + model_dir.string());
    } else {
      util::warning(log_tag + "failed to create model directory: " + model_dir.string());
    }
  } else {
    util::debug(log_tag + "model directory already exists: " + model_dir.string());
  }

  populate_model_listbox();

  set_active_model_label();
}

RNNoiseUi::~RNNoiseUi() {
  util::debug(name + " ui destroyed");
}

void RNNoiseUi::on_import_model_clicked() {
  auto* main_window = dynamic_cast<Gtk::Window*>(this->get_toplevel());

  auto dialog = Gtk::FileChooserNative::create(
      _("Import Model File"), *main_window, Gtk::FileChooserAction::FILE_CHOOSER_ACTION_OPEN, _("Open"), _("Cancel"));

  auto dialog_filter = Gtk::FileFilter::create();

  dialog_filter->set_name(_("RNNoise Models"));
  dialog_filter->add_pattern("*.rnnn");

  dialog->add_filter(dialog_filter);

  dialog->signal_response().connect([=](auto response_id) {
    switch (response_id) {
      case Gtk::ResponseType::RESPONSE_ACCEPT: {
        import_model_file(dialog->get_file()->get_path());

        populate_model_listbox();

        break;
      }
      default:
        break;
    }
  });

  dialog->set_modal(true);
  dialog->show();
}

auto RNNoiseUi::on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) -> int {
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

void RNNoiseUi::import_model_file(const std::string& file_path) {
  std::filesystem::path p{file_path};

  if (std::filesystem::is_regular_file(p)) {
    auto out_path = model_dir / p.filename();

    out_path.replace_extension(".rnnn");

    std::filesystem::copy_file(p, out_path, std::filesystem::copy_options::overwrite_existing);

    util::debug(log_tag + "imported model file to: " + out_path.string());
  } else {
    util::warning(log_tag + p.string() + " is not a file!");
  }
}

void RNNoiseUi::populate_model_listbox() {
  auto children = model_listbox->get_children();

  for (const auto& c : children) {
    model_listbox->remove(*c);
  }

  auto names = get_model_names();

  if (names.empty()) {
    settings->set_string("model-path", default_model_name);
  }

  for (const auto& name : names) {
    auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/irs_row.glade");

    Gtk::ListBoxRow* row = nullptr;
    Gtk::Button* remove_btn = nullptr;
    Gtk::Button* apply_btn = nullptr;
    Gtk::Label* label = nullptr;

    b->get_widget("irs_row", row);
    b->get_widget("remove", remove_btn);
    b->get_widget("apply", apply_btn);
    b->get_widget("name", label);

    row->set_name(name);
    label->set_text(name);

    connections.emplace_back(remove_btn->signal_clicked().connect([=]() {
      remove_model_file(name);
      populate_model_listbox();
    }));

    connections.emplace_back(apply_btn->signal_clicked().connect([=]() {
      auto model_file = model_dir / std::filesystem::path{row->get_name() + ".rnnn"};

      settings->set_string("model-path", model_file.string());
    }));

    model_listbox->add(*row);
    model_listbox->show_all();
  }
}

auto RNNoiseUi::get_model_names() -> std::vector<std::string> {
  std::filesystem::directory_iterator it{model_dir};
  std::vector<std::string> names;

  while (it != std::filesystem::directory_iterator{}) {
    if (std::filesystem::is_regular_file(it->status())) {
      if (it->path().extension().string() == ".rnnn") {
        names.emplace_back(it->path().stem().string());
      }
    }

    it++;
  }

  return names;
}

void RNNoiseUi::remove_model_file(const std::string& name) {
  auto model_file = model_dir / std::filesystem::path{name + ".rnnn"};

  if (std::filesystem::exists(model_file)) {
    std::filesystem::remove(model_file);

    util::debug(log_tag + "removed model file: " + model_file.string());
  }
}

void RNNoiseUi::set_active_model_label() {
  auto path = std::filesystem::path{settings->get_string("model-path")};

  if (settings->get_string("model-path").empty()) {
    active_model_name->set_text(default_model_name);
  } else {
    active_model_name->set_text(path.stem().string());
  }
}

void RNNoiseUi::reset() {
  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("model-path");
}
