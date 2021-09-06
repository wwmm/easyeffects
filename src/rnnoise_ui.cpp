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

#include "rnnoise_ui.hpp"

RNNoiseUi::RNNoiseUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& builder,
                     const std::string& schema,
                     const std::string& schema_path)
    : Gtk::Box(cobject),
      PluginUiBase(builder, schema, schema_path),
      default_model_name(_("Standard Model")),
      string_list(Gtk::StringList::create({default_model_name})),
      model_dir(Glib::get_user_config_dir() + "/easyeffects/rnnoise") {
  name = plugin_name::rnnoise;

  // loading builder widgets

  model_list_frame = builder->get_widget<Gtk::Frame>("model_list_frame");

  listview = builder->get_widget<Gtk::ListView>("listview");

  import_model = builder->get_widget<Gtk::Button>("import_model");

  active_model_name = builder->get_widget<Gtk::Label>("active_model_name");

  // signals connection

  import_model->signal_clicked().connect(sigc::mem_fun(*this, &RNNoiseUi::on_import_model_clicked));

  // gsettings bindings

  connections.emplace_back(
      settings->signal_changed("model-path").connect([=, this](const auto& key) { set_active_model_label(); }));

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

  setup_input_output_gain(builder);

  setup_listview();

  set_active_model_label();

  folder_monitor = Gio::File::create_for_path(model_dir.string())->monitor_directory();

  folder_monitor->signal_changed().connect(
      [=, this](const Glib::RefPtr<Gio::File>& file, const auto& other_f, const auto& event) {
        const auto& rnn_filename = util::remove_filename_extension(file->get_basename());

        if (rnn_filename.empty()) {
          util::warning("Can't retrieve information about the rnn file");

          return;
        }

        switch (event) {
          case Gio::FileMonitor::Event::CREATED: {
            for (guint n = 0, list_size = string_list->get_n_items(); n < list_size; n++) {
              if (string_list->get_string(n) == rnn_filename) {
                return;
              }
            }

            string_list->append(rnn_filename);

            break;
          }
          case Gio::FileMonitor::Event::DELETED: {
            for (guint n = 0, list_size = string_list->get_n_items(); n < list_size; n++) {
              if (string_list->get_string(n) == rnn_filename) {
                string_list->remove(n);

                // Workaround for GTK not calling the listview signal_selection_changed (issue #1110)

                on_selection_changed();

                break;
              }
            }

            break;
          }
          default:
            break;
        }
      });
}

RNNoiseUi::~RNNoiseUi() {
  util::debug(name + " ui destroyed");

  folder_monitor->cancel();
}

auto RNNoiseUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> RNNoiseUi* {
  const auto& builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/rnnoise.ui");

  auto* const ui = Gtk::Builder::get_widget_derived<RNNoiseUi>(builder, "top_box", "com.github.wwmm.easyeffects.rnnoise",
                                                         schema_path + "rnnoise/");

  stack->add(*ui, plugin_name::rnnoise);

  return ui;
}

void RNNoiseUi::setup_listview() {
  const auto& names = get_model_names();

  for (const auto& name : names) {
    string_list->append(name);
  }

  if (names.empty()) {
    settings->set_string("model-path", "");
  }

  // sorter

  const auto& sorter =
      Gtk::StringSorter::create(Gtk::PropertyExpression<Glib::ustring>::create(GTK_TYPE_STRING_OBJECT, "string"));

  const auto& sort_list_model = Gtk::SortListModel::create(string_list, sorter);

  // setting the listview model and factory

  listview->set_model(Gtk::SingleSelection::create(sort_list_model));

  auto factory = Gtk::SignalListItemFactory::create();

  listview->set_factory(factory);

  // setting the factory callbacks

  factory->signal_setup().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const box = Gtk::make_managed<Gtk::Box>();
    auto* const label = Gtk::make_managed<Gtk::Label>();
    auto* const remove = Gtk::make_managed<Gtk::Button>();

    label->set_hexpand(true);
    label->set_halign(Gtk::Align::START);

    remove->set_icon_name("user-trash-symbolic");

    box->set_spacing(6);
    box->append(*label);
    box->append(*remove);

    list_item->set_data("name", label);
    list_item->set_data("remove", remove);

    list_item->set_child(*box);
  });

  factory->signal_bind().connect([=, this](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const label = static_cast<Gtk::Label*>(list_item->get_data("name"));
    auto* const remove = static_cast<Gtk::Button*>(list_item->get_data("remove"));

    const auto& name = list_item->get_item()->get_property<Glib::ustring>("string");

    label->set_text(name);

    if (name == default_model_name) {
      remove->hide();
    }

    auto connection_remove = remove->signal_clicked().connect([=, this]() { remove_model_file(name); });

    list_item->set_data("connection_remove", new sigc::connection(connection_remove),
                        Glib::destroy_notify_delete<sigc::connection>);
  });

  factory->signal_unbind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    if (auto* connection = static_cast<sigc::connection*>(list_item->get_data("connection_remove"))) {
      connection->disconnect();

      list_item->set_data("connection_remove", nullptr);
    }
  });

  // selection callback

  listview->get_model()->signal_selection_changed().connect(
      [&, this](const guint& position, const guint& n_items) { on_selection_changed(); });

  // initializing selecting the row that corresponds to the saved model

  const auto* model_path = settings->get_string("model-path").c_str();

  const Glib::ustring saved_name = std::filesystem::path{model_path}.stem().c_str();

  auto single = std::dynamic_pointer_cast<Gtk::SingleSelection>(listview->get_model());

  for (guint n = 0U, m = single->get_n_items(); n < m; n++) {
    if (single->get_object(n)->get_property<Glib::ustring>("string") == saved_name) {
      single->select_item(n, true);
    }
  }
}

void RNNoiseUi::on_import_model_clicked() {
  Glib::RefPtr<Gtk::FileChooserNative> dialog;

  if (transient_window != nullptr) {
    dialog = Gtk::FileChooserNative::create(_("Import Model File"), *transient_window, Gtk::FileChooser::Action::OPEN,
                                            _("Open"), _("Cancel"));
  } else {
    dialog =
        Gtk::FileChooserNative::create(_("Import Model File"), Gtk::FileChooser::Action::OPEN, _("Open"), _("Cancel"));
  }

  auto dialog_filter = Gtk::FileFilter::create();

  dialog_filter->set_name(_("RNNoise Models"));
  dialog_filter->add_pattern("*.rnnn");

  dialog->add_filter(dialog_filter);

  dialog->signal_response().connect([=, this](const auto& response_id) {
    switch (response_id) {
      case Gtk::ResponseType::ACCEPT: {
        import_model_file(dialog->get_file()->get_path());

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

    out_path.replace_extension(rnnn_ext);

    std::filesystem::copy_file(p, out_path, std::filesystem::copy_options::overwrite_existing);

    util::debug(log_tag + "imported model file to: " + out_path.string());
  } else {
    util::warning(log_tag + p.string() + " is not a file!");
  }
}

auto RNNoiseUi::get_model_names() -> std::vector<Glib::ustring> {
  std::filesystem::directory_iterator it{model_dir};
  std::vector<Glib::ustring> names;

  while (it != std::filesystem::directory_iterator{}) {
    if (std::filesystem::is_regular_file(it->status())) {
      if (it->path().extension().c_str() == rnnn_ext) {
        names.emplace_back(it->path().stem().c_str());
      }
    }

    ++it;
  }

  return names;
}

void RNNoiseUi::remove_model_file(const Glib::ustring& name) {
  const auto& model_file = model_dir / std::filesystem::path{name.c_str() + rnnn_ext};

  if (std::filesystem::exists(model_file)) {
    std::filesystem::remove(model_file);

    util::debug(log_tag + "removed model file: " + model_file.string());
  }
}

void RNNoiseUi::set_active_model_label() {
  const auto& path = std::filesystem::path{settings->get_string("model-path")};

  if (settings->get_string("model-path").empty()) {
    active_model_name->set_text(default_model_name);
  } else {
    active_model_name->set_text(path.stem().string());
  }
}

void RNNoiseUi::on_selection_changed() {
  auto single = std::dynamic_pointer_cast<Gtk::SingleSelection>(listview->get_model());

  const auto& selected_name = single->get_selected_item()->get_property<Glib::ustring>("string");

  const auto& model_file = model_dir / std::filesystem::path{selected_name.c_str() + rnnn_ext};

  settings->set_string("model-path", model_file.c_str());
}

void RNNoiseUi::reset() {
  bypass->set_active(false);

  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("model-path");
}
