#include "pulse_info_ui.hpp"
#include "util.hpp"

PulseInfoUi::PulseInfoUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         PulseManager* pm_ptr)
    : Gtk::Box(cobject), pm(pm_ptr) {
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

  listbox_modules->set_sort_func(
      sigc::mem_fun(*this, &PulseInfoUi::on_listbox_sort));

  connections.push_back(
      pm->server_changed.connect([=]() { update_server_info(); }));

  connections.push_back(pm->module_info.connect([=](auto info) {
    auto b = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/ui/module_info.glade");

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

  update_server_info();

  pm->get_modules_info();
}

PulseInfoUi::~PulseInfoUi() {
  for (auto c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

PulseInfoUi* PulseInfoUi::add_to_stack(Gtk::Stack* stack, PulseManager* pm) {
  auto builder = Gtk::Builder::create_from_resource(
      "/com/github/wwmm/pulseeffects/ui/pulse_info.glade");

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
