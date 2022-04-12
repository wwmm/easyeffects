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

struct Data {
 public:
  ~Data() { util::debug(log_tag + "data struct destroyed"s); }

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

  std::locale user_locale = std::locale(setlocale(LC_ALL, nullptr));
};

struct _EffectsBox {
  GtkBox parent_instance;

  AdwViewStack* stack;

  AdwViewStackPage *apps_box_page, *plugins_box_page;

  GtkLabel *device_state, *latency_status, *label_global_output_level_left, *label_global_output_level_right;

  GtkToggleButton* toggle_listen_mic;

  GtkMenuButton* menubutton_blocklist;

  GtkImage* saturation_icon;

  GtkIconTheme* icon_theme;

  ui::chart::Chart* spectrum_chart;

  ui::apps_box::AppsBox* appsBox;

  ui::plugins_box::PluginsBox* pluginsBox;

  ui::blocklist_menu::BlocklistMenu* blocklist_menu;

  GSettings *settings_spectrum, *app_settings;

  Data* data;
};

G_DEFINE_TYPE(EffectsBox, effects_box, GTK_TYPE_BOX)

void init_spectrum_frequency_axis(EffectsBox* self) {
  self->data->spectrum_freqs.resize(self->data->spectrum_n_bands);

  for (uint n = 0U; n < self->data->spectrum_n_bands; n++) {
    self->data->spectrum_freqs[n] = 0.5F * static_cast<float>(self->data->spectrum_rate) * static_cast<float>(n) /
                                    static_cast<float>(self->data->spectrum_n_bands);
  }

  if (!self->data->spectrum_freqs.empty()) {
    const auto min_freq = static_cast<float>(g_settings_get_int(self->settings_spectrum, "minimum-frequency"));
    const auto max_freq = static_cast<float>(g_settings_get_int(self->settings_spectrum, "maximum-frequency"));

    if (min_freq > (max_freq - 100.0F)) {
      return;
    }

    self->data->spectrum_x_axis =
        util::logspace(min_freq, max_freq, g_settings_get_int(self->settings_spectrum, "n-points"));

    const auto x_axis_size = self->data->spectrum_x_axis.size();

    self->data->spectrum_mag.resize(x_axis_size);

    self->data->spectrum_bin_count.resize(x_axis_size);

    ui::chart::set_x_data(self->spectrum_chart, self->data->spectrum_x_axis);
  }
}

void setup_spectrum(EffectsBox* self) {
  self->data->spectrum_rate = 0U;
  self->data->spectrum_n_bands = 0U;

  ui::chart::set_color(self->spectrum_chart, util::gsettings_get_color(self->settings_spectrum, "color"));

  ui::chart::set_axis_labels_color(self->spectrum_chart,
                                   util::gsettings_get_color(self->settings_spectrum, "color-axis-labels"));

  ui::chart::set_fill_bars(self->spectrum_chart, g_settings_get_boolean(self->settings_spectrum, "fill") != 0);

  ui::chart::set_rounded_corners(self->spectrum_chart,
                                 g_settings_get_boolean(self->settings_spectrum, "rounded-corners") != 0);

  ui::chart::set_draw_bar_border(self->spectrum_chart,
                                 g_settings_get_boolean(self->settings_spectrum, "show-bar-border") != 0);

  ui::chart::set_line_width(self->spectrum_chart, g_settings_get_double(self->settings_spectrum, "line-width"));

  ui::chart::set_chart_scale(self->spectrum_chart, ui::chart::ChartScale::logarithmic);

  ui::chart::set_x_unit(self->spectrum_chart, "Hz");
  ui::chart::set_y_unit(self->spectrum_chart, "dB");

  ui::chart::set_n_x_decimals(self->spectrum_chart, 0);
  ui::chart::set_n_y_decimals(self->spectrum_chart, 1);

  ui::chart::set_margin(self->spectrum_chart, 0);

  gtk_widget_set_size_request(GTK_WIDGET(self->spectrum_chart), -1,
                              g_settings_get_int(self->settings_spectrum, "height"));

  auto chart_type = util::gsettings_get_string(self->settings_spectrum, "type");

  if (chart_type == "Bars") {
    ui::chart::set_chart_type(self->spectrum_chart, chart::ChartType::bar);
  } else if (chart_type == "Lines") {
    ui::chart::set_chart_type(self->spectrum_chart, chart::ChartType::line);
  } else if (chart_type == "Dots") {
    ui::chart::set_chart_type(self->spectrum_chart, chart::ChartType::dots);
  }

  g_settings_bind(self->settings_spectrum, "show", self->spectrum_chart, "visible", G_SETTINGS_BIND_GET);

  self->data->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::color", G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) {
        ui::chart::set_color(self->spectrum_chart, util::gsettings_get_color(self->settings_spectrum, key));
      }),
      self));

  self->data->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::color-axis-labels",
      G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) {
        ui::chart::set_axis_labels_color(self->spectrum_chart, util::gsettings_get_color(self->settings_spectrum, key));
      }),
      self));

  self->data->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::fill", G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) {
        ui::chart::set_fill_bars(self->spectrum_chart, g_settings_get_boolean(self->settings_spectrum, key) != 0);
      }),
      self));

  self->data->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::rounded-corners",
      G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) {
        ui::chart::set_rounded_corners(self->spectrum_chart, g_settings_get_boolean(self->settings_spectrum, key) != 0);
      }),
      self));

  self->data->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::show-bar-border",
      G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) {
        ui::chart::set_draw_bar_border(self->spectrum_chart, g_settings_get_boolean(self->settings_spectrum, key) != 0);
      }),
      self));

  self->data->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::line-width", G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) {
        ui::chart::set_line_width(self->spectrum_chart, g_settings_get_double(self->settings_spectrum, key));
      }),
      self));

  self->data->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::height", G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) {
        gtk_widget_set_size_request(GTK_WIDGET(self->spectrum_chart), -1,
                                    g_settings_get_int(self->settings_spectrum, key));
      }),
      self));

  self->data->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::type", G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) {
        auto chart_type = util::gsettings_get_string(self->settings_spectrum, key);

        if (chart_type == "Bars") {
          ui::chart::set_chart_type(self->spectrum_chart, chart::ChartType::bar);
        } else if (chart_type == "Lines") {
          ui::chart::set_chart_type(self->spectrum_chart, chart::ChartType::line);
        } else if (chart_type == "Dots") {
          ui::chart::set_chart_type(self->spectrum_chart, chart::ChartType::dots);
        }
      }),
      self));

  self->data->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::n-points",
      G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) { init_spectrum_frequency_axis(self); }), self));

  self->data->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::minimum-frequency",
      G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) { init_spectrum_frequency_axis(self); }), self));

  self->data->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::maximum-frequency",
      G_CALLBACK(+[](GSettings* settings, char* key, EffectsBox* self) { init_spectrum_frequency_axis(self); }), self));
}
void stack_visible_child_changed(EffectsBox* self, GParamSpec* pspec, GtkWidget* stack) {
  const auto* name = adw_view_stack_get_visible_child_name(ADW_VIEW_STACK(stack));

  gtk_widget_set_visible(GTK_WIDGET(self->menubutton_blocklist), static_cast<gboolean>(g_strcmp0(name, "apps") == 0));

  if (self->data->pipeline_type == PipelineType::input) {
    gtk_widget_set_visible(GTK_WIDGET(self->toggle_listen_mic), static_cast<gboolean>(g_strcmp0(name, "plugins") == 0));
  }
}

void on_listen_mic_toggled(EffectsBox* self, GtkToggleButton* button) {
  self->data->application->sie->set_listen_to_mic(gtk_toggle_button_get_active(button) != 0);
}

void setup(EffectsBox* self, app::Application* application, PipelineType pipeline_type, GtkIconTheme* icon_theme) {
  self->data->application = application;
  self->data->pipeline_type = pipeline_type;
  self->icon_theme = icon_theme;

  switch (pipeline_type) {
    case PipelineType::input: {
      self->data->effects_base = static_cast<EffectsBase*>(self->data->application->sie);

      self->apps_box_page = adw_view_stack_add_titled(self->stack, GTK_WIDGET(self->appsBox), "apps", _("Recorders"));

      adw_view_stack_page_set_icon_name(self->apps_box_page, "media-record-symbolic");

      auto set_device_state_label = [=]() {
        auto source_rate = static_cast<float>(application->pm->ee_source_node.rate) * 0.001F;

        gtk_label_set_text(self->device_state,
                           fmt::format(self->data->user_locale, "{0:.1Lf} kHz", source_rate).c_str());
      };

      set_device_state_label();

      self->data->connections.push_back(application->pm->source_changed.connect([=](const auto nd_info) {
        if (nd_info.id == application->pm->ee_source_node.id) {
          set_device_state_label();
        }
      }));

      break;
    }
    case PipelineType::output: {
      self->data->effects_base = static_cast<EffectsBase*>(self->data->application->soe);

      self->apps_box_page = adw_view_stack_add_titled(self->stack, GTK_WIDGET(self->appsBox), "apps", _("Players"));

      adw_view_stack_page_set_icon_name(self->apps_box_page, "multimedia-player-symbolic");

      auto set_device_state_label = [=]() {
        auto sink_rate = static_cast<float>(application->pm->ee_sink_node.rate) * 0.001F;

        gtk_label_set_text(self->device_state, fmt::format(self->data->user_locale, "{0:.1Lf} kHz", sink_rate).c_str());
      };

      set_device_state_label();

      self->data->connections.push_back(application->pm->sink_changed.connect([=](const auto nd_info) {
        if (nd_info.id == application->pm->ee_sink_node.id) {
          set_device_state_label();
        }
      }));

      break;
    }
  }

  self->plugins_box_page =
      adw_view_stack_add_titled(self->stack, GTK_WIDGET(self->pluginsBox), "plugins", _("Effects"));

  adw_view_stack_page_set_icon_name(self->plugins_box_page, "emblem-music-symbolic");

  // setting up the boxes we added t othe stack

  ui::apps_box::setup(self->appsBox, application, pipeline_type, icon_theme);
  ui::plugins_box::setup(self->pluginsBox, application, pipeline_type);
  ui::blocklist_menu::setup(self->blocklist_menu, application, pipeline_type);

  // output level

  self->data->connections.push_back(
      self->data->effects_base->output_level->output_level.connect([=](const float& left, const float& right) {
        self->data->global_output_level_left = left;
        self->data->global_output_level_right = right;

        if (!self->data->schedule_signal_idle) {
          return;
        }

        g_idle_add((GSourceFunc) +
                       [](EffectsBox* self) {
                         gtk_label_set_text(self->label_global_output_level_left,
                                            fmt::format("{0:.0f}", self->data->global_output_level_left).c_str());

                         gtk_label_set_text(self->label_global_output_level_right,
                                            fmt::format("{0:.0f}", self->data->global_output_level_right).c_str());

                         gtk_widget_set_opacity(
                             GTK_WIDGET(self->saturation_icon),
                             (self->data->global_output_level_left > 0.0 || self->data->global_output_level_right > 0.0)
                                 ? 1.0
                                 : 0.0);

                         return G_SOURCE_REMOVE;
                       },
                   self);
      }));

  // spectrum array

  self->data->connections.push_back(
      self->data->effects_base->spectrum->power.connect([=](uint rate, uint n_bands, std::vector<float> magnitudes) {
        if (!ui::chart::get_is_visible(self->spectrum_chart)) {
          return;
        }

        if (!self->data->schedule_signal_idle) {
          return;
        }

        if (self->data->spectrum_rate != rate || self->data->spectrum_n_bands != n_bands) {
          self->data->spectrum_rate = rate;
          self->data->spectrum_n_bands = n_bands;

          init_spectrum_frequency_axis(self);
        }

        std::ranges::fill(self->data->spectrum_mag, 0.0F);
        std::ranges::fill(self->data->spectrum_bin_count, 0U);

        // reducing the amount of data so we can plot them

        size_t last_j = 0;

        for (size_t n = 0U; n < self->data->spectrum_x_axis.size(); n++) {
          for (size_t j = last_j; j < self->data->spectrum_freqs.size(); j++) {
            if (self->data->spectrum_freqs[j] > self->data->spectrum_x_axis[n]) {
              last_j = j;

              break;
            } else {
              self->data->spectrum_mag[n] += magnitudes[j];

              self->data->spectrum_bin_count[n]++;
            }
          }
        }

        for (size_t n = 0U; n < self->data->spectrum_bin_count.size(); n++) {
          if (self->data->spectrum_bin_count[n] != 0U) {
            continue;
          }

          if (n > 0U && n < self->data->spectrum_bin_count.size() + 1) {
            self->data->spectrum_mag[n] = self->data->spectrum_mag[n - 1U];
          }
        }

        std::ranges::for_each(self->data->spectrum_mag, [](auto& v) {
          v = 10.0F * std::log10(v);

          if (!std::isinf(v)) {
            v = (v > util::minimum_db_level) ? v : util::minimum_db_level;
          } else {
            v = util::minimum_db_level;
          }
        });

        ui::chart::set_y_data(self->spectrum_chart, self->data->spectrum_mag);
      }));

  // As we are showing the window we want the filters to send notifications about level meters, etc

  PluginBase::post_messages = true;

  self->data->effects_base->spectrum->bypass = !g_settings_get_boolean(self->settings_spectrum, "show");

  // pipeline latency

  gtk_label_set_text(self->latency_status, fmt::format(self->data->user_locale, "     {0:.1Lf} ms",
                                                       self->data->effects_base->get_pipeline_latency())
                                               .c_str());

  self->data->connections.push_back(self->data->effects_base->pipeline_latency.connect([=](const float& v) {
    self->data->pipeline_latency_ms = v;

    if (!self->data->schedule_signal_idle) {
      return;
    }

    g_idle_add(
        (GSourceFunc) +
            [](EffectsBox* self) {
              gtk_label_set_text(
                  self->latency_status,
                  fmt::format(self->data->user_locale, "     {0:.1Lf} ms", self->data->pipeline_latency_ms).c_str());

              return G_SOURCE_REMOVE;
            },
        self);
  }));
}

void realize(GtkWidget* widget) {
  auto* self = EE_EFFECTS_BOX(widget);

  self->data->schedule_signal_idle = true;

  GTK_WIDGET_CLASS(effects_box_parent_class)->realize(widget);
}

void unroot(GtkWidget* widget) {
  auto* self = EE_EFFECTS_BOX(widget);

  self->data->schedule_signal_idle = false;

  GTK_WIDGET_CLASS(effects_box_parent_class)->unroot(widget);
}

void dispose(GObject* object) {
  auto* self = EE_EFFECTS_BOX(object);

  PluginBase::post_messages = false;

  self->data->effects_base->spectrum->bypass = true;

  for (auto& c : self->data->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->data->gconnections_spectrum) {
    g_signal_handler_disconnect(self->settings_spectrum, handler_id);
  }

  self->data->connections.clear();
  self->data->gconnections_spectrum.clear();

  g_object_unref(self->app_settings);
  g_object_unref(self->settings_spectrum);

  util::debug(log_tag + "disposed"s);

  G_OBJECT_CLASS(effects_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_EFFECTS_BOX(object);

  delete self->data;

  util::debug(log_tag + "finalized"s);

  G_OBJECT_CLASS(effects_box_parent_class)->finalize(object);
}

void effects_box_class_init(EffectsBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

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
  gtk_widget_class_bind_template_child(widget_class, EffectsBox, saturation_icon);

  gtk_widget_class_bind_template_callback(widget_class, stack_visible_child_changed);
  gtk_widget_class_bind_template_callback(widget_class, on_listen_mic_toggled);
}

void effects_box_init(EffectsBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  self->data->schedule_signal_idle = false;

  self->app_settings = g_settings_new("com.github.wwmm.easyeffects");
  self->settings_spectrum = g_settings_new("com.github.wwmm.easyeffects.spectrum");

  self->spectrum_chart = ui::chart::create();

  self->appsBox = ui::apps_box::create();
  self->pluginsBox = ui::plugins_box::create();
  self->blocklist_menu = ui::blocklist_menu::create();

  gtk_menu_button_set_popover(self->menubutton_blocklist, GTK_WIDGET(self->blocklist_menu));

  setup_spectrum(self);

  gtk_box_insert_child_after(GTK_BOX(self), GTK_WIDGET(self->spectrum_chart), nullptr);

  g_signal_connect(GTK_WIDGET(self->spectrum_chart), "show", G_CALLBACK(+[](GtkWidget* widget, EffectsBox* self) {
                     self->data->effects_base->spectrum->bypass = false;
                   }),
                   self);

  g_signal_connect(GTK_WIDGET(self->spectrum_chart), "hide", G_CALLBACK(+[](GtkWidget* widget, EffectsBox* self) {
                     self->data->effects_base->spectrum->bypass = true;
                   }),
                   self);
}

auto create() -> EffectsBox* {
  return static_cast<EffectsBox*>(g_object_new(EE_TYPE_EFFECTS_BOX, nullptr));
}

}  // namespace ui::effects_box
