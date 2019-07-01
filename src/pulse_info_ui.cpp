#include "pulse_info_ui.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/process.hpp>
#include "util.hpp"

PulseInfoUi::PulseInfoUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, PulseManager* pm_ptr)
    : Gtk::Box(cobject), pm(pm_ptr) {
  builder->get_widget("stack", stack);
  builder->get_widget("server_name", server_name);
  builder->get_widget("server_version", server_version);
  builder->get_widget("default_sink", default_sink);
  builder->get_widget("default_source", default_source);
  builder->get_widget("server_protocol", protocol);
  builder->get_widget("server_sample_format", server_sample_format);
  builder->get_widget("server_rate", server_rate);
  builder->get_widget("server_channels", server_channels);
  builder->get_widget("server_channel_mapping", server_channel_mapping);
  builder->get_widget("listbox_modules", listbox_modules);
  builder->get_widget("listbox_clients", listbox_clients);
  builder->get_widget("listbox_config", listbox_config);
  builder->get_widget("listbox_resamplers", listbox_resamplers);
  builder->get_widget("config_file", config_file);

  listbox_modules->set_sort_func(sigc::mem_fun(*this, &PulseInfoUi::on_listbox_sort));

  listbox_clients->set_sort_func(sigc::mem_fun(*this, &PulseInfoUi::on_listbox_sort));

  listbox_config->set_sort_func(sigc::mem_fun(*this, &PulseInfoUi::on_listbox_sort));

  listbox_resamplers->set_sort_func(sigc::mem_fun(*this, &PulseInfoUi::on_listbox_sort));

  stack->connect_property_changed("visible-child", sigc::mem_fun(*this, &PulseInfoUi::on_stack_visible_child_changed));

  connections.push_back(pm->server_changed.connect([=]() { update_server_info(); }));

  connections.push_back(pm->module_info.connect([=](auto info) {
    auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/module_info.glade");

    Gtk::ListBoxRow* row;
    Gtk::Label *module_name, *module_argument;

    b->get_widget("module_row", row);
    b->get_widget("module_name", module_name);
    b->get_widget("module_argument", module_argument);

    row->set_name(info->name);
    module_name->set_text(info->name);
    module_argument->set_text(info->argument);

    listbox_modules->add(*row);
    listbox_modules->show_all();
  }));

  connections.push_back(pm->client_info.connect([=](auto info) {
    auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/client_info.glade");

    Gtk::ListBoxRow* row;
    Gtk::Label *client_name, *client_binary;

    b->get_widget("client_row", row);
    b->get_widget("client_name", client_name);
    b->get_widget("client_binary", client_binary);

    row->set_name(info->name);
    client_name->set_text(info->name);
    client_binary->set_text(info->binary);

    listbox_clients->add(*row);
    listbox_clients->show_all();
  }));

  update_server_info();

  pm->get_modules_info();
  pm->get_clients_info();

  get_pulse_conf();
  get_resamplers();
}

PulseInfoUi::~PulseInfoUi() {
  for (auto c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

PulseInfoUi* PulseInfoUi::add_to_stack(Gtk::Stack* stack, PulseManager* pm) {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/pulse_info.glade");

  PulseInfoUi* ui;

  builder->get_widget_derived("widgets_box", ui, pm);

  stack->add(*ui, "pulse_info");
  stack->child_property_icon_name(*ui).set_value("network-server-symbolic");

  return ui;
}

void PulseInfoUi::update_server_info() {
  server_name->set_text(pm->server_info.server_name);
  server_version->set_text(pm->server_info.server_version);
  default_sink->set_text(pm->server_info.default_sink_name);
  default_source->set_text(pm->server_info.default_source_name);
  protocol->set_text(pm->server_info.protocol);
  server_sample_format->set_text(pm->server_info.format);
  server_rate->set_text(std::to_string(pm->server_info.rate));
  server_channels->set_text(std::to_string(pm->server_info.channels));
  server_channel_mapping->set_text(pm->server_info.channel_map);
}

int PulseInfoUi::on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) {
  auto name1 = row1->get_name();
  auto name2 = row2->get_name();

  std::vector<std::string> names = {name1, name2};

  std::sort(names.begin(), names.end());

  if (name1 == names[0]) {
    return -1;
  } else if (name2 == names[0]) {
    return 1;
  } else {
    return 0;
  }
}

void PulseInfoUi::on_stack_visible_child_changed() {
  auto name = stack->get_visible_child_name();

  if (name == std::string("page_server")) {
    update_server_info();
  } else if (name == std::string("page_modules")) {
    auto children = listbox_modules->get_children();

    for (auto c : children) {
      listbox_modules->remove(*c);
    }

    pm->get_modules_info();
  } else if (name == std::string("page_clients")) {
    auto children = listbox_clients->get_children();

    for (auto c : children) {
      listbox_clients->remove(*c);
    }

    pm->get_clients_info();
  }
}

void PulseInfoUi::get_pulse_conf() {
  std::string command = "pulseaudio --dump-conf";

  try {
    boost::process::ipstream pipe_stream;
    boost::process::child c(command, boost::process::std_out > pipe_stream);

    std::string line;

    while (pipe_stream && std::getline(pipe_stream, line) && !line.empty()) {
      std::vector<std::string> aux;
      std::string key, value;

      boost::split(aux, line, boost::is_any_of("="));

      if (aux.size() > 1) {
        auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/pulse_conf_file_line.glade");

        Gtk::ListBoxRow* row;
        Gtk::Label *conf_key, *conf_value;

        b->get_widget("conf_row", row);
        b->get_widget("conf_key", conf_key);
        b->get_widget("conf_value", conf_value);

        row->set_name(aux[0]);
        conf_key->set_text(aux[0]);
        conf_value->set_text(aux[1]);

        listbox_config->add(*row);
      } else {
        boost::split(aux, line, boost::is_any_of(":"));

        if (aux.size() > 1) {
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

void PulseInfoUi::get_resamplers() {
  std::string command = "pulseaudio --dump-resample-methods";

  try {
    boost::process::ipstream pipe_stream;
    boost::process::child c(command, boost::process::std_out > pipe_stream);

    std::string line;

    while (pipe_stream && std::getline(pipe_stream, line) && !line.empty()) {
      auto row = Gtk::manage(new Gtk::ListBoxRow());
      auto label = Gtk::manage(new Gtk::Label());

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
