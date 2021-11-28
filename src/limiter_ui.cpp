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

#include "limiter_ui.hpp"

namespace ui::limiter_box {

using namespace std::string_literals;

auto constexpr log_tag = "limiter_box: ";

struct _LimiterBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkToggleButton* bypass;

  GtkComboBoxText *mode, *oversampling, *dither;

  GtkDropDown* dropdown_input_devices;

  GtkLabel *gain_left, *gain_right, *sidechain_left, *sidechain_right;

  GtkSpinButton *sc_preamp, *lookahead, *attack, *release, *threshold, *stereo_link, *alr_attack, *alr_release,
      *alr_knee;

  GtkCheckButton* gain_boost;

  GtkToggleButton *alr, *external_sidechain;

  GListStore* input_devices_model;

  GSettings* settings;

  std::shared_ptr<Limiter> limiter;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

G_DEFINE_TYPE(LimiterBox, limiter_box, GTK_TYPE_BOX)

void on_bypass(LimiterBox* self, GtkToggleButton* btn) {
  self->limiter->bypass = gtk_toggle_button_get_active(btn);
}

void on_reset(LimiterBox* self, GtkButton* btn) {
  gtk_toggle_button_set_active(self->bypass, 0);

  g_settings_reset(self->settings, "input-gain");

  g_settings_reset(self->settings, "output-gain");

  g_settings_reset(self->settings, "mode");

  g_settings_reset(self->settings, "oversampling");

  g_settings_reset(self->settings, "dither");

  g_settings_reset(self->settings, "sidechain-preamp");

  g_settings_reset(self->settings, "lookahead");

  g_settings_reset(self->settings, "attack");

  g_settings_reset(self->settings, "release");

  g_settings_reset(self->settings, "threshold");

  g_settings_reset(self->settings, "gain-boost");

  g_settings_reset(self->settings, "stereo-link");

  g_settings_reset(self->settings, "alr");

  g_settings_reset(self->settings, "alr-attack");

  g_settings_reset(self->settings, "alr-release");

  g_settings_reset(self->settings, "alr-knee");

  g_settings_reset(self->settings, "external-sidechain");

  g_settings_reset(self->settings, "sidechain-input-device");
}

void setup_dropdown_input_device(LimiterBox* self) {
  auto* factory = gtk_signal_list_item_factory_new();

  // setting the factory callbacks

  g_signal_connect(factory, "setup",
                   G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, LimiterBox* self) {
                     auto* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
                     auto* label = gtk_label_new(nullptr);
                     auto* icon = gtk_image_new_from_icon_name("audio-input-microphone-symbolic");

                     gtk_widget_set_halign(GTK_WIDGET(label), GTK_ALIGN_START);
                     gtk_widget_set_hexpand(GTK_WIDGET(label), 1);

                     gtk_box_append(GTK_BOX(box), GTK_WIDGET(icon));
                     gtk_box_append(GTK_BOX(box), GTK_WIDGET(label));

                     gtk_list_item_set_child(item, GTK_WIDGET(box));

                     g_object_set_data(G_OBJECT(item), "name", label);
                   }),
                   self);

  g_signal_connect(factory, "bind",
                   G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, LimiterBox* self) {
                     auto* label = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "name"));

                     auto* holder = static_cast<ui::holders::NodeInfoHolder*>(gtk_list_item_get_item(item));

                     gtk_label_set_text(label, holder->name.c_str());
                   }),
                   self);

  gtk_drop_down_set_factory(self->dropdown_input_devices, factory);

  g_object_unref(factory);

  /*
    DropDowns know how to deal with GtkStringList. But we are passing a custom holder and no expression was set. So
    we have to set the model after configuring the factory. Why this was not a problem with gtkmm I have no idea...
  */

  auto* selection = gtk_single_selection_new(G_LIST_MODEL(self->input_devices_model));

  gtk_drop_down_set_model(self->dropdown_input_devices, G_LIST_MODEL(self->input_devices_model));

  g_object_unref(selection);
}

void setup(LimiterBox* self, std::shared_ptr<Limiter> limiter, const std::string& schema_path, PipeManager* pm) {
  self->limiter = limiter;

  self->settings = g_settings_new_with_path("com.github.wwmm.easyeffects.limiter", schema_path.c_str());

  limiter->post_messages = true;
  limiter->bypass = false;

  setup_dropdown_input_device(self);

  for (const auto& [ts, node] : pm->node_map) {
    if (node.name == pm->ee_sink_name || node.name == pm->ee_source_name) {
      continue;
    }

    if (node.media_class == pm->media_class_source || node.media_class == pm->media_class_virtual_source) {
      g_list_store_append(self->input_devices_model, ui::holders::create(node));
    }
  }

  self->connections.push_back(limiter->input_level.connect([=](const float& left, const float& right) {
    update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                 self->input_level_right_label, left, right);
  }));

  self->connections.push_back(limiter->output_level.connect([=](const float& left, const float& right) {
    update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                 self->output_level_right_label, left, right);
  }));

  self->connections.push_back(limiter->gain_left.connect([=](const double& value) {
    gtk_label_set_text(self->gain_left, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
  }));

  self->connections.push_back(limiter->gain_right.connect([=](const double& value) {
    gtk_label_set_text(self->gain_right, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
  }));

  self->connections.push_back(limiter->sidechain_left.connect([=](const double& value) {
    gtk_label_set_text(self->sidechain_left, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
  }));

  self->connections.push_back(limiter->sidechain_right.connect([=](const double& value) {
    gtk_label_set_text(self->sidechain_right, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
  }));

  self->connections.push_back(pm->source_added.connect([=](const NodeInfo info) {
    for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->input_devices_model)); n++) {
      if (static_cast<ui::holders::NodeInfoHolder*>(g_list_model_get_item(G_LIST_MODEL(self->input_devices_model), n))
              ->id == info.id) {
        return;
      }
    }

    g_list_store_append(self->input_devices_model, ui::holders::create(info));
  }));

  self->connections.push_back(pm->source_removed.connect([=](const NodeInfo info) {
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

  g_settings_bind(self->settings, "sidechain-preamp", gtk_spin_button_get_adjustment(self->sc_preamp), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "lookahead", gtk_spin_button_get_adjustment(self->lookahead), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "attack", gtk_spin_button_get_adjustment(self->attack), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "release", gtk_spin_button_get_adjustment(self->release), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "threshold", gtk_spin_button_get_adjustment(self->threshold), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "stereo-link", gtk_spin_button_get_adjustment(self->stereo_link), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "alr-attack", gtk_spin_button_get_adjustment(self->alr_attack), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "alr-release", gtk_spin_button_get_adjustment(self->alr_release), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "alr-knee", gtk_spin_button_get_adjustment(self->alr_knee), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "gain-boost", self->gain_boost, "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "external-sidechain", self->external_sidechain, "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "alr", self->alr, "active", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind_with_mapping(
      self->settings, "mode", self->mode, "active", G_SETTINGS_BIND_DEFAULT,
      +[](GValue* value, GVariant* variant, gpointer user_data) {
        const auto* v = g_variant_get_string(variant, nullptr);

        if (g_strcmp0(v, "Herm Thin") == 0) {
          g_value_set_int(value, 0);
        } else if (g_strcmp0(v, "Herm Wide") == 0) {
          g_value_set_int(value, 1);
        } else if (g_strcmp0(v, "Herm Tail") == 0) {
          g_value_set_int(value, 2);
        } else if (g_strcmp0(v, "Herm Duck") == 0) {
          g_value_set_int(value, 3);
        } else if (g_strcmp0(v, "Exp Thin") == 0) {
          g_value_set_int(value, 4);
        } else if (g_strcmp0(v, "Exp Wide") == 0) {
          g_value_set_int(value, 5);
        } else if (g_strcmp0(v, "Exp Tail") == 0) {
          g_value_set_int(value, 6);
        } else if (g_strcmp0(v, "Exp Duck") == 0) {
          g_value_set_int(value, 7);
        } else if (g_strcmp0(v, "Line Thin") == 0) {
          g_value_set_int(value, 8);
        } else if (g_strcmp0(v, "Line Wide") == 0) {
          g_value_set_int(value, 9);
        } else if (g_strcmp0(v, "Line Tail") == 0) {
          g_value_set_int(value, 10);
        } else if (g_strcmp0(v, "Line Duck") == 0) {
          g_value_set_int(value, 11);
        }

        return 1;
      },
      +[](const GValue* value, const GVariantType* expected_type, gpointer user_data) {
        switch (g_value_get_int(value)) {
          case 0:
            return g_variant_new_string("Herm Thin");
          case 1:
            return g_variant_new_string("Herm Wide");
          case 2:
            return g_variant_new_string("Herm Tail");
          case 3:
            return g_variant_new_string("Herm Duck");
          case 4:
            return g_variant_new_string("Exp Thin");
          case 5:
            return g_variant_new_string("Exp Wide");
          case 6:
            return g_variant_new_string("Exp Tail");
          case 7:
            return g_variant_new_string("Exp Duck");
          case 8:
            return g_variant_new_string("Line Thin");
          case 9:
            return g_variant_new_string("Line Wide");
          case 10:
            return g_variant_new_string("Line Tail");
          case 11:
            return g_variant_new_string("Line Duck");
          default:
            return g_variant_new_string("Herm Thin");
        }
      },
      nullptr, nullptr);

  g_settings_bind_with_mapping(
      self->settings, "oversampling", self->oversampling, "active", G_SETTINGS_BIND_DEFAULT,
      +[](GValue* value, GVariant* variant, gpointer user_data) {
        const auto* v = g_variant_get_string(variant, nullptr);

        if (g_strcmp0(v, "None") == 0) {
          g_value_set_int(value, 0);
        } else if (g_strcmp0(v, "Half x2(2L)") == 0) {
          g_value_set_int(value, 1);
        } else if (g_strcmp0(v, "Half x2(3L)") == 0) {
          g_value_set_int(value, 2);
        } else if (g_strcmp0(v, "Half x3(2L)") == 0) {
          g_value_set_int(value, 3);
        } else if (g_strcmp0(v, "Half x3(3L)") == 0) {
          g_value_set_int(value, 4);
        } else if (g_strcmp0(v, "Half x4(2L)") == 0) {
          g_value_set_int(value, 5);
        } else if (g_strcmp0(v, "Half x4(3L)") == 0) {
          g_value_set_int(value, 6);
        } else if (g_strcmp0(v, "Half x6(2L)") == 0) {
          g_value_set_int(value, 7);
        } else if (g_strcmp0(v, "Half x6(3L)") == 0) {
          g_value_set_int(value, 8);
        } else if (g_strcmp0(v, "Half x8(2L)") == 0) {
          g_value_set_int(value, 9);
        } else if (g_strcmp0(v, "Half x8(3L)") == 0) {
          g_value_set_int(value, 10);
        } else if (g_strcmp0(v, "Full x2(2L)") == 0) {
          g_value_set_int(value, 11);
        } else if (g_strcmp0(v, "Full x2(3L)") == 0) {
          g_value_set_int(value, 12);
        } else if (g_strcmp0(v, "Full x3(2L)") == 0) {
          g_value_set_int(value, 13);
        } else if (g_strcmp0(v, "Full x3(3L)") == 0) {
          g_value_set_int(value, 14);
        } else if (g_strcmp0(v, "Full x4(2L)") == 0) {
          g_value_set_int(value, 15);
        } else if (g_strcmp0(v, "Full x4(3L)") == 0) {
          g_value_set_int(value, 16);
        } else if (g_strcmp0(v, "Full x6(2L)") == 0) {
          g_value_set_int(value, 17);
        } else if (g_strcmp0(v, "Full x6(3L)") == 0) {
          g_value_set_int(value, 18);
        } else if (g_strcmp0(v, "Full x8(2L)") == 0) {
          g_value_set_int(value, 19);
        } else if (g_strcmp0(v, "Full x8(3L)") == 0) {
          g_value_set_int(value, 20);
        }

        return 1;
      },
      +[](const GValue* value, const GVariantType* expected_type, gpointer user_data) {
        switch (g_value_get_int(value)) {
          case 0:
            return g_variant_new_string("None");
          case 1:
            return g_variant_new_string("Half x2(2L)");
          case 2:
            return g_variant_new_string("Half x2(3L)");
          case 3:
            return g_variant_new_string("Half x3(2L)");
          case 4:
            return g_variant_new_string("Half x3(3L)");
          case 5:
            return g_variant_new_string("Half x4(2L)");
          case 6:
            return g_variant_new_string("Half x4(3L)");
          case 7:
            return g_variant_new_string("Half x6(2L)");
          case 8:
            return g_variant_new_string("Half x6(3L)");
          case 9:
            return g_variant_new_string("Half x8(2L)");
          case 10:
            return g_variant_new_string("Half x8(3L)");
          case 11:
            return g_variant_new_string("Full x2(2L)");
          case 12:
            return g_variant_new_string("Full x2(3L)");
          case 13:
            return g_variant_new_string("Full x3(2L)");
          case 14:
            return g_variant_new_string("Full x3(3L)");
          case 15:
            return g_variant_new_string("Full x4(2L)");
          case 16:
            return g_variant_new_string("Full x4(3L)");
          case 17:
            return g_variant_new_string("Full x6(2L)");
          case 18:
            return g_variant_new_string("Full x6(3L)");
          case 19:
            return g_variant_new_string("Full x8(2L)");
          case 20:
            return g_variant_new_string("Full x8(3L)");
          default:
            return g_variant_new_string("None");
        }
      },
      nullptr, nullptr);

  g_settings_bind_with_mapping(
      self->settings, "dithering", self->dither, "active", G_SETTINGS_BIND_DEFAULT,
      +[](GValue* value, GVariant* variant, gpointer user_data) {
        const auto* v = g_variant_get_string(variant, nullptr);

        if (g_strcmp0(v, "None") == 0) {
          g_value_set_int(value, 0);
        } else if (g_strcmp0(v, "7bit") == 0) {
          g_value_set_int(value, 1);
        } else if (g_strcmp0(v, "8bit") == 0) {
          g_value_set_int(value, 2);
        } else if (g_strcmp0(v, "11bit") == 0) {
          g_value_set_int(value, 3);
        } else if (g_strcmp0(v, "12bit") == 0) {
          g_value_set_int(value, 4);
        } else if (g_strcmp0(v, "15bit") == 0) {
          g_value_set_int(value, 5);
        } else if (g_strcmp0(v, "16bit") == 0) {
          g_value_set_int(value, 6);
        } else if (g_strcmp0(v, "23bit") == 0) {
          g_value_set_int(value, 7);
        } else if (g_strcmp0(v, "24bit") == 0) {
          g_value_set_int(value, 8);
        }

        return 1;
      },
      +[](const GValue* value, const GVariantType* expected_type, gpointer user_data) {
        switch (g_value_get_int(value)) {
          case 0:
            return g_variant_new_string("None");
          case 1:
            return g_variant_new_string("7bit");
          case 2:
            return g_variant_new_string("8bit");
          case 3:
            return g_variant_new_string("11bit");
          case 4:
            return g_variant_new_string("12bit");
          case 5:
            return g_variant_new_string("15bit");
          case 6:
            return g_variant_new_string("16bit");
          case 7:
            return g_variant_new_string("23bit");
          case 8:
            return g_variant_new_string("24bit");
          default:
            return g_variant_new_string("None");
        }
      },
      nullptr, nullptr);
}

void dispose(GObject* object) {
  auto* self = EE_LIMITER_BOX(object);

  self->limiter->post_messages = false;
  self->limiter->bypass = false;

  for (auto& c : self->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  self->connections.clear();
  self->gconnections.clear();

  g_object_unref(self->settings);

  util::debug(log_tag + "disposed"s);

  G_OBJECT_CLASS(limiter_box_parent_class)->dispose(object);
}

void limiter_box_class_init(LimiterBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/limiter.ui");

  gtk_widget_class_bind_template_child(widget_class, LimiterBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, LimiterBox, bypass);

  gtk_widget_class_bind_template_child(widget_class, LimiterBox, mode);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, oversampling);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, dither);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, sc_preamp);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, lookahead);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, attack);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, release);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, threshold);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, stereo_link);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, alr_attack);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, alr_release);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, alr_knee);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, alr);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, external_sidechain);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, gain_boost);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, gain_left);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, gain_right);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, sidechain_left);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, sidechain_right);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, dropdown_input_devices);

  gtk_widget_class_bind_template_callback(widget_class, on_bypass);
  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void limiter_box_init(LimiterBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->input_devices_model = g_list_store_new(ui::holders::node_info_holder_get_type());

  prepare_spinbutton<"dB">(self->sc_preamp);
  prepare_spinbutton<"dB">(self->threshold);
  prepare_spinbutton<"dB">(self->alr_knee);
  prepare_spinbutton<"ms">(self->lookahead);
  prepare_spinbutton<"ms">(self->attack);
  prepare_spinbutton<"ms">(self->release);
  prepare_spinbutton<"ms">(self->alr_attack);
  prepare_spinbutton<"ms">(self->alr_release);
  prepare_spinbutton<"%">(self->stereo_link);
}

auto create() -> LimiterBox* {
  return static_cast<LimiterBox*>(g_object_new(EE_TYPE_LIMITER_BOX, nullptr));
}

}  // namespace ui::limiter_box
