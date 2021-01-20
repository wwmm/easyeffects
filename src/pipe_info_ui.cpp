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

PipeInfoUi::PipeInfoUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, PipeManager* pm_ptr)
    : Gtk::Box(cobject), pm(pm_ptr) {
  builder->get_widget("stack", stack);
  builder->get_widget("server_name", server_name);
  builder->get_widget("header_version", header_version);
  builder->get_widget("library_version", library_version);
  builder->get_widget("default_sink", default_sink);
  builder->get_widget("default_source", default_source);
  builder->get_widget("quantum", quantum);
  builder->get_widget("max_quantum", max_quantum);
  builder->get_widget("min_quantum", min_quantum);
  builder->get_widget("server_rate", server_rate);
  builder->get_widget("listbox_modules", listbox_modules);
  builder->get_widget("listbox_clients", listbox_clients);
  builder->get_widget("config_file", config_file);
  builder->get_widget("textview_config_file", textview_config_file);

  listbox_modules->set_sort_func(sigc::ptr_fun(&PipeInfoUi::on_listbox_sort));

  listbox_clients->set_sort_func(sigc::ptr_fun(&PipeInfoUi::on_listbox_sort));

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
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/pipe_info.glade");

  PipeInfoUi* ui = nullptr;

  builder->get_widget_derived("widgets_box", ui, pm);

  stack->add(*ui, "pipe_info");
  stack->child_property_icon_name(*ui).set_value("network-server-symbolic");

  return ui;
}

void PipeInfoUi::update_server_info() {
  header_version->set_text(pm->header_version);
  library_version->set_text(pm->library_version);

  server_name->set_text(pm->core_name);

  default_sink->set_text(pm->default_sink.name);
  default_source->set_text(pm->default_source.name);
  server_rate->set_text(pm->default_clock_rate);

  min_quantum->set_text(pm->default_min_quantum);
  max_quantum->set_text(pm->default_max_quantum);
  quantum->set_text(pm->default_quantum);
}

void PipeInfoUi::update_modules_info() {
  auto children = listbox_modules->get_children();

  for (const auto& c : children) {
    listbox_modules->remove(*c);
  }

  for (auto& module : pm->list_modules) {
    auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/module_info.glade");

    Gtk::ListBoxRow* row = nullptr;
    Gtk::Label* module_name = nullptr;
    Gtk::Label* module_argument = nullptr;

    b->get_widget("module_row", row);
    b->get_widget("module_name", module_name);
    b->get_widget("module_argument", module_argument);

    row->set_name(module.name);
    module_name->set_text(module.name);
    module_argument->set_text(module.description);

    listbox_modules->add(*row);
    listbox_modules->show_all();
  }
}

void PipeInfoUi::update_clients_info() {
  auto children = listbox_clients->get_children();

  for (const auto& c : children) {
    listbox_clients->remove(*c);
  }

  for (auto& client : pm->list_clients) {
    auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/client_info.glade");

    Gtk::ListBoxRow* row = nullptr;
    Gtk::Label* client_name = nullptr;
    Gtk::Label* client_id = nullptr;
    Gtk::Label* client_access = nullptr;
    Gtk::Label* client_api = nullptr;

    b->get_widget("client_row", row);
    b->get_widget("client_name", client_name);
    b->get_widget("client_id", client_id);
    b->get_widget("client_access", client_access);
    b->get_widget("client_api", client_api);

    row->set_name(client.name);

    client_name->set_text(client.name);
    client_id->set_text(std::to_string(client.id));
    client_access->set_text(client.access);
    client_api->set_text(client.api);

    listbox_clients->add(*row);
    listbox_clients->show_all();
  }
}

auto PipeInfoUi::on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) -> int {
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

void PipeInfoUi::on_stack_visible_child_changed() {
  auto name = stack->get_visible_child_name();

  if (name == std::string("page_server")) {
    update_server_info();
  } else if (name == std::string("page_modules")) {
    update_modules_info();
  } else if (name == std::string("page_clients")) {
    update_clients_info();
  }
}

void PipeInfoUi::get_pipe_conf() {
  std::string path = "/etc/pipewire/pipewire.conf";

  config_file->set_text(path);

  if (!std::filesystem::is_regular_file(path)) {
    util::debug("the file " + path + " does not exist!");
  } else {
    std::ifstream f;

    f.open(path);

    std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    auto buffer = textview_config_file->get_buffer();

    buffer->set_text(str);

    f.close();
  }
}
