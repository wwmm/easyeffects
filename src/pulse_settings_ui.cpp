#include "pulse_settings_ui.hpp"
#include "util.hpp"

namespace {

gboolean blocksize_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("64")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("128")) {
    g_value_set_int(value, 1);
  } else if (v == std::string("256")) {
    g_value_set_int(value, 2);
  } else if (v == std::string("512")) {
    g_value_set_int(value, 3);
  } else if (v == std::string("1024")) {
    g_value_set_int(value, 4);
  } else if (v == std::string("2048")) {
    g_value_set_int(value, 5);
  } else if (v == std::string("4096")) {
    g_value_set_int(value, 6);
  }

  return true;
}

GVariant* int_to_blocksize_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("64");
  } else if (v == 1) {
    return g_variant_new_string("128");
  } else if (v == 2) {
    return g_variant_new_string("256");
  } else if (v == 3) {
    return g_variant_new_string("512");
  } else if (v == 4) {
    return g_variant_new_string("1024");
  } else if (v == 5) {
    return g_variant_new_string("2048");
  } else {
    return g_variant_new_string("4096");
  }
}

}  // namespace

PulseSettingsUi::PulseSettingsUi(BaseObjectType* cobject,
                                 const Glib::RefPtr<Gtk::Builder>& builder,
                                 Application* application)
    : Gtk::Grid(cobject),
      settings(Gio::Settings::create("com.github.wwmm.pulseeffects")),
      sie_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs")),
      soe_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sourceoutputs")),
      app(application) {
  // loading glade widgets

  builder->get_widget("use_default_sink", use_default_sink);
  builder->get_widget("use_default_source", use_default_source);
  builder->get_widget("input_device", input_device);
  builder->get_widget("output_device", output_device);
  builder->get_widget("blocksize_in", blocksize_in);
  builder->get_widget("blocksize_out", blocksize_out);

  get_object(builder, "sie_input_buffer", sie_input_buffer);
  get_object(builder, "sie_input_latency", sie_input_latency);
  get_object(builder, "sie_output_buffer", sie_output_buffer);
  get_object(builder, "sie_output_latency", sie_output_latency);

  get_object(builder, "soe_input_buffer", soe_input_buffer);
  get_object(builder, "soe_input_latency", soe_input_latency);
  get_object(builder, "soe_output_buffer", soe_output_buffer);
  get_object(builder, "soe_output_latency", soe_output_latency);

  get_object(builder, "sink_list", sink_list);
  get_object(builder, "source_list", source_list);

  // signals connection

  use_default_sink->signal_toggled().connect(sigc::mem_fun(*this, &PulseSettingsUi::on_use_default_sink_toggled));
  use_default_source->signal_toggled().connect(sigc::mem_fun(*this, &PulseSettingsUi::on_use_default_source_toggled));

  connections.push_back(
      input_device->signal_changed().connect(sigc::mem_fun(*this, &PulseSettingsUi::on_input_device_changed)));
  connections.push_back(
      output_device->signal_changed().connect(sigc::mem_fun(*this, &PulseSettingsUi::on_output_device_changed)));

  app->pm->sink_added.connect(sigc::mem_fun(*this, &PulseSettingsUi::on_sink_added));
  app->pm->sink_removed.connect(sigc::mem_fun(*this, &PulseSettingsUi::on_sink_removed));
  app->pm->source_added.connect(sigc::mem_fun(*this, &PulseSettingsUi::on_source_added));
  app->pm->source_removed.connect(sigc::mem_fun(*this, &PulseSettingsUi::on_source_removed));

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;
  auto flag_invert_boolean = Gio::SettingsBindFlags::SETTINGS_BIND_INVERT_BOOLEAN;

  settings->bind("use-default-sink", use_default_sink, "active", flag);

  settings->bind("use-default-sink", output_device, "sensitive", flag | flag_invert_boolean);

  settings->bind("use-default-source", use_default_source, "active", flag);

  settings->bind("use-default-source", input_device, "sensitive", flag | flag_invert_boolean);

  sie_settings->bind("buffer-pulsesrc", sie_input_buffer.get(), "value", flag);
  sie_settings->bind("latency-pulsesrc", sie_input_latency.get(), "value", flag);
  sie_settings->bind("buffer-pulsesink", sie_output_buffer.get(), "value", flag);
  sie_settings->bind("latency-pulsesink", sie_output_latency.get(), "value", flag);

  soe_settings->bind("buffer-pulsesrc", soe_input_buffer.get(), "value", flag);
  soe_settings->bind("latency-pulsesrc", soe_input_latency.get(), "value", flag);
  soe_settings->bind("buffer-pulsesink", soe_output_buffer.get(), "value", flag);
  soe_settings->bind("latency-pulsesink", soe_output_latency.get(), "value", flag);

  g_settings_bind_with_mapping(settings->gobj(), "blocksize-in", blocksize_in->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, blocksize_enum_to_int, int_to_blocksize_enum, nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "blocksize-out", blocksize_out->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, blocksize_enum_to_int, int_to_blocksize_enum, nullptr, nullptr);
}

PulseSettingsUi::~PulseSettingsUi() {
  for (auto c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

void PulseSettingsUi::add_to_stack(Gtk::Stack* stack, Application* app) {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/pulse_settings.glade");

  PulseSettingsUi* ui;

  builder->get_widget_derived("widgets_grid", ui, app);

  stack->add(*ui, "settings_pulse", _("Pulseaudio"));
}

void PulseSettingsUi::on_sink_added(std::shared_ptr<mySinkInfo> info) {
  bool add_to_list = true;

  auto children = sink_list->children();

  for (auto c : children) {
    uint i;
    std::string name;

    c.get_value(0, i);
    c.get_value(1, name);

    if (info->index == i) {
      add_to_list = false;

      break;
    }
  }

  if (add_to_list) {
    Gtk::TreeModel::Row row = *(sink_list->append());

    row->set_value(0, info->index);
    row->set_value(1, info->name);

    if (use_default_sink->get_active()) {
      if (info->name == app->pm->server_info.default_sink_name) {
        output_device->set_active(row);
      }
    } else {
      auto custom_sink = settings->get_string("custom-sink");

      if (info->name == custom_sink) {
        output_device->set_active(row);
      }
    }

    util::debug(log_tag + "added sink: " + info->name);
  }
}

void PulseSettingsUi::on_sink_removed(uint idx) {
  Gtk::TreeIter remove_iter;
  std::string remove_name;

  auto children = sink_list->children();

  for (auto c : children) {
    uint i;
    std::string name;

    c.get_value(0, i);
    c.get_value(1, name);

    if (idx == i) {
      remove_iter = c;
      remove_name = name;

      sink_list->erase(remove_iter);

      util::debug(log_tag + "removed sink: " + remove_name);

      break;
    }
  }
}

void PulseSettingsUi::on_source_added(std::shared_ptr<mySourceInfo> info) {
  bool add_to_list = true;

  auto children = source_list->children();

  for (auto c : children) {
    uint i;
    std::string name;

    c.get_value(0, i);
    c.get_value(1, name);

    if (info->index == i) {
      add_to_list = false;

      break;
    }
  }

  if (add_to_list) {
    Gtk::TreeModel::Row row = *(source_list->append());

    row->set_value(0, info->index);
    row->set_value(1, info->name);

    if (use_default_source->get_active()) {
      if (info->name == app->pm->server_info.default_source_name) {
        input_device->set_active(row);
      }
    } else {
      auto custom_source = settings->get_string("custom-source");

      if (info->name == custom_source) {
        input_device->set_active(row);
      }
    }

    util::debug(log_tag + "added source: " + info->name);
  }
}

void PulseSettingsUi::on_source_removed(uint idx) {
  Gtk::TreeIter remove_iter;
  std::string remove_name;

  auto children = source_list->children();

  for (auto c : children) {
    uint i;
    std::string name;

    c.get_value(0, i);
    c.get_value(1, name);

    if (idx == i) {
      remove_iter = c;
      remove_name = name;

      source_list->erase(remove_iter);

      util::debug(log_tag + "removed source: " + remove_name);

      break;
    }
  }
}

void PulseSettingsUi::on_use_default_sink_toggled() {
  if (use_default_sink->get_active()) {
    auto children = sink_list->children();

    for (auto c : children) {
      std::string name;

      c.get_value(1, name);

      if (name == app->pm->server_info.default_sink_name) {
        output_device->set_active(c);
      }
    }
  }
}

void PulseSettingsUi::on_use_default_source_toggled() {
  if (use_default_source->get_active()) {
    auto children = source_list->children();

    for (auto c : children) {
      std::string name;

      c.get_value(1, name);

      if (name == app->pm->server_info.default_source_name) {
        input_device->set_active(c);
      }
    }
  }
}

void PulseSettingsUi::on_input_device_changed() {
  Gtk::TreeModel::Row row = *(input_device->get_active());

  if (row) {
    uint index;
    std::string name;

    row.get_value(0, index);
    row.get_value(1, name);

    app->soe->set_source_monitor_name(name);

    if (!use_default_source->get_active()) {
      settings->set_string("custom-source", name);
    }

    util::debug(log_tag + "input device changed: " + name);
  }
}

void PulseSettingsUi::on_output_device_changed() {
  Gtk::TreeModel::Row row = *(output_device->get_active());

  if (row) {
    uint index;
    std::string name;

    row.get_value(0, index);
    row.get_value(1, name);

    app->sie->set_output_sink_name(name);
    app->soe->webrtc->set_probe_src_device(name + ".monitor");

    if (!use_default_sink->get_active()) {
      settings->set_string("custom-sink", name);
    }

    util::debug(log_tag + "output device changed: " + name);
  }
}
