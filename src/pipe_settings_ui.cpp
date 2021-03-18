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

#include "pipe_settings_ui.hpp"
#include <cstring>
#include "util.hpp"

PipeSettingsUi::PipeSettingsUi(BaseObjectType* cobject,
                               const Glib::RefPtr<Gtk::Builder>& builder,
                               Application* application)
    : Gtk::Box(cobject),
      settings(Gio::Settings::create("com.github.wwmm.pulseeffects")),
      sie_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs")),
      soe_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.sourceoutputs")),
      app(application) {
  // loading glade widgets

  builder->get_widget("use_default_sink", use_default_sink);
  builder->get_widget("use_default_source", use_default_source);
  builder->get_widget("input_device", input_device);
  builder->get_widget("output_device", output_device);

  get_object(builder, "sie_latency", sie_latency);
  get_object(builder, "soe_latency", soe_latency);

  get_object(builder, "sink_list", sink_list);
  get_object(builder, "source_list", source_list);

  // initializing widgets

  for (const auto& node : app->pm->list_nodes) {
    if (node.name == "pulseeffects_sink" || node.name == "pulseeffects_source") {
      continue;
    }

    if (node.media_class == "Audio/Sink") {
      on_sink_added(node);
    } else if (node.media_class == "Audio/Source") {
      on_source_added(node);
    }
  }

  // signals connection

  use_default_sink->signal_toggled().connect(sigc::mem_fun(*this, &PipeSettingsUi::on_use_default_sink_toggled));

  use_default_source->signal_toggled().connect(sigc::mem_fun(*this, &PipeSettingsUi::on_use_default_source_toggled));

  app->pm->sink_added.connect(sigc::mem_fun(*this, &PipeSettingsUi::on_sink_added));
  app->pm->sink_removed.connect(sigc::mem_fun(*this, &PipeSettingsUi::on_sink_removed));
  app->pm->source_added.connect(sigc::mem_fun(*this, &PipeSettingsUi::on_source_added));
  app->pm->source_removed.connect(sigc::mem_fun(*this, &PipeSettingsUi::on_source_removed));

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;
  auto flag_invert_boolean = Gio::SettingsBindFlags::SETTINGS_BIND_INVERT_BOOLEAN;

  settings->bind("use-default-sink", use_default_sink, "active", flag);

  settings->bind("use-default-sink", output_device, "sensitive", flag | flag_invert_boolean);

  settings->bind("use-default-source", use_default_source, "active", flag);

  settings->bind("use-default-source", input_device, "sensitive", flag | flag_invert_boolean);

  sie_settings->bind("latency", sie_latency.get(), "value", flag);
  soe_settings->bind("latency", soe_latency.get(), "value", flag);

  // These connections have to come after the binding to gsettings.

  connections.emplace_back(
      input_device->signal_changed().connect(sigc::mem_fun(*this, &PipeSettingsUi::on_input_device_changed)));

  connections.emplace_back(
      output_device->signal_changed().connect(sigc::mem_fun(*this, &PipeSettingsUi::on_output_device_changed)));
}

PipeSettingsUi::~PipeSettingsUi() {
  for (auto& c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

void PipeSettingsUi::add_to_stack(Gtk::Stack* stack, Application* app) {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/pipe_settings.glade");

  PipeSettingsUi* ui = nullptr;

  builder->get_widget_derived("widgets_grid", ui, app);

  stack->add(*ui, "settings_pipe", _("PipeWire"));
}

void PipeSettingsUi::on_sink_added(const NodeInfo& info) {
  bool add_to_list = true;

  auto children = sink_list->children();

  for (const auto& c : children) {
    uint i = 0U;
    std::string name;

    c.get_value(0, i);
    c.get_value(1, name);

    if (info.id == i) {
      add_to_list = false;

      break;
    }
  }

  if (add_to_list) {
    Gtk::TreeModel::Row row = *(sink_list->append());

    row->set_value(0, info.id);
    row->set_value(1, info.name);

    if (use_default_sink->get_active()) {
      if (info.name == app->pm->default_sink.name) {
        output_device->set_active(row);
      }
    } else {
      auto custom_sink = settings->get_string("custom-sink");

      if (info.name == custom_sink) {
        output_device->set_active(row);
      }
    }
  }
}

void PipeSettingsUi::on_sink_removed(const NodeInfo& info) {
  Gtk::TreeIter remove_iter;
  std::string remove_name;

  auto children = sink_list->children();

  for (const auto& c : children) {
    uint i = 0U;
    std::string name;

    c.get_value(0, i);
    c.get_value(1, name);

    if (info.id == i) {
      remove_iter = c;
      remove_name = name;

      sink_list->erase(remove_iter);

      break;
    }
  }
}

void PipeSettingsUi::on_source_added(const NodeInfo& info) {
  bool add_to_list = true;

  auto children = source_list->children();

  for (const auto& c : children) {
    uint i = 0U;
    std::string name;

    c.get_value(0, i);
    c.get_value(1, name);

    if (info.id == i) {
      add_to_list = false;

      break;
    }
  }

  if (add_to_list) {
    Gtk::TreeModel::Row row = *(source_list->append());

    row->set_value(0, info.id);
    row->set_value(1, info.name);

    if (use_default_source->get_active()) {
      if (info.name == app->pm->default_source.name) {
        input_device->set_active(row);
      }
    } else {
      auto custom_source = settings->get_string("custom-source");

      if (info.name == custom_source) {
        input_device->set_active(row);
      }
    }
  }
}

void PipeSettingsUi::on_source_removed(const NodeInfo& info) {
  Gtk::TreeIter remove_iter;
  std::string remove_name;

  auto children = source_list->children();

  for (const auto& c : children) {
    uint i = 0U;
    std::string name;

    c.get_value(0, i);
    c.get_value(1, name);

    if (info.id == i) {
      remove_iter = c;
      remove_name = name;

      source_list->erase(remove_iter);

      break;
    }
  }
}

void PipeSettingsUi::on_use_default_sink_toggled() {
  if (use_default_sink->get_active()) {
    auto default_sink = app->pm->default_sink.name;

    auto children = sink_list->children();

    for (const auto& c : children) {
      std::string name;

      c.get_value(1, name);

      if (name == default_sink) {
        output_device->set_active(c);
      }
    }
  }
}

void PipeSettingsUi::on_use_default_source_toggled() {
  if (use_default_source->get_active()) {
    auto default_source = app->pm->default_source.name;

    auto children = source_list->children();

    for (const auto& c : children) {
      std::string name;

      c.get_value(1, name);

      if (name == default_source) {
        input_device->set_active(c);
      }
    }
  }
}

void PipeSettingsUi::on_input_device_changed() {
  Gtk::TreeModel::Row row = *(input_device->get_active());

  if (row) {
    uint index = 0U;
    std::string name;

    row.get_value(0, index);
    row.get_value(1, name);

    for (const auto& node : app->soe->pm->list_nodes) {
      if (node.name == name) {
        app->sie->change_input_device(node);

        break;
      }
    }

    if (!use_default_source->get_active()) {
      settings->set_string("custom-source", name);
    }

    util::debug(log_tag + "selected input device changed: " + name);
  }
}

void PipeSettingsUi::on_output_device_changed() {
  Gtk::TreeModel::Row row = *(output_device->get_active());

  if (row) {
    uint index = 0U;
    std::string name;

    row.get_value(0, index);
    row.get_value(1, name);

    for (const auto& node : app->soe->pm->list_nodes) {
      if (node.name == name) {
        app->soe->change_output_device(node);

        app->sie->webrtc->set_probe_input_node_id(node.id);

        break;
      }
    }

    if (!use_default_sink->get_active()) {
      settings->set_string("custom-sink", name);
    }

    util::debug(log_tag + "selected output device changed: " + name);
  }
}
