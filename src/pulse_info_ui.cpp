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

  server_name->set_text(pm->server_info.server_name);
  server_version->set_text(pm->server_info.server_version);
  default_sink->set_text(pm->server_info.default_sink_name);
  default_source->set_text(pm->server_info.default_source_name);
  protocol->set_text(pm->server_info.protocol);
}

PulseInfoUi::~PulseInfoUi() {
  util::debug(log_tag + "destroyed");
}

PulseInfoUi* PulseInfoUi::add_to_stack(Gtk::Stack* stack, PulseManager* pm) {
  auto builder = Gtk::Builder::create_from_resource(
      "/com/github/wwmm/pulseeffects/ui/pulse_info.glade");

  PulseInfoUi* ui;

  builder->get_widget_derived("widgets_box", ui, pm);

  stack->add(*ui, "pulse_info");
  stack->child_property_icon_name(*ui).set_value("audio-card-symbolic");

  return ui;
}
