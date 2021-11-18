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

#include "effects_box.hpp"

namespace ui::effects_box {

using namespace std::string_literals;

auto constexpr log_tag = "effects_box: ";

struct _EffectsBox {
  GtkBox parent_instance{};

  AdwViewStack* stack;

  AdwViewStackPage *apps_box_page, *plugins_box_page;

  GtkLabel *device_state, *latency_status, *label_global_output_level_left, *label_global_output_level_right;

  GtkToggleButton* toggle_listen_mic;

  GtkMenuButton* menubutton_blocklist;

  ui::chart::Chart* spectrum_chart;

  ui::apps_box::AppsBox* appsBox;

  ui::plugins_box::PluginsBox* pluginsBox;

  GSettings *settings, *settings_spectrum, *app_settings;

  app::Application* application;

  EffectsBase* effects_base;

  PipelineType pipeline_type;

  bool schedule_signal_idle;

  uint spectrum_rate, spectrum_n_bands;

  float global_output_level_left, global_output_level_right, pipeline_latency_ms;

  std::vector<float> spectrum_mag, spectrum_freqs, spectrum_x_axis;

  std::vector<uint> spectrum_bin_count;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections_spectrum;
};

G_DEFINE_TYPE(EffectsBox, effects_box, GTK_TYPE_BOX)

void init_spectrum_frequency_axis(EffectsBox* self) {
  self->spectrum_freqs.resize(self->spectrum_n_bands);

  for (uint n = 0U; n < self->spectrum_n_bands; n++) {
    self->spectrum_freqs[n] = 0.5F * static_cast<float>(self->spectrum_rate) * static_cast<float>(n) /
                              static_cast<float>(self->spectrum_n_bands);
  }

  if (!self->spectrum_freqs.empty()) {
    const auto min_freq = static_cast<float>(g_settings_get_int(self->settings_spectrum, "minimum-frequency"));
    const auto max_freq = static_cast<float>(g_settings_get_int(self->settings_spectrum, "maximum-frequency"));

    if (min_freq > (max_freq - 100.0F)) {
      return;
    }

    self->spectrum_x_axis = util::logspace(min_freq, max_freq, g_settings_get_int(self->settings_spectrum, "n-points"));

    const auto x_axis_size = self->spectrum_x_axis.size();

    self->spectrum_mag.resize(x_axis_size);

    self->spectrum_bin_count.resize(x_axis_size);
  }
}

void setup_spectrum(EffectsBox* self) {
  self->spectrum_rate = 0U;
  self->spectrum_n_bands = 0U;

  ui::chart::set_color(self->spectrum_chart, util::gsettings_get_color(self->settings_spectrum, "color"));

  ui::chart::set_axis_labels_color(self->spectrum_chart,
                                   util::gsettings_get_color(self->settings_spectrum, "color-axis-labels"));

  ui::chart::set_fill_bars(self->spectrum_chart, g_settings_get_boolean(self->settings_spectrum, "fill") != 0);

  ui::chart::set_draw_bar_border(self->spectrum_chart,
                                 g_settings_get_boolean(self->settings_spectrum, "show-bar-border") != 0);

  ui::chart::set_line_width(self->spectrum_chart, g_settings_get_double(self->settings_spectrum, "line-width"));

  ui::chart::set_x_unit(self->spectrum_chart, "Hz");
  ui::chart::set_y_unit(self->spectrum_chart, "dB");

  ui::chart::set_n_x_decimals(self->spectrum_chart, 0);
  ui::chart::set_n_y_decimals(self->spectrum_chart, 1);

  ui::chart::set_margin(self->spectrum_chart, 0);

  gtk_widget_set_size_request(GTK_WIDGET(self->spectrum_chart), -1,
                              g_settings_get_int(self->settings_spectrum, "height"));

  if (g_strcmp0(g_settings_get_string(self->settings_spectrum, "type"), "Bars") == 0) {
    ui::chart::set_plot_type(self->spectrum_chart, chart::ChartType::bar);
  } else if (g_strcmp0(g_settings_get_string(self->settings_spectrum, "type"), "Lines") == 0) {
    ui::chart::set_plot_type(self->spectrum_chart, chart::ChartType::line);
  }

  g_settings_bind(self->settings_spectrum, "show", self->spectrum_chart, "visible", G_SETTINGS_BIND_GET);

  self->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::color", G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) {
        ui::chart::set_color(self->spectrum_chart, util::gsettings_get_color(self->settings_spectrum, key));
      }),
      self));

  self->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::color-axis-labels",
      G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) {
        ui::chart::set_axis_labels_color(self->spectrum_chart, util::gsettings_get_color(self->settings_spectrum, key));
      }),
      self));

  self->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::fill", G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) {
        ui::chart::set_fill_bars(self->spectrum_chart, g_settings_get_boolean(self->settings_spectrum, "fill") != 0);
      }),
      self));

  self->gconnections_spectrum.push_back(
      g_signal_connect(self->settings_spectrum, "changed::show-bar-border",
                       G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) {
                         ui::chart::set_draw_bar_border(self->spectrum_chart,
                                                        g_settings_get_boolean(self->settings_spectrum, "fill") != 0);
                       }),
                       self));

  self->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::line-width", G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) {
        ui::chart::set_line_width(self->spectrum_chart, g_settings_get_double(self->settings_spectrum, "line-width"));
      }),
      self));

  self->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::height", G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) {
        gtk_widget_set_size_request(GTK_WIDGET(self->spectrum_chart), -1,
                                    g_settings_get_int(self->settings_spectrum, "height"));
      }),
      self));

  self->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::type", G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) {
        if (g_strcmp0(g_settings_get_string(self->settings_spectrum, key), "Bars") == 0) {
          ui::chart::set_plot_type(self->spectrum_chart, chart::ChartType::bar);
        } else if (g_strcmp0(g_settings_get_string(self->settings_spectrum, key), "Lines") == 0) {
          ui::chart::set_plot_type(self->spectrum_chart, chart::ChartType::line);
        }
      }),
      self));

  self->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::n-points",
      G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) { init_spectrum_frequency_axis(self); }), self));

  self->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::minimum-frequency",
      G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) { init_spectrum_frequency_axis(self); }), self));

  self->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::maximum-frequency",
      G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) { init_spectrum_frequency_axis(self); }), self));
}
void stack_visible_child_changed(EffectsBox* self, GParamSpec* pspec, GtkWidget* stack) {
  const auto* name = adw_view_stack_get_visible_child_name(ADW_VIEW_STACK(stack));

  gtk_widget_set_visible(GTK_WIDGET(self->menubutton_blocklist), static_cast<gboolean>(g_strcmp0(name, "apps") == 0));

  if (self->pipeline_type == PipelineType::input) {
    gtk_widget_set_visible(GTK_WIDGET(self->toggle_listen_mic), static_cast<gboolean>(g_strcmp0(name, "plugins") == 0));
  }
}

void setup(EffectsBox* self, app::Application* application, PipelineType pipeline_type) {
  self->application = application;
  self->pipeline_type = pipeline_type;

  switch (pipeline_type) {
    case PipelineType::input: {
      self->settings = g_settings_new("com.github.wwmm.easyeffects.streamoutputs");

      self->effects_base = static_cast<EffectsBase*>(self->application->sie.get());

      self->apps_box_page = adw_view_stack_add_titled(self->stack, GTK_WIDGET(self->appsBox), "apps", _("Recorders"));

      adw_view_stack_page_set_icon_name(self->apps_box_page, "media-record-symbolic");

      auto set_device_state_label = [=]() {
        auto source_rate = static_cast<float>(application->pm->ee_source_node.rate) * 0.001F;

        gtk_label_set_text(self->device_state, fmt::format("{0:.1f} kHz", source_rate).c_str());
      };

      set_device_state_label();

      self->connections.push_back(application->pm->source_changed.connect([=](const auto nd_info) {
        if (nd_info.id == application->pm->ee_source_node.id) {
          set_device_state_label();
        }
      }));

      break;
    }
    case PipelineType::output: {
      self->settings = g_settings_new("com.github.wwmm.easyeffects.streaminputs");

      self->effects_base = static_cast<EffectsBase*>(self->application->soe.get());

      self->apps_box_page = adw_view_stack_add_titled(self->stack, GTK_WIDGET(self->appsBox), "apps", _("Players"));

      adw_view_stack_page_set_icon_name(self->apps_box_page, "multimedia-player-symbolic");

      auto set_device_state_label = [=]() {
        auto sink_rate = static_cast<float>(application->pm->ee_sink_node.rate) * 0.001F;

        gtk_label_set_text(self->device_state, fmt::format("{0:.1f} kHz", sink_rate).c_str());
      };

      set_device_state_label();

      self->connections.push_back(application->pm->sink_changed.connect([=](const auto nd_info) {
        if (nd_info.id == application->pm->ee_sink_node.id) {
          set_device_state_label();
        }
      }));

      break;
    }
  }

  self->plugins_box_page =
      adw_view_stack_add_titled(self->stack, GTK_WIDGET(self->pluginsBox), "plugins", _("Plugins"));

  // "ee-plugins-symbolic" is just Adwaita's "application-x-addon-symbolic.svg" renamed
  adw_view_stack_page_set_icon_name(self->plugins_box_page, "ee-plugins-symbolic");

  // setting up the boxes we added t othe stack

  ui::apps_box::setup(self->appsBox, application, pipeline_type);
  ui::plugins_box::setup(self->pluginsBox, application, pipeline_type);

  // output level

  self->connections.push_back(
      self->effects_base->output_level->output_level.connect([=](const float& left, const float& right) {
        self->global_output_level_left = left;
        self->global_output_level_right = right;

        if (!self->schedule_signal_idle) {
          return;
        }

        g_idle_add((GSourceFunc) +
                       [](EffectsBox* self) {
                         ui::chart::set_data(self->spectrum_chart, self->spectrum_x_axis, self->spectrum_mag);

                         gtk_label_set_text(self->label_global_output_level_left,
                                            fmt::format("{0:.0f}", self->global_output_level_left).c_str());

                         gtk_label_set_text(self->label_global_output_level_right,
                                            fmt::format("{0:.0f}", self->global_output_level_right).c_str());

                         return G_SOURCE_REMOVE;
                       },
                   self);
      }));

  // spectrum array

  self->connections.push_back(
      self->effects_base->spectrum->power.connect([=](uint rate, uint n_bands, std::vector<float> magnitudes) {
        if (!ui::chart::get_is_visible(self->spectrum_chart)) {
          return;
        }

        if (!self->schedule_signal_idle) {
          return;
        }

        if (self->spectrum_rate != rate || self->spectrum_n_bands != n_bands) {
          self->spectrum_rate = rate;
          self->spectrum_n_bands = n_bands;

          init_spectrum_frequency_axis(self);
        }

        std::ranges::fill(self->spectrum_mag, 0.0F);
        std::ranges::fill(self->spectrum_bin_count, 0U);

        // reducing the amount of data so we can plot them

        for (size_t j = 0U; j < self->spectrum_freqs.size(); j++) {
          for (size_t n = 0U; n < self->spectrum_x_axis.size(); n++) {
            if (n > 0U) {
              if (self->spectrum_freqs[j] <= self->spectrum_x_axis[n] &&
                  self->spectrum_freqs[j] > self->spectrum_x_axis[n - 1U]) {
                self->spectrum_mag[n] += magnitudes[j];

                self->spectrum_bin_count[n]++;
              }
            } else {
              if (self->spectrum_freqs[j] <= self->spectrum_x_axis[n]) {
                self->spectrum_mag[n] += magnitudes[j];

                self->spectrum_bin_count[n]++;
              }
            }
          }
        }

        for (size_t n = 0U; n < self->spectrum_bin_count.size(); n++) {
          if (self->spectrum_bin_count[n] == 0U && n > 0U) {
            self->spectrum_mag[n] = self->spectrum_mag[n - 1U];
          }
        }

        std::ranges::for_each(self->spectrum_mag, [](auto& v) {
          v = 10.0F * std::log10(v);

          if (!std::isinf(v)) {
            v = (v > util::minimum_db_level) ? v : util::minimum_db_level;
          } else {
            v = util::minimum_db_level;
          }
        });

        g_idle_add((GSourceFunc) +
                       [](EffectsBox* self) {
                         ui::chart::set_data(self->spectrum_chart, self->spectrum_x_axis, self->spectrum_mag);

                         return G_SOURCE_REMOVE;
                       },
                   self);
      }));

  self->effects_base->output_level->post_messages = true;
  self->effects_base->spectrum->post_messages = true;

  // pipeline latency

  gtk_label_set_text(self->latency_status,
                     fmt::format("     {0:.1f} ms", self->effects_base->get_pipeline_latency()).c_str());

  self->connections.push_back(self->effects_base->pipeline_latency.connect([=](const float& v) {
    self->pipeline_latency_ms = v;

    if (!self->schedule_signal_idle) {
      return;
    }

    g_idle_add((GSourceFunc) +
                   [](EffectsBox* self) {
                     gtk_label_set_text(self->latency_status,
                                        fmt::format("     {0:.1f} ms", self->pipeline_latency_ms).c_str());

                     return G_SOURCE_REMOVE;
                   },
               self);
  }));
}

void realize(GtkWidget* widget) {
  auto* self = EE_EFFECTS_BOX(widget);

  self->schedule_signal_idle = true;

  GTK_WIDGET_CLASS(effects_box_parent_class)->realize(widget);
}

void unroot(GtkWidget* widget) {
  auto* self = EE_EFFECTS_BOX(widget);

  self->schedule_signal_idle = false;

  GTK_WIDGET_CLASS(effects_box_parent_class)->unroot(widget);
}

void dispose(GObject* object) {
  auto* self = EE_EFFECTS_BOX(object);

  self->effects_base->spectrum->post_messages = false;

  for (auto& c : self->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->gconnections_spectrum) {
    g_signal_handler_disconnect(self->settings_spectrum, handler_id);
  }

  g_object_unref(self->settings);
  g_object_unref(self->app_settings);
  g_object_unref(self->settings_spectrum);

  util::debug(log_tag + "disposed"s);

  G_OBJECT_CLASS(effects_box_parent_class)->dispose(object);
}

void effects_box_class_init(EffectsBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  widget_class->realize = realize;
  widget_class->unroot = unroot;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/effects_box.ui");

  gtk_widget_class_bind_template_child(widget_class, EffectsBox, stack);
  gtk_widget_class_bind_template_child(widget_class, EffectsBox, device_state);
  gtk_widget_class_bind_template_child(widget_class, EffectsBox, latency_status);
  gtk_widget_class_bind_template_child(widget_class, EffectsBox, label_global_output_level_left);
  gtk_widget_class_bind_template_child(widget_class, EffectsBox, label_global_output_level_right);
  gtk_widget_class_bind_template_child(widget_class, EffectsBox, toggle_listen_mic);
  gtk_widget_class_bind_template_child(widget_class, EffectsBox, menubutton_blocklist);

  gtk_widget_class_bind_template_callback(widget_class, stack_visible_child_changed);
}

void effects_box_init(EffectsBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->schedule_signal_idle = false;

  self->app_settings = g_settings_new("com.github.wwmm.easyeffects");
  self->settings_spectrum = g_settings_new("com.github.wwmm.easyeffects.spectrum");

  self->spectrum_chart = ui::chart::create();

  self->appsBox = ui::apps_box::create();
  self->pluginsBox = ui::plugins_box::create();

  setup_spectrum(self);

  gtk_box_insert_child_after(GTK_BOX(self), GTK_WIDGET(self->spectrum_chart), nullptr);

  g_signal_connect(GTK_WIDGET(self->spectrum_chart), "hide", G_CALLBACK(+[](GtkWidget* widget, EffectsBox* self) {
                     self->effects_base->spectrum->post_messages = false;
                   }),
                   self);
}

auto create() -> EffectsBox* {
  return static_cast<EffectsBox*>(g_object_new(EE_TYPE_EFFECTS_BOX, nullptr));
}

}  // namespace ui::effects_box