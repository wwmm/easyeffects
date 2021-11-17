#include "effects_box.hpp"

namespace ui::effects_box {

using namespace std::string_literals;

auto constexpr log_tag = "effects_box: ";

struct _EffectsBox {
  GtkBox parent_instance{};

  AdwViewStack* stack;

  ui::chart::Chart* spectrum_chart;

  GSettings *settings, *settings_spectrum, *app_settings;

  app::Application* application;

  EffectsBase* effects_base;

  uint spectrum_rate, spectrum_n_bands;

  std::vector<float> spectrum_mag, spectrum_freqs, spectrum_x_axis;

  std::vector<uint> spectrum_bin_count;

  std::vector<sigc::connection> connections;
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

  g_signal_connect(
      self->settings_spectrum, "changed::color", G_CALLBACK((+[](GSettings* settings, char* key, EffectsBox* self) {
        ui::chart::set_color(self->spectrum_chart, util::gsettings_get_color(self->settings_spectrum, key));
      })),
      self);

  g_signal_connect(self->settings_spectrum, "changed::color-axis-labels",
                   G_CALLBACK((+[](GSettings* settings, char* key, EffectsBox* self) {
                     ui::chart::set_axis_labels_color(self->spectrum_chart,
                                                      util::gsettings_get_color(self->settings_spectrum, key));
                   })),
                   self);

  g_signal_connect(
      self->settings_spectrum, "changed::fill", G_CALLBACK((+[](GSettings* settings, char* key, EffectsBox* self) {
        ui::chart::set_fill_bars(self->spectrum_chart, g_settings_get_boolean(self->settings_spectrum, "fill") != 0);
      })),
      self);

  g_signal_connect(self->settings_spectrum, "changed::show-bar-border",
                   G_CALLBACK((+[](GSettings* settings, char* key, EffectsBox* self) {
                     ui::chart::set_draw_bar_border(self->spectrum_chart,
                                                    g_settings_get_boolean(self->settings_spectrum, "fill") != 0);
                   })),
                   self);

  g_signal_connect(self->settings_spectrum, "changed::line-width",
                   G_CALLBACK((+[](GSettings* settings, char* key, EffectsBox* self) {
                     ui::chart::set_line_width(self->spectrum_chart,
                                               g_settings_get_double(self->settings_spectrum, "line-width"));
                   })),
                   self);

  g_signal_connect(self->settings_spectrum, "changed::height",
                   G_CALLBACK((+[](GSettings* settings, char* key, EffectsBox* self) {
                     gtk_widget_set_size_request(GTK_WIDGET(self->spectrum_chart), -1,
                                                 g_settings_get_int(self->settings_spectrum, "height"));
                   })),
                   self);

  g_signal_connect(self->settings_spectrum, "changed::type",
                   G_CALLBACK((+[](GSettings* settings, char* key, EffectsBox* self) {
                     if (g_strcmp0(g_settings_get_string(self->settings_spectrum, key), "Bars") == 0) {
                       ui::chart::set_plot_type(self->spectrum_chart, chart::ChartType::bar);
                     } else if (g_strcmp0(g_settings_get_string(self->settings_spectrum, key), "Lines") == 0) {
                       ui::chart::set_plot_type(self->spectrum_chart, chart::ChartType::line);
                     }

                     util::warning("connected");
                   })),
                   self);
}

void setup(EffectsBox* self, app::Application* application, PipelineType pipeline_type) {
  self->application = application;

  switch (pipeline_type) {
    case PipelineType::input: {
      self->effects_base = static_cast<EffectsBase*>(self->application->sie.get());

      self->settings = g_settings_new("com.github.wwmm.easyeffects.streamoutputs");

      break;
    }
    case PipelineType::output: {
      self->effects_base = static_cast<EffectsBase*>(self->application->soe.get());

      self->settings = g_settings_new("com.github.wwmm.easyeffects.streaminputs");

      break;
    }
  }

  self->connections.push_back(
      self->effects_base->spectrum->power.connect([=](uint rate, uint n_bands, std::vector<float> magnitudes) {
        if (!ui::chart::get_is_visible(self->spectrum_chart)) {
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
      }));

  self->effects_base->spectrum->post_messages = gtk_widget_get_visible(GTK_WIDGET(self->spectrum_chart)) != 0;

  self->effects_base->spectrum->bypass = false;
}

void dispose(GObject* object) {
  auto* self = EE_EFFECTS_BOX(object);

  self->effects_base->spectrum->post_messages = false;

  for (auto& c : self->connections) {
    c.disconnect();
  }

  g_object_unref(self->settings);
  g_object_unref(self->app_settings);
  g_object_unref(self->settings_spectrum);

  util::debug(log_tag + "destroyed"s);

  G_OBJECT_CLASS(effects_box_parent_class)->dispose(object);
}

void effects_box_class_init(EffectsBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/effects_box.ui");

  gtk_widget_class_bind_template_child(widget_class, EffectsBox, stack);
}

void effects_box_init(EffectsBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->app_settings = g_settings_new("com.github.wwmm.easyeffects");
  self->settings_spectrum = g_settings_new("com.github.wwmm.easyeffects.spectrum");

  self->spectrum_chart = ui::chart::create();

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