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
#include <boost/algorithm/string.hpp>
#include <boost/process.hpp>
#include "util.hpp"

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
  builder->get_widget("listbox_config", listbox_config);
  builder->get_widget("listbox_resamplers", listbox_resamplers);
  builder->get_widget("config_file", config_file);

  listbox_modules->set_sort_func(sigc::ptr_fun(&PipeInfoUi::on_listbox_sort));

  listbox_clients->set_sort_func(sigc::ptr_fun(&PipeInfoUi::on_listbox_sort));

  listbox_config->set_sort_func(sigc::ptr_fun(&PipeInfoUi::on_listbox_sort));

  listbox_resamplers->set_sort_func(sigc::ptr_fun(&PipeInfoUi::on_listbox_sort));

  stack->connect_property_changed("visible-child", sigc::mem_fun(*this, &PipeInfoUi::on_stack_visible_child_changed));

  // connections.emplace_back(pm->server_changed.connect([=]() { update_server_info(); }));

  // connections.emplace_back(pm->module_info.connect([=](auto info) {
  //   auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/module_info.glade");

  //   Gtk::ListBoxRow* row;
  //   Gtk::Label* module_name;
  //   Gtk::Label* module_argument;

  //   b->get_widget("module_row", row);
  //   b->get_widget("module_name", module_name);
  //   b->get_widget("module_argument", module_argument);

  //   row->set_name(info->name);
  //   module_name->set_text(info->name);
  //   module_argument->set_text(info->argument);

  //   listbox_modules->add(*row);
  //   listbox_modules->show_all();
  // }));

  // connections.emplace_back(pm->client_info.connect([=](auto info) {
  //   auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/client_info.glade");

  //   Gtk::ListBoxRow* row;
  //   Gtk::Label* client_name;
  //   Gtk::Label* client_binary;

  //   b->get_widget("client_row", row);
  //   b->get_widget("client_name", client_name);
  //   b->get_widget("client_binary", client_binary);

  //   row->set_name(info->name);
  //   client_name->set_text(info->name);
  //   client_binary->set_text(info->binary);

  //   listbox_clients->add(*row);
  //   listbox_clients->show_all();
  // }));

  update_server_info();

  // get_pulse_conf();
  // get_resamplers();
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

  default_sink->set_text(pm->get_default_sink().name);
  default_source->set_text(pm->get_default_source().name);
  server_rate->set_text(pm->default_clock_rate);

  min_quantum->set_text(pm->default_min_quantum);
  max_quantum->set_text(pm->default_max_quantum);
  quantum->set_text(pm->default_quantum);

  // protocol->set_text(pm->server_info.protocol);
  // server_sample_format->set_text(pm->server_info.format);
  // server_channels->set_text(std::to_string(pm->server_info.channels));
  // server_channel_mapping->set_text(pm->server_info.channel_map);
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
    auto children = listbox_modules->get_children();

    for (const auto& c : children) {
      listbox_modules->remove(*c);
    }

  } else if (name == std::string("page_clients")) {
    auto children = listbox_clients->get_children();

    for (const auto& c : children) {
      listbox_clients->remove(*c);
    }
  }
}

void PipeInfoUi::get_pulse_conf() {
  std::string command = "pulseaudio --dump-conf";

  try {
    boost::process::ipstream pipe_stream;
    boost::process::child c(command, boost::process::std_out > pipe_stream);

    std::string line;

    while (pipe_stream && std::getline(pipe_stream, line) && !line.empty()) {
      std::vector<std::string> aux;
      std::string key;
      std::string value;

      boost::split(aux, line, boost::is_any_of("="));

      if (aux.size() > 1U) {
        auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/pipe_conf_file_line.glade");

        Gtk::ListBoxRow* row = nullptr;
        Gtk::Label* conf_key = nullptr;
        Gtk::Label* conf_value = nullptr;

        b->get_widget("conf_row", row);
        b->get_widget("conf_key", conf_key);
        b->get_widget("conf_value", conf_value);

        row->set_name(aux[0]);
        conf_key->set_text(aux[0]);
        conf_value->set_text(aux[1]);

        listbox_config->add(*row);
      } else {
        boost::split(aux, line, boost::is_any_of(":"));

        if (aux.size() > 1U) {
          std::string tmp = aux[1];

          boost::split(aux, tmp, boost::is_any_of("#"));

          config_file->set_text(aux[0]);
        }
      }
    }

    // c.wait();
    listbox_config->show_all();
  } catch (std::exception& e) {
    util::warning(log_tag + command + " : " + e.what());
  }
}

void PipeInfoUi::get_resamplers() {
  std::string command = "pulseaudio --dump-resample-methods";

  try {
    boost::process::ipstream pipe_stream;
    boost::process::child c(command, boost::process::std_out > pipe_stream);

    std::string line;

    while (pipe_stream && std::getline(pipe_stream, line) && !line.empty()) {
      auto* row = Gtk::manage(new Gtk::ListBoxRow());
      auto* label = Gtk::manage(new Gtk::Label());

      row->set_name(line);

      label->set_text(line);
      label->set_halign(Gtk::Align::ALIGN_START);

      row->add(*label);

      listbox_resamplers->add(*row);
    }

    listbox_resamplers->show_all();
  } catch (std::exception& e) {
    util::warning(log_tag + command + " : " + e.what());
  }
}
