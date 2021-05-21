/*
 *  Copyright © 2017-2020 Wellington Wallace
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

PipeInfoUi::PipeInfoUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, PipeManager* pm_ptr)
    : Gtk::Box(cobject),
      pm(pm_ptr),
      sie_settings(Gio::Settings::create("com.github.wwmm.easyeffects.streaminputs")),
      soe_settings(Gio::Settings::create("com.github.wwmm.easyeffects.streamoutputs")),
      input_devices_model(Gio::ListStore<NodeInfoHolder>::create()),
      output_devices_model(Gio::ListStore<NodeInfoHolder>::create()),
      modules_model(Gio::ListStore<ModuleInfoHolder>::create()),
      clients_model(Gio::ListStore<ClientInfoHolder>::create()) {
  stack = builder->get_widget<Gtk::Stack>("stack");

  use_default_input = builder->get_widget<Gtk::Switch>("use_default_input");
  use_default_output = builder->get_widget<Gtk::Switch>("use_default_output");

  dropdown_input_devices = builder->get_widget<Gtk::DropDown>("dropdown_input_devices");
  dropdown_output_devices = builder->get_widget<Gtk::DropDown>("dropdown_output_devices");

  listview_modules = builder->get_widget<Gtk::ListView>("listview_modules");
  listview_clients = builder->get_widget<Gtk::ListView>("listview_clients");

  header_version = builder->get_widget<Gtk::Label>("header_version");
  library_version = builder->get_widget<Gtk::Label>("library_version");
  quantum = builder->get_widget<Gtk::Label>("quantum");
  max_quantum = builder->get_widget<Gtk::Label>("max_quantum");
  min_quantum = builder->get_widget<Gtk::Label>("min_quantum");
  server_rate = builder->get_widget<Gtk::Label>("server_rate");

  setup_dropdown_devices(dropdown_input_devices, input_devices_model);
  setup_dropdown_devices(dropdown_output_devices, output_devices_model);

  setup_listview_modules();
  setup_listview_clients();

  stack->connect_property_changed("visible-child", sigc::mem_fun(*this, &PipeInfoUi::on_stack_visible_child_changed));

  use_default_input->signal_state_set().connect(
      [=, this](bool state) {
        if (state) {
          sie_settings->set_string("input-device", pm->default_input_device.name);
        }

        return false;
      },
      true);

  use_default_output->signal_state_set().connect(
      [=, this](bool state) {
        if (state) {
          soe_settings->set_string("output-device", pm->default_output_device.name);
        }

        return true;
      },
      true);

  sie_settings->bind("use-default-input-device", use_default_input, "active");
  sie_settings->bind("use-default-input-device", dropdown_input_devices, "sensitive",
                     Gio::Settings::BindFlags::INVERT_BOOLEAN);

  soe_settings->bind("use-default-output-device", use_default_output, "active");
  soe_settings->bind("use-default-output-device", dropdown_output_devices, "sensitive",
                     Gio::Settings::BindFlags::INVERT_BOOLEAN);

  for (const auto& node : pm->list_nodes) {
    if (node.name == "easyeffects_sink" || node.name == "easyeffects_source") {
      continue;
    }

    if (node.media_class == "Audio/Sink") {
      output_devices_model->append(NodeInfoHolder::create(node));
    } else if (node.media_class == "Audio/Source") {
      input_devices_model->append(NodeInfoHolder::create(node));
    }
  }

  pm->sink_added.connect([=, this](const NodeInfo& info) {
    for (guint n = 0; n < output_devices_model->get_n_items(); n++) {
      auto item = output_devices_model->get_item(n);

      if (item->info.id == info.id) {
        return;
      }
    }

    output_devices_model->append(NodeInfoHolder::create(info));
  });

  pm->sink_removed.connect([=, this](const NodeInfo& info) {
    for (guint n = 0; n < output_devices_model->get_n_items(); n++) {
      auto item = output_devices_model->get_item(n);

      if (item->info.id == info.id) {
        output_devices_model->remove(n);

        return;
      }
    }
  });

  pm->source_added.connect([=, this](const NodeInfo& info) {
    for (guint n = 0; n < input_devices_model->get_n_items(); n++) {
      auto item = input_devices_model->get_item(n);

      if (item->info.id == info.id) {
        return;
      }
    }

    input_devices_model->append(NodeInfoHolder::create(info));
  });

  pm->source_removed.connect([=, this](const NodeInfo& info) {
    for (guint n = 0; n < input_devices_model->get_n_items(); n++) {
      auto item = input_devices_model->get_item(n);

      if (item->info.id == info.id) {
        input_devices_model->remove(n);

        return;
      }
    }
  });

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

auto PipeInfoUi::add_to_stack(Gtk::Stack* stack, PipeManager* pm) -> PipeInfoUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/pipe_info.ui");

  auto* ui = Gtk::Builder::get_widget_derived<PipeInfoUi>(builder, "top_box", pm);

  auto stack_page = stack->add(*ui, "pipe_info");

  return ui;
}

void PipeInfoUi::setup_dropdown_devices(Gtk::DropDown* dropdown,
                                        const Glib::RefPtr<Gio::ListStore<NodeInfoHolder>>& model) {
  // setting the dropdown model and factory

  auto selection_model = Gtk::SingleSelection::create(model);

  dropdown->set_model(selection_model);

  auto factory = Gtk::SignalListItemFactory::create();

  dropdown->set_factory(factory);

  // setting the item selection callback

  dropdown->property_selected_item().signal_changed().connect([=, this]() {
    if (dropdown->get_selected_item() == nullptr) {
      return;
    }

    auto holder = std::dynamic_pointer_cast<NodeInfoHolder>(dropdown->get_selected_item());

    if (holder->info.media_class == "Audio/Sink") {
      soe_settings->set_string("output-device", holder->info.name);
    } else if (holder->info.media_class == "Audio/Source") {
      sie_settings->set_string("input-device", holder->info.name);
    }
  });

  // setting the factory callbacks

  factory->signal_setup().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* box = Gtk::make_managed<Gtk::Box>();
    auto* label = Gtk::make_managed<Gtk::Label>();
    auto* icon = Gtk::make_managed<Gtk::Image>();

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
    auto* label = static_cast<Gtk::Label*>(list_item->get_data("name"));
    auto* icon = static_cast<Gtk::Image*>(list_item->get_data("icon"));

    auto holder = std::dynamic_pointer_cast<NodeInfoHolder>(list_item->get_item());

    auto name = holder->info.name;

    if (holder->info.media_class == "Audio/Sink") {
      icon->set_from_icon_name("audio-card-symbolic");
    } else if (holder->info.media_class == "Audio/Source") {
      icon->set_from_icon_name("audio-input-microphone-symbolic");
    }

    label->set_name(name);
    label->set_text(name);
  });
}

void PipeInfoUi::setup_listview_modules() {
  // setting the listview model and factory

  listview_modules->set_model(Gtk::NoSelection::create(modules_model));

  auto factory = Gtk::SignalListItemFactory::create();

  listview_modules->set_factory(factory);

  // setting the factory callbacks

  factory->signal_setup().connect([](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/module_info.ui");

    auto* top_box = b->get_widget<Gtk::Box>("top_box");

    list_item->set_data("id", b->get_widget<Gtk::Label>("id"));
    list_item->set_data("name", b->get_widget<Gtk::Label>("name"));
    list_item->set_data("description", b->get_widget<Gtk::Label>("description"));

    list_item->set_child(*top_box);
  });

  factory->signal_bind().connect([](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* id = static_cast<Gtk::Label*>(list_item->get_data("id"));
    auto* name = static_cast<Gtk::Label*>(list_item->get_data("name"));
    auto* description = static_cast<Gtk::Label*>(list_item->get_data("description"));

    auto holder = std::dynamic_pointer_cast<ModuleInfoHolder>(list_item->get_item());

    id->set_text(std::to_string(holder->info.id));
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
    auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/client_info.ui");

    auto* top_box = b->get_widget<Gtk::Box>("top_box");

    list_item->set_data("id", b->get_widget<Gtk::Label>("id"));
    list_item->set_data("name", b->get_widget<Gtk::Label>("name"));
    list_item->set_data("api", b->get_widget<Gtk::Label>("api"));
    list_item->set_data("access", b->get_widget<Gtk::Label>("access"));

    list_item->set_child(*top_box);
  });

  factory->signal_bind().connect([](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* id = static_cast<Gtk::Label*>(list_item->get_data("id"));
    auto* name = static_cast<Gtk::Label*>(list_item->get_data("name"));
    auto* api = static_cast<Gtk::Label*>(list_item->get_data("api"));
    auto* access = static_cast<Gtk::Label*>(list_item->get_data("access"));

    auto holder = std::dynamic_pointer_cast<ClientInfoHolder>(list_item->get_item());

    id->set_text(std::to_string(holder->info.id));
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
  auto name = stack->get_visible_child_name();

  if (name == "page_modules") {
    update_modules_info();
  } else if (name == "page_clients") {
    update_clients_info();
  }
}
