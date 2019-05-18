#include "pulse_info_ui.hpp"
#include "util.hpp"

PulseInfoUi::PulseInfoUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& refBuilder)
    : Gtk::Box(cobject) {}

PulseInfoUi::~PulseInfoUi() {
  util::debug(log_tag + "destroyed");
}

PulseInfoUi* PulseInfoUi::add_to_stack(Gtk::Stack* stack) {
  auto builder = Gtk::Builder::create_from_resource(
      "/com/github/wwmm/pulseeffects/ui/pulse_info.glade");

  PulseInfoUi* ui;

  builder->get_widget_derived("widgets_box", ui);

  stack->add(*ui, "pulse_info");
  stack->child_property_icon_name(*ui).set_value("audio-card-symbolic");

  return ui;
}
