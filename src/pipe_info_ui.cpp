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

#include "pipe_info_ui.hpp"

ModuleInfoHolder::ModuleInfoHolder(ModuleInfo info)
    : Glib::ObjectBase(typeid(ModuleInfoHolder)), Glib::Object(), info(std::move(info)) {}

auto ModuleInfoHolder::create(const ModuleInfo& info) -> Glib::RefPtr<ModuleInfoHolder> {
  return Glib::make_refptr_for_instance<ModuleInfoHolder>(new ModuleInfoHolder(info));
}

ClientInfoHolder::ClientInfoHolder(ClientInfo info)
    : Glib::ObjectBase(typeid(ClientInfoHolder)), Glib::Object(), info(std::move(info)) {}

auto ClientInfoHolder::create(const ClientInfo& info) -> Glib::RefPtr<ClientInfoHolder> {
  return Glib::make_refptr_for_instance<ClientInfoHolder>(new ClientInfoHolder(info));
}

PipeInfoUi::PipeInfoUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, PipeManager* pm_ptr)
    : Gtk::Box(cobject),
      pm(pm_ptr),
      modules_model(Gio::ListStore<ModuleInfoHolder>::create()),
      clients_model(Gio::ListStore<ClientInfoHolder>::create()) {
  stack = builder->get_widget<Gtk::Stack>("stack");

  listview_modules = builder->get_widget<Gtk::ListView>("listview_modules");
  listview_clients = builder->get_widget<Gtk::ListView>("listview_clients");

  header_version = builder->get_widget<Gtk::Label>("header_version");
  library_version = builder->get_widget<Gtk::Label>("library_version");
  default_sink = builder->get_widget<Gtk::Label>("default_sink");
  default_source = builder->get_widget<Gtk::Label>("default_source");
  quantum = builder->get_widget<Gtk::Label>("quantum");
  max_quantum = builder->get_widget<Gtk::Label>("max_quantum");
  min_quantum = builder->get_widget<Gtk::Label>("min_quantum");
  server_rate = builder->get_widget<Gtk::Label>("server_rate");

  setup_listview_modules();
  setup_listview_clients();

  stack->connect_property_changed("visible-child", sigc::mem_fun(*this, &PipeInfoUi::on_stack_visible_child_changed));

  update_server_info();
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
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/pipe_info.ui");

  auto* ui = Gtk::Builder::get_widget_derived<PipeInfoUi>(builder, "top_box", pm);

  auto stack_page = stack->add(*ui, "pipe_info");

  return ui;
}

void PipeInfoUi::setup_listview_modules() {
  // setting the listview model and factory

  listview_modules->set_model(Gtk::NoSelection::create(modules_model));

  auto factory = Gtk::SignalListItemFactory::create();

  listview_modules->set_factory(factory);

  // setting the factory callbacks

  factory->signal_setup().connect([](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/module_info.ui");

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
    auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/client_info.ui");

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

void PipeInfoUi::update_server_info() {
  header_version->set_text(pm->header_version);
  library_version->set_text(pm->library_version);

  default_sink->set_text(pm->default_output_device.name);
  default_source->set_text(pm->default_input_device.name);
  server_rate->set_text(pm->default_clock_rate);

  min_quantum->set_text(pm->default_min_quantum);
  max_quantum->set_text(pm->default_max_quantum);
  quantum->set_text(pm->default_quantum);
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

  if (name == "page_server") {
    update_server_info();
  } else if (name == "page_modules") {
    update_modules_info();
  } else if (name == "page_clients") {
    update_clients_info();
  }
}
