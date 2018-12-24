#include "pulse_settings_ui.hpp"
#include "util.hpp"

PulseSettingsUi::PulseSettingsUi(BaseObjectType* cobject,
                                 const Glib::RefPtr<Gtk::Builder>& builder,
                                 const Glib::RefPtr<Gio::Settings>& refSettings,
                                 Application* application)
    : Gtk::Grid(cobject), settings(refSettings), app(application) {
  // loading glade widgets

  builder->get_widget("use_default_sink", use_default_sink);
  builder->get_widget("use_default_source", use_default_source);
  builder->get_widget("input_device", input_device);
  builder->get_widget("output_device", output_device);
  builder->get_widget("blocksize_in", blocksize_in);
  builder->get_widget("blocksize_out", blocksize_out);

  get_object(builder, "buffer_in", buffer_in);
  get_object(builder, "buffer_out", buffer_out);
  get_object(builder, "latency_in", latency_in);
  get_object(builder, "latency_out", latency_out);
  get_object(builder, "sink_list", sink_list);
  get_object(builder, "source_list", source_list);

  // signals connection

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  // settings->bind("show-spectrum", show_spectrum, "active", flag);
}

PulseSettingsUi::~PulseSettingsUi() {
  for (auto c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}
