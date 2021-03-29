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
    : Gtk::Box(cobject),
      PluginUiBase(builder, schema, schema_path),
      model_dir(Glib::get_user_config_dir() + "/PulseEffects/rnnoise") {
  name = plugin_name::rnnoise;

  default_model_name = _("Standard RNNoise Model");

  // loading builder widgets

  input_gain = builder->get_widget<Gtk::Scale>("input_gain");
  output_gain = builder->get_widget<Gtk::Scale>("output_gain");

  model_list_frame = builder->get_widget<Gtk::Frame>("model_list_frame");

  import_model = builder->get_widget<Gtk::Button>("import_model");

  active_model_name = builder->get_widget<Gtk::Label>("active_model_name");

  // signals connection

  import_model->signal_clicked().connect(sigc::mem_fun(*this, &RNNoiseUi::on_import_model_clicked));

  // gsettings bindings

  settings->bind("input-gain", input_gain->get_adjustment().get(), "value");
  settings->bind("output-gain", output_gain->get_adjustment().get(), "value");

  connections.emplace_back(
      settings->signal_changed("model-path").connect([=, this](auto key) { set_active_model_label(); }));

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

  set_active_model_label();
}

RNNoiseUi::~RNNoiseUi() {
  util::debug(name + " ui destroyed");
}

auto RNNoiseUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> RNNoiseUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/rnnoise.ui");

  auto* ui = Gtk::Builder::get_widget_derived<RNNoiseUi>(builder, "top_box", "com.github.wwmm.pulseeffects.rnnoise",
                                                         schema_path + "rnnoise/");

  auto stack_page = stack->add(*ui, plugin_name::rnnoise);

  return ui;
}

void RNNoiseUi::on_import_model_clicked() {
  auto dialog =
      Gtk::FileChooserNative::create(_("Import Model File"), Gtk::FileChooser::Action::OPEN, _("Open"), _("Cancel"));

  auto dialog_filter = Gtk::FileFilter::create();

  dialog_filter->set_name(_("RNNoise Models"));
  dialog_filter->add_pattern("*.rnnn");

  dialog->add_filter(dialog_filter);

  dialog->signal_response().connect([=, this](auto response_id) {
    switch (response_id) {
      case Gtk::ResponseType::ACCEPT: {
        import_model_file(dialog->get_file()->get_path());

        // populate_model_listbox();

        break;
      }
      default:
        break;
    }
  });

  dialog->set_modal(true);
  dialog->show();
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

// void RNNoiseUi::populate_model_listbox() {
//   auto names = get_model_names();

//   if (names.empty()) {
//     settings->set_string("model-path", default_model_name);
//     model_list_frame->set_visible(false);
//   } else {
//     model_list_frame->set_visible(true);
//   }

//   for (const auto& name : names) {
//     auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/irs_row.glade");

//     Gtk::ListBoxRow* row = nullptr;
//     Gtk::Button* remove_btn = nullptr;
//     Gtk::Button* apply_btn = nullptr;
//     Gtk::Label* label = nullptr;

//     b->get_widget("irs_row", row);
//     b->get_widget("remove", remove_btn);
//     b->get_widget("apply", apply_btn);
//     b->get_widget("name", label);

//     row->set_name(name);
//     label->set_text(name);

//     connections.emplace_back(remove_btn->signal_clicked().connect([=]() {
//       remove_model_file(name);
//       populate_model_listbox();
//     }));

//     connections.emplace_back(apply_btn->signal_clicked().connect([=]() {
//       auto model_file = model_dir / std::filesystem::path{row->get_name() + ".rnnn"};

//       settings->set_string("model-path", model_file.string());
//     }));
//   }
// }

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
