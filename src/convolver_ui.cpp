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

auto constexpr log_tag = "convolver_box: ";
auto constexpr irs_ext = ".irs";

static std::filesystem::path irs_dir = g_get_user_config_dir() + "/easyeffects/irs"s;

struct _ConvolverBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkToggleButton* bypass;

  GtkMenuButton *menu_button_impulses, *menu_button_combine;

  GtkLabel *label_file_name, *label_sampling_rate, *label_samples, *label_duration;

  GtkSpinButton* ir_width;

  GtkCheckButton *check_left, *check_right;

  GtkToggleButton* show_fft;

  ui::convolver_menu_impulses::ConvolverMenuImpulses* impulses_menu;
  ui::convolver_menu_combine::ConvolverMenuCombine* combine_menu;

  GSettings* settings;

  GFileMonitor* folder_monitor;

  app::Application* application;

  std::shared_ptr<Convolver> convolver;

  std::mutex lock_guard_irs_info;

  std::vector<float> left_mag, right_mag, time_axis, left_spectrum, right_spectrum, freq_axis;

  std::vector<std::thread> mythreads;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

G_DEFINE_TYPE(ConvolverBox, convolver_box, GTK_TYPE_BOX)

void on_bypass(ConvolverBox* self, GtkToggleButton* btn) {
  self->convolver->bypass = gtk_toggle_button_get_active(btn);
}

void on_reset(ConvolverBox* self, GtkButton* btn) {
  gtk_toggle_button_set_active(self->bypass, 0);

  g_settings_reset(self->settings, "input-gain");

  g_settings_reset(self->settings, "output-gain");

  g_settings_reset(self->settings, "kernel-path");

  g_settings_reset(self->settings, "ir-width");
}

void setup(ConvolverBox* self,
           std::shared_ptr<Convolver> convolver,
           const std::string& schema_path,
           app::Application* application) {
  self->convolver = convolver;
  self->application = application;

  self->settings = g_settings_new_with_path("com.github.wwmm.easyeffects.convolver", schema_path.c_str());

  convolver->post_messages = true;
  convolver->bypass = false;

  ui::convolver_menu_impulses::setup(self->impulses_menu, schema_path, application);

  self->connections.push_back(convolver->input_level.connect([=](const float& left, const float& right) {
    update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                 self->input_level_right_label, left, right);
  }));

  self->connections.push_back(convolver->output_level.connect([=](const float& left, const float& right) {
    update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                 self->output_level_right_label, left, right);
  }));

  g_settings_bind(self->settings, "input-gain", gtk_range_get_adjustment(GTK_RANGE(self->input_gain)), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "output-gain", gtk_range_get_adjustment(GTK_RANGE(self->output_gain)), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "ir-width", gtk_spin_button_get_adjustment(self->ir_width), "value",
                  G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_CONVOLVER_BOX(object);

  self->convolver->post_messages = false;
  self->convolver->bypass = false;

  g_file_monitor_cancel(self->folder_monitor);

  for (auto& t : self->mythreads) {
    t.join();
  }

  self->mythreads.clear();

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

  G_OBJECT_CLASS(convolver_box_parent_class)->dispose(object);
}

void convolver_box_class_init(ConvolverBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/convolver.ui");

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

  gtk_widget_class_bind_template_child(widget_class, ConvolverBox, bypass);

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

  gtk_widget_class_bind_template_callback(widget_class, on_bypass);
  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void convolver_box_init(ConvolverBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  prepare_spinbutton<"%">(self->ir_width);

  self->impulses_menu = ui::convolver_menu_impulses::create();
  self->combine_menu = ui::convolver_menu_combine::create();

  gtk_menu_button_set_popover(self->menu_button_impulses, GTK_WIDGET(self->impulses_menu));
  gtk_menu_button_set_popover(self->menu_button_combine, GTK_WIDGET(self->combine_menu));

  // irs dir

  if (!std::filesystem::is_directory(irs_dir)) {
    if (std::filesystem::create_directories(irs_dir)) {
      util::debug(log_tag + "irs directory created: "s + irs_dir.string());
    } else {
      util::warning(log_tag + "failed to create irs directory: "s + irs_dir.string());
    }
  } else {
    util::debug(log_tag + "irs directory already exists: "s + irs_dir.string());
  }

  auto gfile = g_file_new_for_path(irs_dir.c_str());

  self->folder_monitor = g_file_monitor_directory(gfile, G_FILE_MONITOR_NONE, nullptr, nullptr);

  g_signal_connect(self->folder_monitor, "changed",
                   G_CALLBACK(+[](GFileMonitor* monitor, GFile* file, GFile* other_file, GFileMonitorEvent event_type,
                                  ConvolverBox* self) {
                     const auto irs_filename = util::remove_filename_extension(g_file_get_basename(file));

                     if (irs_filename.empty()) {
                       util::warning(log_tag + "can't retrieve information about irs file"s);

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
}

auto create() -> ConvolverBox* {
  return static_cast<ConvolverBox*>(g_object_new(EE_TYPE_CONVOLVER_BOX, nullptr));
}

}  // namespace ui::convolver_box

ConvolverUi::ConvolverUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& schema,
                         const std::string& schema_path)
    : Gtk::Box(cobject),
      PluginUiBase(builder, schema, schema_path),
      spectrum_settings(Gio::Settings::create("com.github.wwmm.easyeffects.spectrum")) {
  name = plugin_name::convolver;

  show_fft = builder->get_widget<Gtk::ToggleButton>("show_fft");
  check_left = builder->get_widget<Gtk::CheckButton>("check_left");
  check_right = builder->get_widget<Gtk::CheckButton>("check_right");

  label_sampling_rate = builder->get_widget<Gtk::Label>("label_sampling_rate");
  label_samples = builder->get_widget<Gtk::Label>("label_samples");
  label_duration = builder->get_widget<Gtk::Label>("label_duration");
  label_file_name = builder->get_widget<Gtk::Label>("label_file_name");

  drawing_area = builder->get_widget<Gtk::DrawingArea>("drawing_area");

  // plot = std::make_unique<Plot>(drawing_area);

  check_left->signal_toggled().connect([&, this]() {
    if (check_left->get_active()) {
      (show_fft->get_active()) ? plot_fft() : plot_waveform();
    }
  });

  check_right->signal_toggled().connect([&, this]() {
    if (check_right->get_active()) {
      (show_fft->get_active()) ? plot_fft() : plot_waveform();
    }
  });

  show_fft->signal_toggled().connect([=, this]() { (show_fft->get_active()) ? plot_fft() : plot_waveform(); });

  // Reading the current configured irs file

  mythreads.emplace_back([this]() {  // Using emplace_back here makes sense
    std::scoped_lock<std::mutex> lock(lock_guard_irs_info);

    get_irs_info();
  });

  /* this is necessary to update the interface with the irs info when a preset
     is loaded
  */

  connections.push_back(settings->signal_changed("kernel-path").connect([=, this](const auto& key) {
    mythreads.emplace_back([this]() {
      std::scoped_lock<std::mutex> lock(lock_guard_irs_info);

      get_irs_info();
    });
  }));
}

ConvolverUi::~ConvolverUi() {
  util::debug(name + " ui destroyed");
}

void ConvolverUi::get_irs_info() {
  const auto path = settings->get_string("kernel-path");

  if (path.empty()) {
    util::warning(log_tag + name + ": irs file path is null.");

    return;
  }

  auto [rate, kernel_L, kernel_R] = ui::convolver::read_kernel(log_tag, irs_dir, irs_ext, path);

  if (rate == 0) {
    // warning user that there is a problem

    connections.push_back(Glib::signal_idle().connect([=, this]() {
      label_sampling_rate->set_text(_("Failed"));
      label_samples->set_text(_("Failed"));

      label_duration->set_text(_("Failed"));

      label_file_name->set_text(_("Could Not Load The Impulse File"));

      return false;
    }));

    return;
  }

  const float dt = 1.0F / static_cast<float>(rate);

  const float duration = (static_cast<float>(kernel_L.size()) - 1.0F) * dt;

  time_axis.resize(kernel_L.size());

  left_mag = kernel_L;
  right_mag = kernel_R;

  for (size_t n = 0; n < time_axis.size(); n++) {
    time_axis[n] = static_cast<float>(n) * dt;
  }

  get_irs_spectrum(rate);

  size_t bin_size = (drawing_area->get_width() > 0) ? time_axis.size() / drawing_area->get_width() : 0;

  if (bin_size > 0) {
    // decimating the data so we can draw it

    std::vector<float> t;
    std::vector<float> l;
    std::vector<float> r;
    std::vector<float> bin_x;
    std::vector<float> bin_l_y;
    std::vector<float> bin_r_y;

    for (size_t n = 0; n < time_axis.size(); n++) {
      bin_x.push_back(time_axis[n]);

      bin_l_y.push_back(left_mag[n]);
      bin_r_y.push_back(right_mag[n]);

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

    time_axis = t;
    left_mag = l;
    right_mag = r;
  }

  time_axis.shrink_to_fit();
  left_mag.shrink_to_fit();
  right_mag.shrink_to_fit();

  // find min and max values

  const auto min_left = std::ranges::min(left_mag);
  const auto max_left = std::ranges::max(left_mag);

  const auto min_right = std::ranges::min(right_mag);
  const auto max_right = std::ranges::max(right_mag);

  // rescaling between 0 and 1

  for (size_t n = 0U; n < left_mag.size(); n++) {
    left_mag[n] = (left_mag[n] - min_left) / (max_left - min_left);
    right_mag[n] = (right_mag[n] - min_right) / (max_right - min_right);
  }

  // updating interface with ir file info

  auto rate_copy = rate;
  auto n_samples = kernel_L.size();

  connections.push_back(Glib::signal_idle().connect([=, this]() {
    label_sampling_rate->set_text(Glib::ustring::format(rate_copy) + " Hz");
    label_samples->set_text(Glib::ustring::format(n_samples));

    label_duration->set_text(level_to_localized_string(duration, 3) + " s");

    const auto fpath = std::filesystem::path{path.raw()};

    label_file_name->set_text(fpath.stem().c_str());

    if (!show_fft->get_active()) {
      plot_waveform();
    }

    return false;
  }));
}

void ConvolverUi::get_irs_spectrum(const int& rate) {
  if (left_mag.empty() || right_mag.empty() || left_mag.size() != right_mag.size()) {
    util::debug(log_tag + name + " aborting the impulse fft calculation...");

    return;
  }

  util::debug(log_tag + name + " calculating the impulse fft...");

  left_spectrum.resize(left_mag.size() / 2U + 1U);
  right_spectrum.resize(right_mag.size() / 2U + 1U);

  auto real_input = left_mag;

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

  for (uint i = 0U; i < left_spectrum.size(); i++) {
    float sqr = complex_output[i][0] * complex_output[i][0] + complex_output[i][1] * complex_output[i][1];

    sqr /= static_cast<float>(left_spectrum.size() * left_spectrum.size());

    left_spectrum[i] = sqr;
  }

  // right channel fft

  real_input = right_mag;

  for (uint n = 0U; n < real_input.size(); n++) {
    // https://en.wikipedia.org/wiki/Hann_function

    const float w = 0.5F * (1.0F - std::cos(2.0F * std::numbers::pi_v<float> * static_cast<float>(n) /
                                            static_cast<float>(real_input.size() - 1U)));

    real_input[n] *= w;
  }

  fftwf_execute(plan);

  for (uint i = 0U; i < right_spectrum.size(); i++) {
    float sqr = complex_output[i][0] * complex_output[i][0] + complex_output[i][1] * complex_output[i][1];

    sqr /= static_cast<float>(right_spectrum.size() * right_spectrum.size());

    right_spectrum[i] = sqr;
  }

  // cleaning

  if (complex_output != nullptr) {
    fftwf_free(complex_output);
  }

  fftwf_destroy_plan(plan);

  // initializing the frequency axis

  freq_axis.resize(left_spectrum.size());

  for (uint n = 0U; n < left_spectrum.size(); n++) {
    freq_axis[n] = 0.5F * static_cast<float>(rate) * static_cast<float>(n) / static_cast<float>(left_spectrum.size());
  }

  size_t bin_size = (drawing_area->get_width() > 0) ? drawing_area->get_width() : 100;

  // initializing the logarithmic frequency axis

  const auto log_axis = util::logspace(20.0F, 22000.0F, bin_size);
  // const auto log_axis = util::logspace(20.0F, 22000.0F, freq_axis.size());
  // auto log_axis = util::linspace(20.0F, 22000.0F, spectrum_settings->get_int("n-points"));

  std::vector<float> l(log_axis.size());
  std::vector<float> r(log_axis.size());
  std::vector<uint> bin_count(log_axis.size());

  std::ranges::fill(l, 0.0F);
  std::ranges::fill(r, 0.0F);
  std::ranges::fill(bin_count, 0U);

  // reducing the amount of data we have to plot and converting the frequency axis to the logarithimic scale

  for (size_t j = 0U; j < freq_axis.size(); j++) {
    for (size_t n = 0U; n < log_axis.size(); n++) {
      if (n > 0U) {
        if (freq_axis[j] <= log_axis[n] && freq_axis[j] > log_axis[n - 1U]) {
          l[n] += left_spectrum[j];
          r[n] += right_spectrum[j];

          bin_count[n]++;
        }
      } else {
        if (freq_axis[j] <= log_axis[n]) {
          l[n] += left_spectrum[j];
          r[n] += right_spectrum[j];

          bin_count[n]++;
        }
      }
    }
  }

  // fillint empty bins with their neighbors value

  for (size_t n = 0U; n < bin_count.size(); n++) {
    if (bin_count[n] == 0U && n > 0U) {
      l[n] = l[n - 1U];
      r[n] = r[n - 1U];
    }
  }

  freq_axis = log_axis;
  left_spectrum = l;
  right_spectrum = r;

  // find min and max values

  const auto fft_min_left = std::ranges::min(left_spectrum);
  const auto fft_max_left = std::ranges::max(left_spectrum);

  const auto fft_min_right = std::ranges::min(right_spectrum);
  const auto fft_max_right = std::ranges::max(right_spectrum);

  // rescaling between 0 and 1

  for (uint n = 0; n < left_spectrum.size(); n++) {
    left_spectrum[n] = (left_spectrum[n] - fft_min_left) / (fft_max_left - fft_min_left);
    right_spectrum[n] = (right_spectrum[n] - fft_min_right) / (fft_max_right - fft_min_right);
  }

  connections.push_back(Glib::signal_idle().connect([=, this]() {
    if (show_fft->get_active()) {
      plot_fft();
    }

    return false;
  }));
}

void ConvolverUi::plot_waveform() {
  if (time_axis.empty() || left_spectrum.empty() || right_spectrum.empty()) {
    return;
  }

  // plot->set_plot_type(PlotType::line);

  // plot->set_plot_scale(PlotScale::linear);

  // plot->set_fill_bars(false);

  // plot->set_line_width(static_cast<float>(spectrum_settings->get_double("line-width")));

  // plot->set_x_unit("s");
  // plot->set_n_x_decimals(2);
  // plot->set_n_y_decimals(2);

  // if (check_left->get_active()) {
  //   plot->set_data(time_axis, left_mag);
  // } else if (check_right->get_active()) {
  //   plot->set_data(time_axis, right_mag);
  // }
}

void ConvolverUi::plot_fft() {
  if (freq_axis.empty() || left_spectrum.empty() || right_spectrum.empty()) {
    return;
  }

  // plot->set_plot_type(PlotType::line);

  // plot->set_plot_scale(PlotScale::logarithmic);

  // plot->set_fill_bars(false);

  // plot->set_line_width(static_cast<float>(spectrum_settings->get_double("line-width")));

  // plot->set_x_unit("Hz");
  // plot->set_n_x_decimals(0);
  // plot->set_n_y_decimals(2);

  // if (check_left->get_active()) {
  //   plot->set_data(freq_axis, left_spectrum);
  // } else if (check_right->get_active()) {
  //   plot->set_data(freq_axis, right_spectrum);
  // }
}
