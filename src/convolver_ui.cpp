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

#include "convolver_ui.hpp"

namespace ui::convolver_box {

using namespace std::string_literals;

auto constexpr irs_ext = ".irs";

static std::filesystem::path irs_dir = g_get_user_config_dir() + "/easyeffects/irs"s;

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  uint serial = 0;

  app::Application* application;

  std::shared_ptr<Convolver> convolver;

  std::mutex lock_guard_irs_info;

  std::vector<std::thread> mythreads;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;

  std::vector<float> left_mag, right_mag, time_axis, left_spectrum, right_spectrum, freq_axis;
};

struct _ConvolverBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkMenuButton *menu_button_impulses, *menu_button_combine;

  GtkLabel *label_file_name, *label_sampling_rate, *label_samples, *label_duration;

  GtkSpinButton* ir_width;

  GtkCheckButton *check_left, *check_right;

  GtkToggleButton *show_fft, *enable_log_scale;

  GtkBox* chart_box;

  ui::convolver_menu_impulses::ConvolverMenuImpulses* impulses_menu;

  ui::convolver_menu_combine::ConvolverMenuCombine* combine_menu;

  ui::chart::Chart* chart;

  GSettings* settings;

  GFileMonitor* folder_monitor;

  Data* data;

  GtkToggleButton* autogain;
};

G_DEFINE_TYPE(ConvolverBox, convolver_box, GTK_TYPE_BOX)

void on_reset(ConvolverBox* self, GtkButton* btn) {
  gtk_toggle_button_set_active(self->autogain, 0);

  util::reset_all_keys_except(self->settings);
}

void plot_fft(ConvolverBox* self) {
  if (self->data->freq_axis.empty() || self->data->left_spectrum.empty() || self->data->right_spectrum.empty()) {
    return;
  }

  ui::chart::set_chart_type(self->chart, ui::chart::ChartType::line);
  ui::chart::set_fill_bars(self->chart, false);
  ui::chart::set_line_width(self->chart, 2.0F);
  ui::chart::set_n_x_decimals(self->chart, 0);
  ui::chart::set_n_y_decimals(self->chart, 2);
  ui::chart::set_x_unit(self->chart, "Hz");

  if (gtk_toggle_button_get_active(self->enable_log_scale) != 0) {
    ui::chart::set_chart_scale(self->chart, ui::chart::ChartScale::logarithmic);
  } else {
    ui::chart::set_chart_scale(self->chart, ui::chart::ChartScale::linear);
  }

  ui::chart::set_x_data(self->chart, self->data->freq_axis);

  if (gtk_check_button_get_active(self->check_left) != 0) {
    ui::chart::set_y_data(self->chart, self->data->left_spectrum);
  } else if (gtk_check_button_get_active(self->check_right) != 0) {
    ui::chart::set_y_data(self->chart, self->data->right_spectrum);
  }
}

void plot_waveform(ConvolverBox* self) {
  if (self->data->time_axis.empty() || self->data->left_spectrum.empty() || self->data->right_spectrum.empty()) {
    return;
  }

  ui::chart::set_chart_type(self->chart, ui::chart::ChartType::line);
  ui::chart::set_chart_scale(self->chart, ui::chart::ChartScale::linear);
  ui::chart::set_fill_bars(self->chart, false);
  ui::chart::set_line_width(self->chart, 2.0F);
  ui::chart::set_n_x_decimals(self->chart, 2);
  ui::chart::set_n_y_decimals(self->chart, 2);
  ui::chart::set_x_unit(self->chart, "s");

  ui::chart::set_x_data(self->chart, self->data->time_axis);

  if (gtk_check_button_get_active(self->check_left) != 0) {
    ui::chart::set_y_data(self->chart, self->data->left_mag);
  } else if (gtk_check_button_get_active(self->check_right) != 0) {
    ui::chart::set_y_data(self->chart, self->data->right_mag);
  }
}

void on_show_fft(ConvolverBox* self, GtkToggleButton* btn) {
  if (gtk_toggle_button_get_active(btn) != 0) {
    plot_fft(self);
  } else {
    plot_waveform(self);
  }
}

void on_show_channel(ConvolverBox* self, GtkCheckButton* btn) {
  if (gtk_check_button_get_active(btn) != 0) {
    on_show_fft(self, self->show_fft);
  }
}

void on_enable_log_scale(ConvolverBox* self, GtkToggleButton* btn) {
  plot_fft(self);
}

void get_irs_spectrum(ConvolverBox* self, const int& rate) {
  if (self->data->left_mag.empty() || self->data->right_mag.empty() ||
      self->data->left_mag.size() != self->data->right_mag.size()) {
    util::debug(" aborting the impulse fft calculation...");

    return;
  }

  util::debug(" calculating the impulse fft...");

  self->data->left_spectrum.resize(self->data->left_mag.size() / 2U + 1U);
  self->data->right_spectrum.resize(self->data->right_mag.size() / 2U + 1U);

  auto real_input = self->data->left_mag;

  for (uint n = 0U; n < real_input.size(); n++) {
    // https://en.wikipedia.org/wiki/Hann_function

    const float w = 0.5F * (1.0F - std::cos(2.0F * std::numbers::pi_v<float> * static_cast<float>(n) /
                                            static_cast<float>(real_input.size() - 1U)));

    real_input[n] *= w;
  }

  auto* complex_output = fftwf_alloc_complex(real_input.size());

  auto* plan =
      fftwf_plan_dft_r2c_1d(static_cast<int>(real_input.size()), real_input.data(), complex_output, FFTW_ESTIMATE);

  fftwf_execute(plan);

  for (uint i = 0U; i < self->data->left_spectrum.size(); i++) {
    float sqr = complex_output[i][0] * complex_output[i][0] + complex_output[i][1] * complex_output[i][1];

    sqr /= static_cast<float>(self->data->left_spectrum.size() * self->data->left_spectrum.size());

    self->data->left_spectrum[i] = sqr;
  }

  // right channel fft

  real_input = self->data->right_mag;

  for (uint n = 0U; n < real_input.size(); n++) {
    // https://en.wikipedia.org/wiki/Hann_function

    const float w = 0.5F * (1.0F - std::cos(2.0F * std::numbers::pi_v<float> * static_cast<float>(n) /
                                            static_cast<float>(real_input.size() - 1U)));

    real_input[n] *= w;
  }

  fftwf_execute(plan);

  for (uint i = 0U; i < self->data->right_spectrum.size(); i++) {
    float sqr = complex_output[i][0] * complex_output[i][0] + complex_output[i][1] * complex_output[i][1];

    sqr /= static_cast<float>(self->data->right_spectrum.size() * self->data->right_spectrum.size());

    self->data->right_spectrum[i] = sqr;
  }

  // cleaning

  if (complex_output != nullptr) {
    fftwf_free(complex_output);
  }

  fftwf_destroy_plan(plan);

  // initializing the frequency axis

  self->data->freq_axis.resize(self->data->left_spectrum.size());

  for (uint n = 0U; n < self->data->freq_axis.size(); n++) {
    self->data->freq_axis[n] =
        0.5F * static_cast<float>(rate) * static_cast<float>(n) / static_cast<float>(self->data->freq_axis.size());
  }

  // removing the DC component at f = 0 Hz

  self->data->freq_axis.erase(self->data->freq_axis.begin());
  self->data->left_spectrum.erase(self->data->left_spectrum.begin());
  self->data->right_spectrum.erase(self->data->right_spectrum.begin());

  const auto chart_width = static_cast<uint>(gtk_widget_get_width(GTK_WIDGET(self->chart)));
  const auto bin_size = (chart_width > 0U) ? chart_width : 500U;

  // initializing the logarithmic frequency axis

  float max_freq = std::ranges::max(self->data->freq_axis);
  float min_freq = std::ranges::min(self->data->freq_axis);

  util::debug("min fft frequency: " + util::to_string(min_freq, ""));
  util::debug("max fft frequency: " + util::to_string(max_freq, ""));

  const auto log_axis = util::logspace(min_freq, max_freq, bin_size);
  // const auto log_axis = util::logspace(20.0F, 22000.0F, self->data->freq_axis.size());

  std::vector<float> l(log_axis.size());
  std::vector<float> r(log_axis.size());

  std::ranges::fill(l, 0.0F);
  std::ranges::fill(r, 0.0F);

  // reducing the amount of data we have to plot and converting the frequency axis to the logarithimic scale

  size_t last_j = 0U;

  for (size_t n = 0U; n < log_axis.size(); n++) {
    for (size_t j = last_j; j < self->data->freq_axis.size(); j++) {
      if (self->data->freq_axis[j] > log_axis[n]) {
        last_j = j;

        break;
      } else {
        l[n] += self->data->left_spectrum[j];
        r[n] += self->data->right_spectrum[j];
      }
    }
  }

  self->data->freq_axis = log_axis;
  self->data->left_spectrum = l;
  self->data->right_spectrum = r;

  // find min and max values

  const auto fft_min_left = std::ranges::min(self->data->left_spectrum);
  const auto fft_max_left = std::ranges::max(self->data->left_spectrum);

  const auto fft_min_right = std::ranges::min(self->data->right_spectrum);
  const auto fft_max_right = std::ranges::max(self->data->right_spectrum);

  // rescaling between 0 and 1

  for (uint n = 0; n < self->data->left_spectrum.size(); n++) {
    self->data->left_spectrum[n] = (self->data->left_spectrum[n] - fft_min_left) / (fft_max_left - fft_min_left);
    self->data->right_spectrum[n] = (self->data->right_spectrum[n] - fft_min_right) / (fft_max_right - fft_min_right);
  }

  util::idle_add([=]() {
    if (!ui::chart::get_is_visible(self->chart)) {
      return;
    }

    if (gtk_toggle_button_get_active(self->show_fft) != 0) {
      plot_fft(self);
    }
  });
}

void get_irs_info(ConvolverBox* self) {
  const std::string path = util::gsettings_get_string(self->settings, "kernel-path");

  if (path.empty()) {
    util::warning(": irs file path is null.");

    // Set label to initial empty state
    gtk_widget_remove_css_class(GTK_WIDGET(self->label_file_name), "error");
    gtk_widget_add_css_class(GTK_WIDGET(self->label_file_name), "dim-label");
    gtk_label_set_text(self->label_file_name, _("No Impulse File Loaded"));

    gtk_label_set_text(self->label_sampling_rate, "");
    gtk_label_set_text(self->label_samples, "");
    gtk_label_set_text(self->label_duration, "");

    return;
  }

  auto [rate, kernel_L, kernel_R] = ui::convolver::read_kernel(irs_dir, irs_ext, path);

  if (rate == 0) {
    // warning the user that there is a problem

    util::idle_add([=]() {
      if (!ui::chart::get_is_visible(self->chart)) {
        return;
      }

      // Move label to error state
      gtk_widget_remove_css_class(GTK_WIDGET(self->label_file_name), "dim-label");
      gtk_widget_add_css_class(GTK_WIDGET(self->label_file_name), "error");
      gtk_label_set_text(self->label_file_name, _("Failed To Load The Impulse File"));

      gtk_label_set_text(self->label_sampling_rate, "");
      gtk_label_set_text(self->label_samples, "");
      gtk_label_set_text(self->label_duration, "");
    });

    return;
  }

  const float dt = 1.0F / static_cast<float>(rate);

  const float duration = (static_cast<float>(kernel_L.size()) - 1.0F) * dt;

  self->data->time_axis.resize(kernel_L.size());

  self->data->left_mag = kernel_L;
  self->data->right_mag = kernel_R;

  for (size_t n = 0U; n < self->data->time_axis.size(); n++) {
    self->data->time_axis[n] = static_cast<float>(n) * dt;
  }

  get_irs_spectrum(self, rate);

  const auto chart_width = static_cast<uint>(gtk_widget_get_width(GTK_WIDGET(self->chart)));
  const auto bin_size = (chart_width > 0U) ? static_cast<uint>(self->data->time_axis.size() / chart_width) : 0U;

  if (bin_size > 0U) {
    // decimating the data so we can draw it

    std::vector<float> t;
    std::vector<float> l;
    std::vector<float> r;
    std::vector<float> bin_x;
    std::vector<float> bin_l_y;
    std::vector<float> bin_r_y;

    for (size_t n = 0U; n < self->data->time_axis.size(); n++) {
      bin_x.push_back(self->data->time_axis[n]);

      bin_l_y.push_back(self->data->left_mag[n]);
      bin_r_y.push_back(self->data->right_mag[n]);

      if (bin_x.size() == bin_size) {
        const auto [min, max] = std::ranges::minmax_element(bin_l_y);

        t.push_back(bin_x[min - bin_l_y.begin()]);
        t.push_back(bin_x[max - bin_l_y.begin()]);

        l.push_back(*min);
        l.push_back(*max);

        const auto [minr, maxr] = std::ranges::minmax_element(bin_r_y);

        r.push_back(*minr);
        r.push_back(*maxr);

        bin_x.resize(0);
        bin_l_y.resize(0);
        bin_r_y.resize(0);
      }
    }

    self->data->time_axis = t;
    self->data->left_mag = l;
    self->data->right_mag = r;
  }

  self->data->time_axis.shrink_to_fit();
  self->data->left_mag.shrink_to_fit();
  self->data->right_mag.shrink_to_fit();

  // find min and max values

  const auto min_left = std::ranges::min(self->data->left_mag);
  const auto max_left = std::ranges::max(self->data->left_mag);

  const auto min_right = std::ranges::min(self->data->right_mag);
  const auto max_right = std::ranges::max(self->data->right_mag);

  // rescaling between 0 and 1

  for (size_t n = 0U; n < self->data->left_mag.size(); n++) {
    self->data->left_mag[n] = (self->data->left_mag[n] - min_left) / (max_left - min_left);
    self->data->right_mag[n] = (self->data->right_mag[n] - min_right) / (max_right - min_right);
  }

  // updating interface with ir file info

  auto rate_copy = rate;
  auto n_samples = kernel_L.size();

  util::idle_add([=]() {
    if (!ui::chart::get_is_visible(self->chart)) {
      return;
    }

    const auto fpath = std::filesystem::path{path};

    // Set label to ready state and update with filename
    gtk_widget_remove_css_class(GTK_WIDGET(self->label_file_name), "error");
    gtk_widget_add_css_class(GTK_WIDGET(self->label_file_name), "dim-label");
    gtk_label_set_text(self->label_file_name, fpath.stem().c_str());

    gtk_label_set_text(self->label_sampling_rate, fmt::format(ui::get_user_locale(), "{0:Ld} Hz", rate_copy).c_str());
    gtk_label_set_text(self->label_samples, fmt::format(ui::get_user_locale(), "{0:Ld}", n_samples).c_str());
    gtk_label_set_text(self->label_duration, fmt::format(ui::get_user_locale(), "{0:.3Lf}", duration).c_str());

    if (gtk_toggle_button_get_active(self->show_fft) == 0) {
      plot_waveform(self);
    }
  });
}

void setup(ConvolverBox* self,
           std::shared_ptr<Convolver> convolver,
           const std::string& schema_path,
           app::Application* application) {
  self->data->convolver = convolver;
  self->data->application = application;

  auto serial = get_new_filter_serial();

  self->data->serial = serial;

  g_object_set_data(G_OBJECT(self), "serial", GUINT_TO_POINTER(serial));

  set_ignore_filter_idle_add(serial, false);

  self->settings = g_settings_new_with_path(tags::schema::convolver::id, schema_path.c_str());

  convolver->set_post_messages(true);

  ui::convolver_menu_impulses::setup(self->impulses_menu, schema_path, application);

  self->data->connections.push_back(convolver->input_level.connect([=](const float left, const float right) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                   self->input_level_right_label, left, right);
    });
  }));

  self->data->connections.push_back(convolver->output_level.connect([=](const float left, const float right) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                   self->output_level_right_label, left, right);
    });
  }));

  self->data->gconnections.push_back(g_signal_connect(
      self->settings, "changed::kernel-path", G_CALLBACK(+[](GSettings* settings, char* key, ConvolverBox* self) {
        self->data->mythreads.emplace_back([=]() {
          std::scoped_lock<std::mutex> lock(self->data->lock_guard_irs_info);

          get_irs_info(self);
        });
      }),
      self));

  gsettings_bind_widgets<"input-gain", "output-gain", "autogain">(self->settings, self->input_gain, self->output_gain,
                                                                  self->autogain);

  g_settings_bind(self->settings, "ir-width", gtk_spin_button_get_adjustment(self->ir_width), "value",
                  G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_CONVOLVER_BOX(object);

  self->data->convolver->set_post_messages(false);

  set_ignore_filter_idle_add(self->data->serial, true);

  g_file_monitor_cancel(self->folder_monitor);

  g_object_unref(self->folder_monitor);

  for (auto& t : self->data->mythreads) {
    t.join();
  }

  self->data->mythreads.clear();

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

  G_OBJECT_CLASS(convolver_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_CONVOLVER_BOX(object);

  for (auto& t : self->data->mythreads) {
    t.join();
  }

  self->data->mythreads.clear();

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(convolver_box_parent_class)->finalize(object);
}

void convolver_box_class_init(ConvolverBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::convolver_ui);

  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, menu_button_impulses);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, menu_button_combine);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, label_file_name);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, label_sampling_rate);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, label_samples);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, label_duration);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, ir_width);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, check_left);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, check_right);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, show_fft);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, enable_log_scale);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, chart_box);
  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, autogain);

  gtk_widget_class_bind_template_callback(widget_class, on_reset);
  gtk_widget_class_bind_template_callback(widget_class, on_show_fft);
  gtk_widget_class_bind_template_callback(widget_class, on_show_channel);
  gtk_widget_class_bind_template_callback(widget_class, on_enable_log_scale);
}

void convolver_box_init(ConvolverBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  // irs dir

  if (!std::filesystem::is_directory(irs_dir)) {
    if (std::filesystem::create_directories(irs_dir)) {
      util::debug("irs directory created: " + irs_dir.string());
    } else {
      util::warning("failed to create irs directory: " + irs_dir.string());
    }
  } else {
    util::debug("irs directory already exists: " + irs_dir.string());
  }

  prepare_spinbuttons<"%">(self->ir_width);

  prepare_scales<"dB">(self->input_gain, self->output_gain);

  self->chart = ui::chart::create();

  gtk_widget_set_vexpand(GTK_WIDGET(self->chart), 1);

  self->impulses_menu = ui::convolver_menu_impulses::create();
  self->combine_menu = ui::convolver_menu_combine::create();

  gtk_menu_button_set_popover(self->menu_button_impulses, GTK_WIDGET(self->impulses_menu));
  gtk_menu_button_set_popover(self->menu_button_combine, GTK_WIDGET(self->combine_menu));

  gtk_box_insert_child_after(self->chart_box, GTK_WIDGET(self->chart),
                             gtk_widget_get_first_child(GTK_WIDGET(self->chart_box)));

  // setting some signals

  auto gfile = g_file_new_for_path(irs_dir.c_str());

  self->folder_monitor = g_file_monitor_directory(gfile, G_FILE_MONITOR_NONE, nullptr, nullptr);

  g_signal_connect(self->folder_monitor, "changed",
                   G_CALLBACK(+[](GFileMonitor* monitor, GFile* file, GFile* other_file, GFileMonitorEvent event_type,
                                  ConvolverBox* self) {
                     const auto irs_filename = util::remove_filename_extension(g_file_get_basename(file));

                     if (irs_filename.empty()) {
                       util::warning("can't retrieve information about irs file");

                       return;
                     }

                     switch (event_type) {
                       case G_FILE_MONITOR_EVENT_CREATED: {
                         ui::convolver_menu_impulses::append_to_string_list(self->impulses_menu, irs_filename);
                         ui::convolver_menu_combine::append_to_string_list(self->combine_menu, irs_filename);

                         break;
                       }

                       case G_FILE_MONITOR_EVENT_DELETED: {
                         ui::convolver_menu_impulses::remove_from_string_list(self->impulses_menu, irs_filename);
                         ui::convolver_menu_combine::remove_from_string_list(self->combine_menu, irs_filename);

                         break;
                       }

                       default:
                         break;
                     }
                   }),
                   self);

  g_object_unref(gfile);

  g_signal_connect(GTK_WIDGET(self), "realize", G_CALLBACK(+[](GtkWidget* widget, ConvolverBox* self) {
                     /*
                       Reading the current configured irs file. We do this here because we need some widgets to be ready
                       when the impulse response file information is available
                     */

                     self->data->mythreads.emplace_back([=]() {  // Using emplace_back here makes sense
                       std::scoped_lock<std::mutex> lock(self->data->lock_guard_irs_info);

                       get_irs_info(self);
                     });
                   }),
                   self);
}

auto create() -> ConvolverBox* {
  return static_cast<ConvolverBox*>(g_object_new(EE_TYPE_CONVOLVER_BOX, nullptr));
}

}  // namespace ui::convolver_box
