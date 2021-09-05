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

#include "pipe_info_ui.hpp"

PipeInfoUi::PipeInfoUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       PipeManager* pm_ptr,
                       PresetsManager* presets_manager)
    : Gtk::Box(cobject),
      pm(pm_ptr),
      presets_manager(presets_manager),
      ts(std::make_unique<TestSignals>(pm_ptr)),
      sie_settings(Gio::Settings::create("com.github.wwmm.easyeffects.streaminputs")),
      soe_settings(Gio::Settings::create("com.github.wwmm.easyeffects.streamoutputs")),
      input_devices_model(Gio::ListStore<NodeInfoHolder>::create()),
      output_devices_model(Gio::ListStore<NodeInfoHolder>::create()),
      modules_model(Gio::ListStore<ModuleInfoHolder>::create()),
      clients_model(Gio::ListStore<ClientInfoHolder>::create()),
      autoloading_output_model(Gio::ListStore<PresetsAutoloadingHolder>::create()),
      autoloading_input_model(Gio::ListStore<PresetsAutoloadingHolder>::create()),
      output_presets_string_list(Gtk::StringList::create({"initial_value"})),
      input_presets_string_list(Gtk::StringList::create({"initial_value"})) {
  for (const auto& [id, node] : pm->node_map) {
    if (node.name == "easyeffects_sink" || node.name == "easyeffects_source") {
      continue;
    }

    if (node.media_class == "Audio/Sink") {
      output_devices_model->append(NodeInfoHolder::create(node));
    } else if (node.media_class == "Audio/Source") {
      input_devices_model->append(NodeInfoHolder::create(node));
    }
  }

  stack = builder->get_widget<Gtk::Stack>("stack");

  use_default_input = builder->get_widget<Gtk::Switch>("use_default_input");
  use_default_output = builder->get_widget<Gtk::Switch>("use_default_output");
  enable_test_signal = builder->get_widget<Gtk::Switch>("enable_test_signal");

  dropdown_input_devices = builder->get_widget<Gtk::DropDown>("dropdown_input_devices");
  dropdown_output_devices = builder->get_widget<Gtk::DropDown>("dropdown_output_devices");
  dropdown_autoloading_output_devices = builder->get_widget<Gtk::DropDown>("dropdown_autoloading_output_devices");
  dropdown_autoloading_input_devices = builder->get_widget<Gtk::DropDown>("dropdown_autoloading_input_devices");
  dropdown_autoloading_output_presets = builder->get_widget<Gtk::DropDown>("dropdown_autoloading_output_presets");
  dropdown_autoloading_input_presets = builder->get_widget<Gtk::DropDown>("dropdown_autoloading_input_presets");

  listview_modules = builder->get_widget<Gtk::ListView>("listview_modules");
  listview_clients = builder->get_widget<Gtk::ListView>("listview_clients");
  listview_autoloading_output = builder->get_widget<Gtk::ListView>("listview_autoloading_output");
  listview_autoloading_input = builder->get_widget<Gtk::ListView>("listview_autoloading_input");

  autoloading_add_input_profile = builder->get_widget<Gtk::Button>("autoloading_add_input_profile");
  autoloading_add_output_profile = builder->get_widget<Gtk::Button>("autoloading_add_output_profile");

  header_version = builder->get_widget<Gtk::Label>("header_version");
  library_version = builder->get_widget<Gtk::Label>("library_version");
  quantum = builder->get_widget<Gtk::Label>("quantum");
  max_quantum = builder->get_widget<Gtk::Label>("max_quantum");
  min_quantum = builder->get_widget<Gtk::Label>("min_quantum");
  server_rate = builder->get_widget<Gtk::Label>("server_rate");

  spinbutton_test_signal_frequency = builder->get_widget<Gtk::SpinButton>("spinbutton_test_signal_frequency");

  checkbutton_channel_left = builder->get_widget<Gtk::CheckButton>("checkbutton_channel_left");
  checkbutton_channel_right = builder->get_widget<Gtk::CheckButton>("checkbutton_channel_right");
  checkbutton_channel_both = builder->get_widget<Gtk::CheckButton>("checkbutton_channel_both");
  checkbutton_signal_sine = builder->get_widget<Gtk::CheckButton>("checkbutton_signal_sine");
  checkbutton_signal_gaussian = builder->get_widget<Gtk::CheckButton>("checkbutton_signal_gaussian");
  // checkbutton_signal_pink = builder->get_widget<Gtk::CheckButton>("checkbutton_signal_pink");

  setup_dropdown_devices(dropdown_input_devices, input_devices_model);
  setup_dropdown_devices(dropdown_output_devices, output_devices_model);

  setup_dropdown_devices(dropdown_autoloading_input_devices, input_devices_model);
  setup_dropdown_devices(dropdown_autoloading_output_devices, output_devices_model);

  setup_dropdown_presets(PresetType::input, input_presets_string_list);
  setup_dropdown_presets(PresetType::output, output_presets_string_list);

  setup_listview_autoloading(PresetType::input, listview_autoloading_input, autoloading_input_model);
  setup_listview_autoloading(PresetType::output, listview_autoloading_output, autoloading_output_model);

  setup_listview_modules();
  setup_listview_clients();

  dropdown_input_devices->property_selected_item().signal_changed().connect([=, this]() {
    if (dropdown_input_devices->get_selected_item() == nullptr) {
      return;
    }

    auto holder = std::dynamic_pointer_cast<NodeInfoHolder>(dropdown_input_devices->get_selected_item());

    sie_settings->set_string("input-device", holder->info.name);
  });

  dropdown_output_devices->property_selected_item().signal_changed().connect([=, this]() {
    if (dropdown_output_devices->get_selected_item() == nullptr) {
      return;
    }

    auto holder = std::dynamic_pointer_cast<NodeInfoHolder>(dropdown_output_devices->get_selected_item());

    soe_settings->set_string("output-device", holder->info.name);
  });

  // setting the displayed entry to the right value

  {
    auto holder_selected = std::dynamic_pointer_cast<NodeInfoHolder>(dropdown_input_devices->get_selected_item());

    if (holder_selected != nullptr) {
      const auto* input_device_name = sie_settings->get_string("input-device").c_str();

      if (holder_selected->info.name != input_device_name) {
        for (guint n = 0U, m = input_devices_model->get_n_items(); n < m; n++) {
          if (input_devices_model->get_item(n)->info.name == input_device_name) {
            dropdown_input_devices->set_selected(n);
          }
        }
      }
    }
  }

  {
    auto holder_selected = std::dynamic_pointer_cast<NodeInfoHolder>(dropdown_output_devices->get_selected_item());

    if (holder_selected != nullptr) {
      const auto* output_device_name = soe_settings->get_string("output-device").c_str();

      if (holder_selected->info.name != output_device_name) {
        for (guint n = 0U, m = output_devices_model->get_n_items(); n < m; n++) {
          if (output_devices_model->get_item(n)->info.name == output_device_name) {
            dropdown_output_devices->set_selected(n);
          }
        }
      }
    }
  }

  stack->connect_property_changed("visible-child", sigc::mem_fun(*this, &PipeInfoUi::on_stack_visible_child_changed));

  use_default_input->property_active().signal_changed().connect([=, this]() {
    if (use_default_input->get_active()) {
      sie_settings->set_string("input-device", pm->default_input_device.name);

      auto holder = std::dynamic_pointer_cast<NodeInfoHolder>(dropdown_input_devices->get_selected_item());

      if (holder != nullptr) {
        if (holder->info.name != pm->default_input_device.name) {
          for (guint n = 0U, m = input_devices_model->get_n_items(); n < m; n++) {
            if (input_devices_model->get_item(n)->info.name == pm->default_input_device.name) {
              dropdown_input_devices->set_selected(n);

              break;
            }
          }
        }
      }
    }
  });

  use_default_output->property_active().signal_changed().connect([=, this]() {
    if (use_default_output->get_active()) {
      soe_settings->set_string("output-device", pm->default_output_device.name);

      auto holder_selected = std::dynamic_pointer_cast<NodeInfoHolder>(dropdown_output_devices->get_selected_item());

      if (holder_selected != nullptr) {
        if (holder_selected->info.name != pm->default_output_device.name) {
          for (guint n = 0U, m = output_devices_model->get_n_items(); n < m; n++) {
            if (output_devices_model->get_item(n)->info.name == pm->default_output_device.name) {
              dropdown_output_devices->set_selected(n);

              break;
            }
          }
        }
      }
    }
  });

  enable_test_signal->property_active().signal_changed().connect(
      [=, this]() { ts->set_state(enable_test_signal->get_active()); });

  autoloading_add_output_profile->signal_clicked().connect([=, this]() {
    if (dropdown_autoloading_output_devices->get_selected_item() == nullptr) {
      return;
    }

    auto holder = std::dynamic_pointer_cast<NodeInfoHolder>(dropdown_autoloading_output_devices->get_selected_item());

    std::string device_profile;

    for (const auto& device : pm->list_devices) {
      if (device.id == holder->info.device_id) {
        device_profile = device.profile_name;

        break;
      }
    }

    // first we remove any autoloading profile associated to the target device so that our ui is updated

    for (guint n = 0; n < autoloading_output_model->get_n_items(); n++) {
      const auto& item = autoloading_output_model->get_item(n);

      if (holder->info.name == item->device) {
        presets_manager->remove_autoload(PresetType::output, item->preset_name, item->device, item->device_profile);

        break;
      }
    }

    const auto& id = dropdown_autoloading_output_presets->get_selected();

    presets_manager->add_autoload(PresetType::output, output_presets_string_list->get_string(id).raw(),
                                  holder->info.name, device_profile);
  });

  autoloading_add_input_profile->signal_clicked().connect([=, this]() {
    if (dropdown_autoloading_input_devices->get_selected_item() == nullptr) {
      return;
    }

    auto holder = std::dynamic_pointer_cast<NodeInfoHolder>(dropdown_autoloading_input_devices->get_selected_item());

    std::string device_profile;

    for (const auto& device : pm->list_devices) {
      if (device.id == holder->info.device_id) {
        device_profile = device.profile_name;

        break;
      }
    }

    // first we remove any autoloading profile associated to the target device so that our ui is updated

    for (guint n = 0; n < autoloading_input_model->get_n_items(); n++) {
      const auto& item = autoloading_input_model->get_item(n);

      if (holder->info.name == item->device) {
        presets_manager->remove_autoload(PresetType::input, item->preset_name, item->device, item->device_profile);

        break;
      }
    }

    const auto& id = dropdown_autoloading_input_presets->get_selected();

    presets_manager->add_autoload(PresetType::input, input_presets_string_list->get_string(id).raw(),
                                  holder->info.name, device_profile);
  });

  spinbutton_test_signal_frequency->signal_output().connect(
      [=, this]() { return parse_spinbutton_output(spinbutton_test_signal_frequency, "Hz"); }, true);

  spinbutton_test_signal_frequency->signal_input().connect(
      [=, this](double& new_value) { return parse_spinbutton_input(spinbutton_test_signal_frequency, new_value); },
      true);

  spinbutton_test_signal_frequency->signal_value_changed().connect(
      [=, this]() { ts->set_frequency(static_cast<float>(spinbutton_test_signal_frequency->get_value())); });

  checkbutton_channel_left->signal_toggled().connect([&, this]() {
    if (checkbutton_channel_left->get_active()) {
      ts->create_left_channel = true;
      ts->create_right_channel = false;
    }
  });

  checkbutton_channel_right->signal_toggled().connect([&, this]() {
    if (checkbutton_channel_right->get_active()) {
      ts->create_left_channel = false;
      ts->create_right_channel = true;
    }
  });

  checkbutton_channel_both->signal_toggled().connect([&, this]() {
    if (checkbutton_channel_both->get_active()) {
      ts->create_left_channel = true;
      ts->create_right_channel = true;
    }
  });

  checkbutton_signal_sine->signal_toggled().connect([&, this]() {
    if (checkbutton_signal_sine->get_active()) {
      ts->signal_type = TestSignalType::sine_wave;
      ts->sine_phase = 0.0F;

      spinbutton_test_signal_frequency->set_sensitive(true);
    }
  });

  checkbutton_signal_gaussian->signal_toggled().connect([&, this]() {
    if (checkbutton_signal_gaussian->get_active()) {
      ts->signal_type = TestSignalType::gaussian;

      spinbutton_test_signal_frequency->set_sensitive(false);
    }
  });

  // checkbutton_signal_pink->signal_toggled().connect([&, this]() {
  //   if (checkbutton_signal_pink->get_active()) {
  //     ts->signal_type = TestSignalType::pink;

  //     spinbutton_test_signal_frequency->set_sensitive(false);
  //   }
  // });

  sie_settings->bind("use-default-input-device", use_default_input, "active");
  sie_settings->bind("use-default-input-device", dropdown_input_devices, "sensitive",
                     Gio::Settings::BindFlags::INVERT_BOOLEAN);

  soe_settings->bind("use-default-output-device", use_default_output, "active");
  soe_settings->bind("use-default-output-device", dropdown_output_devices, "sensitive",
                     Gio::Settings::BindFlags::INVERT_BOOLEAN);

  connections.emplace_back(pm->sink_added.connect([=, this](NodeInfo info) {
    for (guint n = 0U, m = output_devices_model->get_n_items(); n < m; n++) {
      if (output_devices_model->get_item(n)->info.id == info.id) {
        return;
      }
    }

    output_devices_model->append(NodeInfoHolder::create(info));
  }));

  connections.emplace_back(pm->sink_removed.connect([=, this](NodeInfo info) {
    for (guint n = 0U, m = output_devices_model->get_n_items(); n < m; n++) {
      if (output_devices_model->get_item(n)->info.id == info.id) {
        output_devices_model->remove(n);

        return;
      }
    }
  }));

  connections.emplace_back(pm->source_added.connect([=, this](NodeInfo info) {
    for (guint n = 0U, m = input_devices_model->get_n_items(); n < m; n++) {
      if (input_devices_model->get_item(n)->info.id == info.id) {
        return;
      }
    }

    input_devices_model->append(NodeInfoHolder::create(info));
  }));

  connections.emplace_back(pm->source_removed.connect([=, this](NodeInfo info) {
    for (guint n = 0U, m = input_devices_model->get_n_items(); n < m; n++) {
      if (input_devices_model->get_item(n)->info.id == info.id) {
        input_devices_model->remove(n);

        return;
      }
    }
  }));

  connections.emplace_back(
      presets_manager->user_output_preset_created.connect([=, this](const Glib::RefPtr<Gio::File>& file) {
        const auto& preset_name = util::remove_filename_extension(file->get_basename());

        if (preset_name.empty()) {
          util::warning("Can't retrieve information about the preset file");

          return;
        }

        for (guint n = 0, list_size = output_presets_string_list->get_n_items(); n < list_size; n++) {
          if (output_presets_string_list->get_string(n) == preset_name) {
            return;
          }
        }

        output_presets_string_list->append(preset_name);
      }));

  connections.emplace_back(
      presets_manager->user_output_preset_removed.connect([=, this](const Glib::RefPtr<Gio::File>& file) {
        const auto& preset_name = util::remove_filename_extension(file->get_basename());

        if (preset_name.empty()) {
          util::warning("Can't retrieve information about the preset file");

          return;
        }

        for (guint n = 0, list_size = output_presets_string_list->get_n_items(); n < list_size; n++) {
          if (output_presets_string_list->get_string(n) == preset_name) {
            output_presets_string_list->remove(n);

            return;
          }
        }
      }));

  connections.emplace_back(
      presets_manager->user_input_preset_created.connect([=, this](const Glib::RefPtr<Gio::File>& file) {
        const auto& preset_name = util::remove_filename_extension(file->get_basename());

        if (preset_name.empty()) {
          util::warning("Can't retrieve information about the preset file");

          return;
        }

        for (guint n = 0, list_size = input_presets_string_list->get_n_items(); n < list_size; n++) {
          if (input_presets_string_list->get_string(n) == preset_name) {
            return;
          }
        }

        input_presets_string_list->append(preset_name);
      }));

  connections.emplace_back(
      presets_manager->user_input_preset_removed.connect([=, this](const Glib::RefPtr<Gio::File>& file) {
        const auto& preset_name = util::remove_filename_extension(file->get_basename());

        if (preset_name.empty()) {
          util::warning("Can't retrieve information about the preset file");

          return;
        }

        for (guint n = 0, list_size = input_presets_string_list->get_n_items(); n < list_size; n++) {
          if (input_presets_string_list->get_string(n) == preset_name) {
            input_presets_string_list->remove(n);

            return;
          }
        }
      }));

  connections.emplace_back(
      presets_manager->autoload_output_profiles_changed.connect([=, this](const std::vector<nlohmann::json>& profiles) {
        std::vector<Glib::RefPtr<PresetsAutoloadingHolder>> list;

        for (const auto& json : profiles) {
          const auto& device = json.value("device", "");
          const auto& device_profile = json.value("device-profile", "");
          const auto& preset_name = json.value("preset-name", "");

          list.emplace_back(PresetsAutoloadingHolder::create(device, device_profile, preset_name));
        }

        autoloading_output_model->splice(0, autoloading_output_model->get_n_items(), list);
      }));

  connections.emplace_back(
      presets_manager->autoload_input_profiles_changed.connect([=, this](const std::vector<nlohmann::json>& profiles) {
        std::vector<Glib::RefPtr<PresetsAutoloadingHolder>> list;

        for (const auto& json : profiles) {
          const auto& device = json.value("device", "");
          const auto& device_profile = json.value("device-profile", "");
          const auto& preset_name = json.value("preset-name", "");

          list.emplace_back(PresetsAutoloadingHolder::create(device, device_profile, preset_name));
        }

        autoloading_input_model->splice(0, autoloading_input_model->get_n_items(), list);
      }));

  header_version->set_text(pm->header_version);
  library_version->set_text(pm->library_version);
  server_rate->set_text(pm->default_clock_rate);
  min_quantum->set_text(pm->default_min_quantum);
  max_quantum->set_text(pm->default_max_quantum);
  quantum->set_text(pm->default_quantum);

  update_modules_info();
  update_clients_info();
}

PipeInfoUi::~PipeInfoUi() {
  for (auto& c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

auto PipeInfoUi::add_to_stack(Gtk::Stack* stack, PipeManager* pm, PresetsManager* presets_manager) -> PipeInfoUi* {
  const auto& builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/pipe_info.ui");

  auto* const ui = Gtk::Builder::get_widget_derived<PipeInfoUi>(builder, "top_box", pm, presets_manager);

  stack->add(*ui, "pipe_info");

  return ui;
}

void PipeInfoUi::setup_dropdown_devices(Gtk::DropDown* dropdown,
                                        const Glib::RefPtr<Gio::ListStore<NodeInfoHolder>>& model) {
  // setting the dropdown model and factory

  const auto& selection_model = Gtk::SingleSelection::create(model);

  dropdown->set_model(selection_model);

  auto factory = Gtk::SignalListItemFactory::create();

  dropdown->set_factory(factory);

  // setting the factory callbacks

  factory->signal_setup().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const box = Gtk::make_managed<Gtk::Box>();
    auto* const label = Gtk::make_managed<Gtk::Label>();
    auto* const icon = Gtk::make_managed<Gtk::Image>();

    label->set_hexpand(true);
    label->set_halign(Gtk::Align::START);

    icon->set_from_icon_name("emblem-system-symbolic");

    box->set_spacing(6);
    box->append(*icon);
    box->append(*label);

    // setting list_item data

    list_item->set_data("name", label);
    list_item->set_data("icon", icon);

    list_item->set_child(*box);
  });

  factory->signal_bind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const label = static_cast<Gtk::Label*>(list_item->get_data("name"));
    auto* const icon = static_cast<Gtk::Image*>(list_item->get_data("icon"));

    auto holder = std::dynamic_pointer_cast<NodeInfoHolder>(list_item->get_item());

    if (holder->info.media_class == "Audio/Sink") {
      icon->set_from_icon_name("audio-card-symbolic");
    } else if (holder->info.media_class == "Audio/Source") {
      icon->set_from_icon_name("audio-input-microphone-symbolic");
    }

    label->set_name(holder->info.name);
    label->set_text(holder->info.name);
  });
}

void PipeInfoUi::setup_dropdown_presets(PresetType preset_type, const Glib::RefPtr<Gtk::StringList>& string_list) {
  Gtk::DropDown* dropdown = nullptr;

  switch (preset_type) {
    case PresetType::input:
      dropdown = dropdown_autoloading_input_presets;

      break;
    case PresetType::output:
      dropdown = dropdown_autoloading_output_presets;

      break;
  }

  string_list->remove(0);

  for (const auto& name : presets_manager->get_names(preset_type)) {
    string_list->append(name);
  }

  // sorter

  const auto& sorter =
      Gtk::StringSorter::create(Gtk::PropertyExpression<Glib::ustring>::create(GTK_TYPE_STRING_OBJECT, "string"));

  const auto& sort_list_model = Gtk::SortListModel::create(string_list, sorter);

  // setting the dropdown model and factory

  const auto& selection_model = Gtk::SingleSelection::create(sort_list_model);

  dropdown->set_model(selection_model);

  auto factory = Gtk::SignalListItemFactory::create();

  dropdown->set_factory(factory);

  // setting the factory callbacks

  factory->signal_setup().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const box = Gtk::make_managed<Gtk::Box>();
    auto* const label = Gtk::make_managed<Gtk::Label>();
    auto* const icon = Gtk::make_managed<Gtk::Image>();

    label->set_hexpand(true);
    label->set_halign(Gtk::Align::START);

    icon->set_from_icon_name("emblem-system-symbolic");

    box->set_spacing(6);
    box->append(*icon);
    box->append(*label);

    // setting list_item data

    list_item->set_data("name", label);

    list_item->set_child(*box);
  });

  factory->signal_bind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const label = static_cast<Gtk::Label*>(list_item->get_data("name"));

    const auto& name = list_item->get_item()->get_property<Glib::ustring>("string");

    label->set_name(name);
    label->set_text(name);
  });
}

void PipeInfoUi::setup_listview_autoloading(PresetType preset_type,
                                            Gtk::ListView* listview,
                                            const Glib::RefPtr<Gio::ListStore<PresetsAutoloadingHolder>>& model) {
  const auto& profiles = presets_manager->get_autoload_profiles(preset_type);

  for (const auto& json : profiles) {
    const auto& device = json.value("device", "");
    const auto& device_profile = json.value("device-profile", "");
    const auto& preset_name = json.value("preset-name", "");

    model->append(PresetsAutoloadingHolder::create(device, device_profile, preset_name));
  }

  // setting the listview model and factory

  listview->set_model(Gtk::NoSelection::create(model));

  auto factory = Gtk::SignalListItemFactory::create();

  listview->set_factory(factory);

  // setting the factory callbacks

  factory->signal_setup().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    const auto& b = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/autoload_row.ui");

    auto* const top_box = b->get_widget<Gtk::Box>("top_box");

    list_item->set_data("device", b->get_widget<Gtk::Label>("device"));
    list_item->set_data("device_profile", b->get_widget<Gtk::Label>("device_profile"));
    list_item->set_data("preset_name", b->get_widget<Gtk::Label>("preset_name"));
    list_item->set_data("remove", b->get_widget<Gtk::Button>("remove"));

    list_item->set_child(*top_box);
  });

  factory->signal_bind().connect([=, this](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const device = static_cast<Gtk::Label*>(list_item->get_data("device"));
    auto* const device_profile = static_cast<Gtk::Label*>(list_item->get_data("device_profile"));
    auto* const preset_name = static_cast<Gtk::Label*>(list_item->get_data("preset_name"));
    auto* const remove = static_cast<Gtk::Button*>(list_item->get_data("remove"));

    auto holder = std::dynamic_pointer_cast<PresetsAutoloadingHolder>(list_item->get_item());

    device->set_text(holder->device);
    device_profile->set_text(holder->device_profile);
    preset_name->set_text(holder->preset_name);

    auto connection_remove = remove->signal_clicked().connect([=, this]() {
      presets_manager->remove_autoload(preset_type, holder->preset_name, holder->device, holder->device_profile);
    });

    list_item->set_data("connection_remove", new sigc::connection(connection_remove),
                        Glib::destroy_notify_delete<sigc::connection>);
  });

  factory->signal_unbind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    if (auto* connection = static_cast<sigc::connection*>(list_item->get_data("connection_remove"))) {
      connection->disconnect();

      list_item->set_data("connection_remove", nullptr);
    }
  });
}

void PipeInfoUi::setup_listview_modules() {
  // setting the listview model and factory

  listview_modules->set_model(Gtk::NoSelection::create(modules_model));

  auto factory = Gtk::SignalListItemFactory::create();

  listview_modules->set_factory(factory);

  // setting the factory callbacks

  factory->signal_setup().connect([](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    const auto& b = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/module_info.ui");

    auto* const top_box = b->get_widget<Gtk::Box>("top_box");

    list_item->set_data("id", b->get_widget<Gtk::Label>("id"));
    list_item->set_data("name", b->get_widget<Gtk::Label>("name"));
    list_item->set_data("description", b->get_widget<Gtk::Label>("description"));

    list_item->set_child(*top_box);
  });

  factory->signal_bind().connect([](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const id = static_cast<Gtk::Label*>(list_item->get_data("id"));
    auto* const name = static_cast<Gtk::Label*>(list_item->get_data("name"));
    auto* const description = static_cast<Gtk::Label*>(list_item->get_data("description"));

    auto holder = std::dynamic_pointer_cast<ModuleInfoHolder>(list_item->get_item());

    id->set_text(Glib::ustring::format(holder->info.id));
    name->set_text(holder->info.name);
    description->set_text(holder->info.description);
  });
}

void PipeInfoUi::setup_listview_clients() {
  // setting the listview model and factory

  listview_clients->set_model(Gtk::NoSelection::create(clients_model));

  auto factory = Gtk::SignalListItemFactory::create();

  listview_clients->set_factory(factory);

  // setting the factory callbacks

  factory->signal_setup().connect([](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    const auto& b = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/client_info.ui");

    auto* const top_box = b->get_widget<Gtk::Box>("top_box");

    list_item->set_data("id", b->get_widget<Gtk::Label>("id"));
    list_item->set_data("name", b->get_widget<Gtk::Label>("name"));
    list_item->set_data("api", b->get_widget<Gtk::Label>("api"));
    list_item->set_data("access", b->get_widget<Gtk::Label>("access"));

    list_item->set_child(*top_box);
  });

  factory->signal_bind().connect([](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const id = static_cast<Gtk::Label*>(list_item->get_data("id"));
    auto* const name = static_cast<Gtk::Label*>(list_item->get_data("name"));
    auto* const api = static_cast<Gtk::Label*>(list_item->get_data("api"));
    auto* const access = static_cast<Gtk::Label*>(list_item->get_data("access"));

    auto holder = std::dynamic_pointer_cast<ClientInfoHolder>(list_item->get_item());

    id->set_text(Glib::ustring::format(holder->info.id));
    name->set_text(holder->info.name);
    api->set_text(holder->info.api);
    access->set_text(holder->info.access);
  });
}

void PipeInfoUi::update_modules_info() {
  std::vector<Glib::RefPtr<ModuleInfoHolder>> values;

  for (const auto& info : pm->list_modules) {
    values.emplace_back(ModuleInfoHolder::create(info));
  }

  modules_model->splice(0, modules_model->get_n_items(), values);
}

void PipeInfoUi::update_clients_info() {
  std::vector<Glib::RefPtr<ClientInfoHolder>> values;

  for (const auto& info : pm->list_clients) {
    values.emplace_back(ClientInfoHolder::create(info));
  }

  clients_model->splice(0, clients_model->get_n_items(), values);
}

void PipeInfoUi::on_stack_visible_child_changed() {
  if (const auto& name = stack->get_visible_child_name(); name == "page_modules") {
    update_modules_info();
  } else if (name == "page_clients") {
    update_clients_info();
  }
}
