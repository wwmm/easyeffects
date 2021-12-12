/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "multiband_compressor_ui.hpp"

namespace ui::multiband_compressor_box {

using namespace std::string_literals;

auto constexpr log_tag = "multiband_compressor_box: ";

constexpr uint n_bands = 8U;

struct Data {
 public:
  ~Data() { util::debug(log_tag + "data struct destroyed"s); }

  std::shared_ptr<MultibandCompressor> multiband_compressor;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _MultibandCompressorBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkToggleButton* bypass;

  GtkComboBoxText *compressor_mode, *envelope_boost;

  GtkDropDown* dropdown_input_devices;

  GListStore* input_devices_model;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(MultibandCompressorBox, multiband_compressor_box, GTK_TYPE_BOX)

void on_bypass(MultibandCompressorBox* self, GtkToggleButton* btn) {
  self->data->multiband_compressor->bypass = gtk_toggle_button_get_active(btn);
}

void on_reset(MultibandCompressorBox* self, GtkButton* btn) {
  gtk_toggle_button_set_active(self->bypass, 0);

  g_settings_reset(self->settings, "input-gain");

  g_settings_reset(self->settings, "output-gain");

  g_settings_reset(self->settings, "compressor-mode");

  g_settings_reset(self->settings, "envelope-boost");

  g_settings_reset(self->settings, "sidechain-input-device");

  for (uint n = 0U; n < n_bands; n++) {
    const auto nstr = std::to_string(n);

    if (n > 0U) {
      g_settings_reset(self->settings, ("enable-band" + nstr).c_str());

      g_settings_reset(self->settings, ("split-frequency" + nstr).c_str());
    }

    g_settings_reset(self->settings, ("compressor-enable" + nstr).c_str());

    g_settings_reset(self->settings, ("solo" + nstr).c_str());

    g_settings_reset(self->settings, ("mute" + nstr).c_str());

    g_settings_reset(self->settings, ("attack-threshold" + nstr).c_str());

    g_settings_reset(self->settings, ("attack-time" + nstr).c_str());

    g_settings_reset(self->settings, ("release-threshold" + nstr).c_str());

    g_settings_reset(self->settings, ("release-time" + nstr).c_str());

    g_settings_reset(self->settings, ("ratio" + nstr).c_str());

    g_settings_reset(self->settings, ("knee" + nstr).c_str());

    g_settings_reset(self->settings, ("makeup" + nstr).c_str());

    g_settings_reset(self->settings, ("compression-mode" + nstr).c_str());

    g_settings_reset(self->settings, ("external-sidechain" + nstr).c_str());

    g_settings_reset(self->settings, ("sidechain-mode" + nstr).c_str());

    g_settings_reset(self->settings, ("sidechain-source" + nstr).c_str());

    g_settings_reset(self->settings, ("sidechain-lookahead" + nstr).c_str());

    g_settings_reset(self->settings, ("sidechain-reactivity" + nstr).c_str());

    g_settings_reset(self->settings, ("sidechain-preamp" + nstr).c_str());

    g_settings_reset(self->settings, ("sidechain-custom-lowcut-filter" + nstr).c_str());

    g_settings_reset(self->settings, ("sidechain-custom-highcut-filter" + nstr).c_str());

    g_settings_reset(self->settings, ("sidechain-lowcut-frequency" + nstr).c_str());

    g_settings_reset(self->settings, ("sidechain-highcut-frequency" + nstr).c_str());

    g_settings_reset(self->settings, ("boost-threshold" + nstr).c_str());

    g_settings_reset(self->settings, ("boost-amount" + nstr).c_str());
  }
}

void setup_dropdown_input_device(MultibandCompressorBox* self) {
  auto* selection = gtk_single_selection_new(G_LIST_MODEL(self->input_devices_model));

  g_signal_connect(self->dropdown_input_devices, "notify::selected-item",
                   G_CALLBACK(+[](GtkDropDown* dropdown, GParamSpec* pspec, MultibandCompressorBox* self) {
                     if (auto selected_item = gtk_drop_down_get_selected_item(dropdown); selected_item != nullptr) {
                       auto* holder = static_cast<ui::holders::NodeInfoHolder*>(selected_item);

                       g_settings_set_string(self->settings, "sidechain-input-device", holder->name.c_str());
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

  self->settings = g_settings_new_with_path("com.github.wwmm.easyeffects.multibandcompressor", schema_path.c_str());

  multiband_compressor->post_messages = true;
  multiband_compressor->bypass = false;

  setup_dropdown_input_device(self);

  for (const auto& [ts, node] : pm->node_map) {
    if (node.name == pm->ee_sink_name || node.name == pm->ee_source_name) {
      continue;
    }

    if (node.media_class == pm->media_class_source || node.media_class == pm->media_class_virtual_source) {
      g_list_store_append(self->input_devices_model, ui::holders::create(node));
    }
  }

  self->data->connections.push_back(
      multiband_compressor->input_level.connect([=](const float& left, const float& right) {
        update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                     self->input_level_right_label, left, right);
      }));

  self->data->connections.push_back(
      multiband_compressor->output_level.connect([=](const float& left, const float& right) {
        update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                     self->output_level_right_label, left, right);
      }));

  self->data->connections.push_back(pm->source_added.connect([=](const NodeInfo info) {
    for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->input_devices_model)); n++) {
      if (static_cast<ui::holders::NodeInfoHolder*>(g_list_model_get_item(G_LIST_MODEL(self->input_devices_model), n))
              ->id == info.id) {
        return;
      }
    }

    g_list_store_append(self->input_devices_model, ui::holders::create(info));
  }));

  self->data->connections.push_back(pm->source_removed.connect([=](const NodeInfo info) {
    for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->input_devices_model)); n++) {
      if (static_cast<ui::holders::NodeInfoHolder*>(g_list_model_get_item(G_LIST_MODEL(self->input_devices_model), n))
              ->id == info.id) {
        g_list_store_remove(self->input_devices_model, n);

        return;
      }
    }
  }));

  g_settings_bind(self->settings, "input-gain", gtk_range_get_adjustment(GTK_RANGE(self->input_gain)), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "output-gain", gtk_range_get_adjustment(GTK_RANGE(self->output_gain)), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "compressor-mode", self->compressor_mode, "active-id", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "envelope-boost", self->envelope_boost, "active-id", G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_MULTIBAND_COMPRESSOR_BOX(object);

  self->data->multiband_compressor->bypass = false;

  for (auto& c : self->data->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->data->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  self->data->connections.clear();
  self->data->gconnections.clear();

  g_object_unref(self->settings);

  util::debug(log_tag + "disposed"s);

  G_OBJECT_CLASS(multiband_compressor_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_MULTIBAND_COMPRESSOR_BOX(object);

  delete self->data;

  util::debug(log_tag + "finalized"s);

  G_OBJECT_CLASS(multiband_compressor_box_parent_class)->finalize(object);
}

void multiband_compressor_box_class_init(MultibandCompressorBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/multiband_compressor.ui");

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

  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, bypass);

  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, compressor_mode);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, envelope_boost);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBox, dropdown_input_devices);

  gtk_widget_class_bind_template_callback(widget_class, on_bypass);
  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void multiband_compressor_box_init(MultibandCompressorBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  self->input_devices_model = g_list_store_new(ui::holders::node_info_holder_get_type());

  prepare_scale<"dB">(self->input_gain);
  prepare_scale<"dB">(self->output_gain);
}

auto create() -> MultibandCompressorBox* {
  return static_cast<MultibandCompressorBox*>(g_object_new(EE_TYPE_MULTIBAND_COMPRESSOR_BOX, nullptr));
}

}  // namespace ui::multiband_compressor_box

namespace {

auto compression_mode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (g_strcmp0(v, "Downward") == 0) {
    g_value_set_int(value, 0);
  } else if (g_strcmp0(v, "Upward") == 0) {
    g_value_set_int(value, 1);
  } else if (g_strcmp0(v, "Boosting") == 0) {
    g_value_set_int(value, 2);
  }

  return 1;
}

auto int_to_compression_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  switch (g_value_get_int(value)) {
    case 0:
      return g_variant_new_string("Downward");

    case 1:
      return g_variant_new_string("Upward");

    case 2:
      return g_variant_new_string("Boosting");

    default:
      return g_variant_new_string("Downward");
  }
}

auto sidechain_mode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (g_strcmp0(v, "Peak") == 0) {
    g_value_set_int(value, 0);
  } else if (g_strcmp0(v, "RMS") == 0) {
    g_value_set_int(value, 1);
  } else if (g_strcmp0(v, "Low-Pass") == 0) {
    g_value_set_int(value, 2);
  } else if (g_strcmp0(v, "Uniform") == 0) {
    g_value_set_int(value, 3);
  }

  return 1;
}

auto int_to_sidechain_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  switch (g_value_get_int(value)) {
    case 0:
      return g_variant_new_string("Peak");

    case 1:
      return g_variant_new_string("RMS");

    case 2:
      return g_variant_new_string("Low-Pass");

    case 3:
      return g_variant_new_string("Uniform");

    default:
      return g_variant_new_string("RMS");
  }
}

auto sidechain_source_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const auto* v = g_variant_get_string(variant, nullptr);

  if (g_strcmp0(v, "Middle") == 0) {
    g_value_set_int(value, 0);
  } else if (g_strcmp0(v, "Side") == 0) {
    g_value_set_int(value, 1);
  } else if (g_strcmp0(v, "Left") == 0) {
    g_value_set_int(value, 2);
  } else if (g_strcmp0(v, "Right") == 0) {
    g_value_set_int(value, 3);
  }

  return 1;
}

auto int_to_sidechain_source_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  switch (g_value_get_int(value)) {
    case 0:
      return g_variant_new_string("Middle");

    case 1:
      return g_variant_new_string("Side");

    case 2:
      return g_variant_new_string("Left");

    case 3:
      return g_variant_new_string("Right");

    default:
      return g_variant_new_string("Middle");
  }
}

}  // namespace

MultibandCompressorUi::MultibandCompressorUi(BaseObjectType* cobject,
                                             const Glib::RefPtr<Gtk::Builder>& builder,
                                             const std::string& schema,
                                             const std::string& schema_path)
    : Gtk::Box(cobject),
      PluginUiBase(builder, schema, schema_path),
      input_devices_model(Gio::ListStore<NodeInfoHolder>::create()) {
  name = plugin_name::multiband_compressor;

  // loading builder widgets

  stack = builder->get_widget<Gtk::Stack>("stack");

  listbox = builder->get_widget<Gtk::ListBox>("listbox");

  listbox->select_row(*listbox->get_row_at_index(0));

  listbox->signal_selected_rows_changed().connect([=, this]() {
    // some core dumps happened here
    // checking pointer and int row integrity might fix them

    if (const auto* selected_row = listbox->get_selected_row(); selected_row != nullptr) {
      if (const auto row = selected_row->get_index(); row > -1) {
        stack->set_visible_child("band" + std::to_string(row));
      }
    }
  });

  set_dropdown_input_devices_sensitivity();

  // setup band checkbuttons

  for (uint n = 1U; n < n_bands; n++) {
    const auto nstr = std::to_string(n);

    // auto* const enable_band = builder->get_widget<Gtk::CheckButton>("enable_band"+ nstr).c_str());

    // settings->bind("enable-band"+ nstr).c_str(), enable_band, "active");
  }

  prepare_bands();
}

MultibandCompressorUi::~MultibandCompressorUi() {
  util::debug(name + " ui destroyed");
}

void MultibandCompressorUi::prepare_bands() {
  for (uint n = 0U; n < n_bands; n++) {
    const auto nstr = std::to_string(n);

    const auto builder =
        Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/multiband_compressor_band.ui");

    if (n > 0U) {
      auto* const split_frequency = builder->get_widget<Gtk::SpinButton>("split_frequency");

      // settings->bind("split-frequency"+ nstr).c_str(), split_frequency->get_adjustment().get(), "value");

      prepare_spinbutton(split_frequency, "Hz");
    } else {
      // removing split frequency from band 0

      auto* const sf_box = builder->get_widget<Gtk::Box>("split_frequency_box");

      for (auto* child = sf_box->get_last_child(); child != nullptr; child = sf_box->get_last_child()) {
        sf_box->remove(*child);
      }

      auto sf_label = Gtk::Label("0 Hz", Gtk::Align::CENTER);

      sf_box->append(sf_label);
    }

    // loading builder widgets

    bands_end.at(n) = builder->get_widget<Gtk::Label>("band_end");

    bands_gain_label.at(n) = builder->get_widget<Gtk::Label>("band_gain_label");

    bands_envelope_label.at(n) = builder->get_widget<Gtk::Label>("band_envelope_label");

    bands_curve_label.at(n) = builder->get_widget<Gtk::Label>("band_curve_label");

    auto* const band_bypass = builder->get_widget<Gtk::ToggleButton>("bypass");

    auto* const mute = builder->get_widget<Gtk::ToggleButton>("mute");

    auto* const solo = builder->get_widget<Gtk::ToggleButton>("solo");

    auto* const lowcut_filter = builder->get_widget<Gtk::CheckButton>("lowcut_filter");

    auto* const highcut_filter = builder->get_widget<Gtk::CheckButton>("highcut_filter");

    auto* const lowcut_filter_frequency = builder->get_widget<Gtk::SpinButton>("lowcut_filter_frequency");

    auto* const highcut_filter_frequency = builder->get_widget<Gtk::SpinButton>("highcut_filter_frequency");

    auto* const attack_time = builder->get_widget<Gtk::SpinButton>("attack_time");

    auto* const attack_threshold = builder->get_widget<Gtk::SpinButton>("attack_threshold");

    auto* const release_time = builder->get_widget<Gtk::SpinButton>("release_time");

    auto* const release_threshold = builder->get_widget<Gtk::SpinButton>("release_threshold");

    auto* const ratio = builder->get_widget<Gtk::SpinButton>("ratio");

    auto* const knee = builder->get_widget<Gtk::SpinButton>("knee");

    auto* const makeup = builder->get_widget<Gtk::SpinButton>("makeup");

    auto* const sidechain_preamp = builder->get_widget<Gtk::SpinButton>("sidechain_preamp");

    auto* const sidechain_reactivity = builder->get_widget<Gtk::SpinButton>("sidechain_reactivity");

    auto* const sidechain_lookahead = builder->get_widget<Gtk::SpinButton>("sidechain_lookahead");

    auto* const boost_amount = builder->get_widget<Gtk::SpinButton>("boost_amount");

    auto* const boost_threshold = builder->get_widget<Gtk::SpinButton>("boost_threshold");

    auto* const compression_mode = builder->get_widget<Gtk::ComboBoxText>("compression_mode");

    auto* const sidechain_mode = builder->get_widget<Gtk::ComboBoxText>("sidechain_mode");

    auto* const sidechain_source = builder->get_widget<Gtk::ComboBoxText>("sidechain_source");

    auto* const external_sidechain = builder->get_widget<Gtk::ToggleButton>("external_sidechain");

    // gsettings bindings

    // settings->bind("compressor-enable"+ nstr).c_str(), band_bypass, "active",
    // Gio::Settings::BindFlags::INVERT_BOOLEAN);

    // settings->bind("mute"+ nstr).c_str(), mute, "active");

    // settings->bind("solo"+ nstr).c_str(), solo, "active");

    // settings->bind("sidechain-custom-lowcut-filter"+ nstr).c_str(), lowcut_filter, "active");

    // settings->bind("sidechain-custom-highcut-filter"+ nstr).c_str(), highcut_filter, "active");

    // settings->bind("sidechain-lowcut-frequency"+ nstr).c_str(), lowcut_filter_frequency->get_adjustment().get(),
    // "value");

    // settings->bind("sidechain-highcut-frequency"+ nstr).c_str(), highcut_filter_frequency->get_adjustment().get(),
    // "value");

    // settings->bind("attack-time"+ nstr).c_str(), attack_time->get_adjustment().get(), "value");

    // settings->bind("attack-threshold"+ nstr).c_str(), attack_threshold->get_adjustment().get(), "value");

    // settings->bind("release-time"+ nstr).c_str(), release_time->get_adjustment().get(), "value");

    // settings->bind("release-threshold"+ nstr).c_str(), release_threshold->get_adjustment().get(), "value");

    // settings->bind("ratio"+ nstr).c_str(), ratio->get_adjustment().get(), "value");

    // settings->bind("knee"+ nstr).c_str(), knee->get_adjustment().get(), "value");

    // settings->bind("makeup"+ nstr).c_str(), makeup->get_adjustment().get(), "value");

    // settings->bind("external-sidechain"+ nstr).c_str(), external_sidechain, "active");

    // settings->bind("sidechain-preamp"+ nstr).c_str(), sidechain_preamp->get_adjustment().get(), "value");

    // settings->bind("sidechain-reactivity"+ nstr).c_str(), sidechain_reactivity->get_adjustment().get(), "value");

    // settings->bind("sidechain-lookahead"+ nstr).c_str(), sidechain_lookahead->get_adjustment().get(), "value");

    // settings->bind("boost-amount"+ nstr).c_str(), boost_amount->get_adjustment().get(), "value");

    // settings->bind("boost-threshold"+ nstr).c_str(), boost_threshold->get_adjustment().get(), "value");

    // g_settings_bind_with_mapping(settings->gobj(), std::string("compression-mode"+ nstr).c_str()).c_str(),
    //                              compression_mode->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
    //                              compression_mode_enum_to_int, int_to_compression_mode_enum, nullptr, nullptr);

    // g_settings_bind_with_mapping(settings->gobj(), std::string("sidechain-mode"+ nstr).c_str()).c_str(),
    // sidechain_mode->gobj(),
    //                              "active", G_SETTINGS_BIND_DEFAULT, sidechain_mode_enum_to_int,
    //                              int_to_sidechain_mode_enum, nullptr, nullptr);

    // g_settings_bind_with_mapping(settings->gobj(), std::string("sidechain-source"+ nstr).c_str()).c_str(),
    //                              sidechain_source->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
    //                              sidechain_source_enum_to_int, int_to_sidechain_source_enum, nullptr, nullptr);

    // connections.push_back(settings->signal_changed("external-sidechain"+ nstr).c_str()).connect([=, this](const auto&
    // key) {
    //   set_dropdown_input_devices_sensitivity();
    // }));

    // prepare widgets

    prepare_spinbutton(lowcut_filter_frequency, "Hz");

    prepare_spinbutton(highcut_filter_frequency, "Hz");

    prepare_spinbutton(attack_time, "ms");

    prepare_spinbutton(attack_threshold, "db");

    prepare_spinbutton(release_time, "ms");

    prepare_spinbutton(release_threshold, "db");

    prepare_spinbutton(ratio);

    prepare_spinbutton(knee, "db");

    prepare_spinbutton(makeup, "db");

    prepare_spinbutton(sidechain_preamp, "db");

    prepare_spinbutton(sidechain_reactivity, "ms");

    prepare_spinbutton(sidechain_lookahead, "ms");

    prepare_spinbutton(boost_amount, "db");

    prepare_spinbutton(boost_threshold, "db");

    // set boost spinbuttons sensitivity on compression mode

    auto set_boost_spinbuttons_sensitivity = [=, this]() {
      const auto row_id = compression_mode->get_active_id();

      if (row_id == "downward_mode") {
        boost_threshold->set_sensitive(false);
        boost_amount->set_sensitive(false);
      } else if (row_id == "upward_mode") {
        boost_threshold->set_sensitive(true);
        boost_amount->set_sensitive(false);
      } else if (row_id == "boosting_mode") {
        boost_threshold->set_sensitive(false);
        boost_amount->set_sensitive(true);
      } else {
        boost_threshold->set_sensitive(true);
        boost_amount->set_sensitive(true);
      }
    };

    set_boost_spinbuttons_sensitivity();

    compression_mode->signal_changed().connect(set_boost_spinbuttons_sensitivity);

    // add to stack

    auto* const top_box = builder->get_widget<Gtk::Box>("top_box");

    // stack->add(*top_box, "band"+ nstr).c_str());
  }
}

void MultibandCompressorUi::on_new_frequency_range(const std::array<float, n_bands>& values) {
  for (size_t n = 0U; n < values.size(); n++) {
    bands_end.at(n)->set_text(level_to_localized_string(values.at(n), 0));
  }
}

void MultibandCompressorUi::on_new_envelope(const std::array<float, n_bands>& values) {
  for (size_t n = 0U; n < values.size(); n++) {
    bands_envelope_label.at(n)->set_text(level_to_localized_string(util::linear_to_db(values.at(n)), 0));
  }
}

void MultibandCompressorUi::on_new_curve(const std::array<float, n_bands>& values) {
  for (size_t n = 0U; n < values.size(); n++) {
    bands_curve_label.at(n)->set_text(level_to_localized_string(util::linear_to_db(values.at(n)), 0));
  }
}

void MultibandCompressorUi::on_new_reduction(const std::array<float, n_bands>& values) {
  for (size_t n = 0U; n < values.size(); n++) {
    bands_gain_label.at(n)->set_text(level_to_localized_string(util::linear_to_db(values.at(n)), 0));
  }
}

void MultibandCompressorUi::set_dropdown_input_devices_sensitivity() {
  for (uint n = 0U; n < n_bands; n++) {
    if (settings->get_boolean("external-sidechain" + std::to_string(n))) {
      dropdown_input_devices->set_sensitive(true);

      return;
    }
  }

  dropdown_input_devices->set_sensitive(false);
}
