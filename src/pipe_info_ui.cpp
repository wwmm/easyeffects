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

PipeInfoUi::PipeInfoUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, PipeManager* pm_ptr)
    : Gtk::Box(cobject), pm(pm_ptr), modules_model(Gio::ListStore<ModuleInfoHolder>::create()) {
  stack = builder->get_widget<Gtk::Stack>("stack");

  listview_modules = builder->get_widget<Gtk::ListView>("listview_modules");
  // listview_clients = builder->get_widget<Gtk::ListView>("listview_clients");

  header_version = builder->get_widget<Gtk::Label>("header_version");
  library_version = builder->get_widget<Gtk::Label>("library_version");
  default_sink = builder->get_widget<Gtk::Label>("default_sink");
  default_source = builder->get_widget<Gtk::Label>("default_source");
  quantum = builder->get_widget<Gtk::Label>("quantum");
  max_quantum = builder->get_widget<Gtk::Label>("max_quantum");
  min_quantum = builder->get_widget<Gtk::Label>("min_quantum");
  server_rate = builder->get_widget<Gtk::Label>("server_rate");
  // config_file = builder->get_widget<Gtk::Label>("config_file");

  // textview_config_file = builder->get_widget<Gtk::TextView>("textview_config_file");

  setup_listview_modules();

  stack->connect_property_changed("visible-child", sigc::mem_fun(*this, &PipeInfoUi::on_stack_visible_child_changed));

  update_server_info();
  update_modules_info();
  update_clients_info();

  get_pipe_conf();
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

  factory->signal_setup().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {});

  factory->signal_bind().connect([=, this](const Glib::RefPtr<Gtk::ListItem>& list_item) {});

  factory->signal_unbind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {});
}

void PipeInfoUi::update_server_info() {
  header_version->set_text(pm->header_version);
  library_version->set_text(pm->library_version);

  default_sink->set_text(pm->default_sink.name);
  default_source->set_text(pm->default_source.name);
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

  // for (auto& module : pm->list_modules) {
  //   auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/module_info.glade");

  //   Gtk::ListBoxRow* row = nullptr;
  //   Gtk::Label* module_name = nullptr;
  //   Gtk::Label* module_argument = nullptr;

  //   b->get_widget("module_row", row);
  //   b->get_widget("module_name", module_name);
  //   b->get_widget("module_argument", module_argument);

  //   row->set_name(module.name);
  //   module_name->set_text(module.name);
  //   module_argument->set_text(module.description);
  // }
}

void PipeInfoUi::update_clients_info() {
  // auto children = listbox_clients->get_children();

  // for (const auto& c : children) {
  //   listbox_clients->remove(*c);
  // }

  // for (auto& client : pm->list_clients) {
  //   auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/client_info.glade");

  //   Gtk::ListBoxRow* row = nullptr;
  //   Gtk::Label* client_name = nullptr;
  //   Gtk::Label* client_id = nullptr;
  //   Gtk::Label* client_access = nullptr;
  //   Gtk::Label* client_api = nullptr;

  //   b->get_widget("client_row", row);
  //   b->get_widget("client_name", client_name);
  //   b->get_widget("client_id", client_id);
  //   b->get_widget("client_access", client_access);
  //   b->get_widget("client_api", client_api);

  //   row->set_name(client.name);

  //   client_name->set_text(client.name);
  //   client_id->set_text(std::to_string(client.id));
  //   client_access->set_text(client.access);
  //   client_api->set_text(client.api);

  //   listbox_clients->add(*row);
  //   listbox_clients->show_all();
  // }
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

void PipeInfoUi::get_pipe_conf() {
  std::string path = "/etc/pipewire/pipewire.conf";

  // config_file->set_text(path);

  if (!std::filesystem::is_regular_file(path)) {
    util::debug("the file " + path + " does not exist!");
  } else {
    std::ifstream f;

    f.open(path);

    std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    // auto buffer = textview_config_file->get_buffer();

    // buffer->set_text(str);

    f.close();
  }
}
