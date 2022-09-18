/*
 *  Copyright © 2017-2023 Wellington Wallace
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "multiband_compressor_ui.hpp"

namespace ui::multiband_compressor_box {

using namespace std::string_literals;

constexpr uint n_bands = 8U;

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  uint serial = 0;

  std::shared_ptr<MultibandCompressor> multiband_compressor;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _MultibandCompressorBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkStack* stack;

  GtkCheckButton *enable_band1, *enable_band2, *enable_band3, *enable_band4, *enable_band5, *enable_band6,
      *enable_band7;

  GtkSpinButton *dry, *wet;

  GtkComboBoxText *compressor_mode, *envelope_boost;

  GtkDropDown* dropdown_input_devices;

  GListStore* input_devices_model;

  GSettings* settings;

  std::array<ui::multiband_compressor_band_box::MultibandCompressorBandBox*, n_bands> bands;

  Data* data;
};

G_DEFINE_TYPE(MultibandCompressorBox, multiband_compressor_box, GTK_TYPE_BOX)

void on_reset(MultibandCompressorBox* self, GtkButton* btn) {
  util::reset_all_keys_except(self->settings);
}

void on_listbox_row_selected(MultibandCompressorBox* self, GtkListBoxRow* row, GtkListBox* listbox) {
  if (auto* selected_row = gtk_list_box_get_selected_row(listbox); selected_row != nullptr) {
    if (auto index = gtk_list_box_row_get_index(selected_row); index != -1) {
      gtk_stack_set_visible_child_name(self->stack, ("band" + util::to_string(index)).c_str());
    }
  }
}

void set_dropdown_input_devices_sensitivity(MultibandCompressorBox* self) {
  for (uint n = 0U; n < n_bands; n++) {
    if (g_settings_get_boolean(self->settings, tags::multiband_compressor::band_external_sidechain[n].data()) != 0) {
      gtk_widget_set_sensitive(GTK_WIDGET(self->dropdown_input_devices), 1);

      return;
    }
  }

  gtk_widget_set_sensitive(GTK_WIDGET(self->dropdown_input_devices), 0);
}

void create_bands(MultibandCompressorBox* self) {
  for (uint n = 0; n < n_bands; n++) {
    auto* band_box = ui::multiband_compressor_band_box::create();

    ui::multiband_compressor_band_box::setup(band_box, self->settings, n);

    gtk_stack_add_named(self->stack, GTK_WIDGET(band_box), ("band" + util::to_string(n)).c_str());

    self->bands[n] = band_box;

    self->data->gconnections.push_back(g_signal_connect(
        self->settings, ("changed::"s + tags::multiband_compressor::band_external_sidechain[n].data()).c_str(),
        G_CALLBACK(+[](GSettings* settings, char* key, MultibandCompressorBox* self) {
          set_dropdown_input_devices_sensitivity(self);
        }),
        self));
  }
}

void setup_dropdown_input_device(MultibandCompressorBox* self) {
  auto* selection = gtk_single_selection_new(G_LIST_MODEL(self->input_devices_model));

  g_signal_connect(self->dropdown_input_devices, "notify::selected-item",
                   G_CALLBACK(+[](GtkDropDown* dropdown, GParamSpec* pspec, MultibandCompressorBox* self) {
                     if (auto selected_item = gtk_drop_down_get_selected_item(dropdown); selected_item != nullptr) {
                       auto* holder = static_cast<ui::holders::NodeInfoHolder*>(selected_item);

                       g_settings_set_string(self->settings, "sidechain-input-device", holder->info->name.c_str());
                     }
                   }),
                   self);

  gtk_drop_down_set_model(self->dropdown_input_devices, G_LIST_MODEL(self->input_devices_model));

  g_object_unref(selection);
}

void setup(MultibandCompressorBox* self,
           std::shared_ptr<MultibandCompressor> multiband_compressor,
           const std::string& schema_path,
           PipeManager* pm) {
  self->data->multiband_compressor = multiband_compressor;

  auto serial = get_new_filter_serial();

  self->data->serial = serial;

  g_object_set_data(G_OBJECT(self), "serial", GUINT_TO_POINTER(serial));

  set_ignore_filter_idle_add(serial, false);

  self->settings = g_settings_new_with_path(tags::schema::multiband_compressor::id, schema_path.c_str());

  multiband_compressor->set_post_messages(true);

  setup_dropdown_input_device(self);

  set_dropdown_input_devices_sensitivity(self);

  create_bands(self);

  for (const auto& [serial, node] : pm->node_map) {
    if (node.name == tags::pipewire::ee_sink_name || node.name == tags::pipewire::ee_source_name) {
      continue;
    }

    if (node.media_class == tags::pipewire::media_class::source ||
        node.media_class == tags::pipewire::media_class::virtual_source ||
        node.media_role == tags::pipewire::media_role::dsp) {
      auto* holder = ui::holders::create(node);

      g_list_store_append(self->input_devices_model, holder);

      g_object_unref(holder);
    }
  }

  self->data->connections.push_back(multiband_compressor->input_level.connect([=](const float left, const float right) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                   self->input_level_right_label, left, right);
    });
  }));

  self->data->connections.push_back(
      multiband_compressor->output_level.connect([=](const float left, const float right) {
        util::idle_add([=]() {
          if (get_ignore_filter_idle_add(serial)) {
            return;
          }

          update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                       self->output_level_right_label, left, right);
        });
      }));

  self->data->connections.push_back(
      multiband_compressor->frequency_range.connect([=](const std::array<float, n_bands> values) {
        util::idle_add([=]() {
          if (get_ignore_filter_idle_add(serial)) {
            return;
          }

          for (size_t n = 0U; n < values.size(); n++) {
            ui::multiband_compressor_band_box::set_end_label(self->bands[n], values[n]);
          }
        });
      }));

  self->data->connections.push_back(
      multiband_compressor->envelope.connect([=](const std::array<float, n_bands> values) {
        util::idle_add([=]() {
          if (get_ignore_filter_idle_add(serial)) {
            return;
          }

          for (size_t n = 0U; n < values.size(); n++) {
            ui::multiband_compressor_band_box::set_envelope_label(self->bands[n], values[n]);
          }
        });
      }));

  self->data->connections.push_back(multiband_compressor->curve.connect([=](const std::array<float, n_bands> values) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      for (size_t n = 0U; n < values.size(); n++) {
        ui::multiband_compressor_band_box::set_curve_label(self->bands[n], values[n]);
      }
    });
  }));

  self->data->connections.push_back(
      multiband_compressor->reduction.connect([=](const std::array<float, n_bands> values) {
        util::idle_add([=]() {
          if (get_ignore_filter_idle_add(serial)) {
            return;
          }

          for (size_t n = 0U; n < values.size(); n++) {
            ui::multiband_compressor_band_box::set_gain_label(self->bands[n], values[n]);
          }
        });
      }));

  self->data->connections.push_back(pm->source_added.connect([=](const NodeInfo info) {
    for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->input_devices_model)); n++) {
      auto* holder =
          static_cast<ui::holders::NodeInfoHolder*>(g_list_model_get_item(G_LIST_MODEL(self->input_devices_model), n));

      if (holder->info->id == info.id) {
        g_object_unref(holder);

        return;
      }

      g_object_unref(holder);
    }

    auto holder = ui::holders::create(info);

    g_list_store_append(self->input_devices_model, holder);

    g_object_unref(holder);
  }));

  self->data->connections.push_back(pm->source_removed.connect([=](const NodeInfo info) {
    for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->input_devices_model)); n++) {
      auto* holder =
          static_cast<ui::holders::NodeInfoHolder*>(g_list_model_get_item(G_LIST_MODEL(self->input_devices_model), n));

      if (holder->info->id == info.id) {
        g_list_store_remove(self->input_devices_model, n);

        g_object_unref(holder);

        return;
      }

      g_object_unref(holder);
    }
  }));

  gsettings_bind_widgets<"input-gain", "output-gain">(self->settings, self->input_gain, self->output_gain);

  g_settings_bind(self->settings, "dry", gtk_spin_button_get_adjustment(self->dry), "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "wet", gtk_spin_button_get_adjustment(self->wet), "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "compressor-mode", self->compressor_mode, "active-id", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "envelope-boost", self->envelope_boost, "active-id", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "enable-band1", self->enable_band1, "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "enable-band2", self->enable_band2, "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "enable-band3", self->enable_band3, "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "enable-band4", self->enable_band4, "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "enable-band5", self->enable_band5, "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "enable-band6", self->enable_band6, "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "enable-band7", self->enable_band7, "active", G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_MULTIBAND_COMPRESSOR_BOX(object);

  self->data->multiband_compressor->set_post_messages(false);

  set_ignore_filter_idle_add(self->data->serial, true);

  for (auto& c : self->data->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->data->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  self->data->connections.clear();
  self->data->gconnections.clear();

  g_object_unref(self->settings);

  util::debug("disposed");

  G_OBJECT_CLASS(multiband_compressor_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_MULTIBAND_COMPRESSOR_BOX(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(multiband_compressor_box_parent_class)->finalize(object);
}

void multiband_compressor_box_class_init(MultibandCompressorBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::multiband_compressor_ui);

  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, stack);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, enable_band1);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, enable_band2);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, enable_band3);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, enable_band4);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, enable_band5);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, enable_band6);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, enable_band7);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, dry);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, wet);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, compressor_mode);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, envelope_boost);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, dropdown_input_devices);

  gtk_widget_class_bind_template_callback(widget_class, on_reset);
  gtk_widget_class_bind_template_callback(widget_class, on_listbox_row_selected);
}

void multiband_compressor_box_init(MultibandCompressorBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  self->input_devices_model = g_list_store_new(ui::holders::node_info_holder_get_type());

  // The following spinbuttons can assume -inf
  prepare_spinbuttons<"dB", false>(self->dry, self->wet);

  prepare_scales<"dB">(self->input_gain, self->output_gain);
}

auto create() -> MultibandCompressorBox* {
  return static_cast<MultibandCompressorBox*>(g_object_new(EE_TYPE_MULTIBAND_COMPRESSOR_BOX, nullptr));
}

}  // namespace ui::multiband_compressor_box
