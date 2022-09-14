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

#include "bass_enhancer_ui.hpp"

namespace ui::bass_enhancer_box {

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  uint serial = 0;

  std::shared_ptr<BassEnhancer> bass_enhancer;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _BassEnhancerBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkLevelBar* harmonics_levelbar;

  GtkLabel* harmonics_levelbar_label;

  GtkSpinButton *floor, *amount, *harmonics, *scope;

  GtkScale* blend;

  GtkToggleButton *floor_active, *listen;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(BassEnhancerBox, bass_enhancer_box, GTK_TYPE_BOX)

void on_reset(BassEnhancerBox* self, GtkButton* btn) {
  util::reset_all_keys_except(self->settings);
}

void setup(BassEnhancerBox* self, std::shared_ptr<BassEnhancer> bass_enhancer, const std::string& schema_path) {
  auto serial = get_new_filter_serial();

  self->data->serial = serial;

  g_object_set_data(G_OBJECT(self), "serial", GUINT_TO_POINTER(serial));

  set_ignore_filter_idle_add(serial, false);

  self->data->bass_enhancer = bass_enhancer;

  self->settings = g_settings_new_with_path(tags::schema::bass_enhancer::id, schema_path.c_str());

  bass_enhancer->set_post_messages(true);

  self->data->connections.push_back(bass_enhancer->input_level.connect([=](const float left, const float right) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                   self->input_level_right_label, left, right);
    });
  }));

  self->data->connections.push_back(bass_enhancer->output_level.connect([=](const float left, const float right) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                   self->output_level_right_label, left, right);
    });
  }));

  self->data->connections.push_back(bass_enhancer->harmonics.connect([=](const double value) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      if (!GTK_IS_LEVEL_BAR(self->harmonics_levelbar) || !GTK_IS_LABEL(self->harmonics_levelbar_label)) {
        return;
      }

      gtk_level_bar_set_value(self->harmonics_levelbar, value);
      gtk_label_set_text(self->harmonics_levelbar_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
    });
  }));

  gsettings_bind_widgets<"input-gain", "output-gain">(self->settings, self->input_gain, self->output_gain);

  gsettings_bind_widgets<"amount", "harmonics", "scope", "floor", "blend", "listen", "floor-active">(
      self->settings, self->amount, self->harmonics, self->scope, self->floor, self->blend, self->listen,
      self->floor_active);
}

void dispose(GObject* object) {
  auto* self = EE_BASS_ENHANCER_BOX(object);

  self->data->bass_enhancer->set_post_messages(false);

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

  G_OBJECT_CLASS(bass_enhancer_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_BASS_ENHANCER_BOX(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(bass_enhancer_box_parent_class)->finalize(object);
}

void bass_enhancer_box_class_init(BassEnhancerBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::bass_enhancer_ui);

  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, harmonics_levelbar_label);
  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, harmonics_levelbar);
  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, floor);
  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, amount);
  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, harmonics);
  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, scope);
  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, blend);
  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, floor_active);
  gtk_widget_class_bind_template_child(widget_class, BassEnhancerBox, listen);

  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void bass_enhancer_box_init(BassEnhancerBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  prepare_spinbuttons<"dB">(self->amount);
  prepare_spinbuttons<"Hz">(self->scope, self->floor);
  prepare_spinbuttons<"">(self->harmonics);

  prepare_scales<"">(self->blend);

  prepare_scales<"dB">(self->input_gain, self->output_gain);
}

auto create() -> BassEnhancerBox* {
  return static_cast<BassEnhancerBox*>(g_object_new(EE_TYPE_BASS_ENHANCER_BOX, nullptr));
}

}  // namespace ui::bass_enhancer_box
