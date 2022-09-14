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

#include "convolver_menu_combine.hpp"

namespace ui::convolver_menu_combine {

using namespace std::string_literals;

auto constexpr irs_ext = ".irs";

std::filesystem::path irs_dir = g_get_user_config_dir() + "/easyeffects/irs"s;

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  std::vector<std::thread> mythreads;
};

struct _ConvolverMenuCombine {
  GtkBox parent_instance;

  GtkDropDown *dropdown_kernel_1, *dropdown_kernel_2;

  GtkEntry* output_kernel_name;

  GtkSpinner* spinner;

  GtkStringList *string_list_1, *string_list_2;

  GSettings* app_settings;

  Data* data;
};

G_DEFINE_TYPE(ConvolverMenuCombine, convolver_menu_combine, GTK_TYPE_POPOVER)

void append_to_string_list(ConvolverMenuCombine* self, const std::string& irs_filename) {
  ui::append_to_string_list(self->string_list_1, irs_filename);
  ui::append_to_string_list(self->string_list_2, irs_filename);
}

void remove_from_string_list(ConvolverMenuCombine* self, const std::string& irs_filename) {
  ui::remove_from_string_list(self->string_list_1, irs_filename);
  ui::remove_from_string_list(self->string_list_2, irs_filename);
}

void direct_conv(const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& c) {
  std::vector<size_t> indices(c.size());

  std::iota(indices.begin(), indices.end(), 0U);

  std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&](const int n) {
    c[n] = 0.0F;

    // Static cast to avoid gcc signedness warning.

    const int a_size = static_cast<int>(a.size());
    const int b_size = static_cast<int>(b.size());

    for (int m = 0; m < b_size; m++) {
      if (const auto z = n - m; z >= 0 && z < a_size - 1) {
        c[n] += b[m] * a[z];
      }
    }
  });
}

void combine_kernels(ConvolverMenuCombine* self,
                     const std::string& kernel_1_name,
                     const std::string& kernel_2_name,
                     const std::string& output_file_name) {
  if (output_file_name.empty()) {
    // The method combine_kernels run in a secondary thread. But the widgets have to be used in the main thread.

    util::idle_add([=] { gtk_spinner_stop(self->spinner); });

    return;
  }

  auto [rate1, kernel_1_L, kernel_1_R] = ui::convolver::read_kernel(irs_dir, irs_ext, kernel_1_name);
  auto [rate2, kernel_2_L, kernel_2_R] = ui::convolver::read_kernel(irs_dir, irs_ext, kernel_2_name);

  if (rate1 == 0 || rate2 == 0) {
    util::idle_add([=] { gtk_spinner_stop(self->spinner); });

    return;
  }

  if (rate1 > rate2) {
    util::debug("resampling the kernel " + kernel_2_name + " to " + util::to_string(rate1) + " Hz");

    auto resampler = std::make_unique<Resampler>(rate2, rate1);

    kernel_2_L = resampler->process(kernel_2_L, true);

    resampler = std::make_unique<Resampler>(rate2, rate1);

    kernel_2_R = resampler->process(kernel_2_R, true);
  } else if (rate2 > rate1) {
    util::debug("resampling the kernel " + kernel_1_name + " to " + util::to_string(rate2) + " Hz");

    auto resampler = std::make_unique<Resampler>(rate1, rate2);

    kernel_1_L = resampler->process(kernel_1_L, true);

    resampler = std::make_unique<Resampler>(rate1, rate2);

    kernel_1_R = resampler->process(kernel_1_R, true);
  }

  std::vector<float> kernel_L(kernel_1_L.size() + kernel_2_L.size() - 1U);
  std::vector<float> kernel_R(kernel_1_R.size() + kernel_2_R.size() - 1U);

  // As the convolution is commutative we change the order based on which will run faster.

  if (kernel_1_L.size() > kernel_2_L.size()) {
    direct_conv(kernel_1_L, kernel_2_L, kernel_L);
    direct_conv(kernel_1_R, kernel_2_R, kernel_R);
  } else {
    direct_conv(kernel_2_L, kernel_1_L, kernel_L);
    direct_conv(kernel_2_R, kernel_1_R, kernel_R);
  }

  std::vector<float> buffer(kernel_L.size() * 2U);  // 2 channels interleaved

  for (size_t n = 0U; n < kernel_L.size(); n++) {
    buffer[2U * n] = kernel_L[n];
    buffer[2U * n + 1U] = kernel_R[n];
  }

  const auto output_file_path = irs_dir / std::filesystem::path{output_file_name + irs_ext};

  auto mode = SFM_WRITE;
  auto format = SF_FORMAT_WAV | SF_FORMAT_PCM_32;
  auto n_channels = 2;
  auto rate = (rate1 > rate2) ? rate1 : rate2;

  auto sndfile = SndfileHandle(output_file_path.string(), mode, format, n_channels, rate);

  sndfile.writef(buffer.data(), static_cast<sf_count_t>(kernel_L.size()));

  util::debug("combined kernel saved: " + output_file_path.string());

  util::idle_add([=] { gtk_spinner_stop(self->spinner); });
}

void on_combine_kernels(ConvolverMenuCombine* self, GtkButton* btn) {
  if (g_list_model_get_n_items(G_LIST_MODEL(self->string_list_1)) == 0 ||
      g_list_model_get_n_items(G_LIST_MODEL(self->string_list_2)) == 0) {
    return;
  }

  auto* dropdown_1_selection = gtk_drop_down_get_selected_item(self->dropdown_kernel_1);
  auto* dropdown_2_selection = gtk_drop_down_get_selected_item(self->dropdown_kernel_2);

  if (dropdown_1_selection == nullptr || dropdown_2_selection == nullptr) {
    return;
  }

  gtk_spinner_start(self->spinner);

  const auto* const kernel_1_name = gtk_string_object_get_string(GTK_STRING_OBJECT(dropdown_1_selection));

  const auto* const kernel_2_name = gtk_string_object_get_string(GTK_STRING_OBJECT(dropdown_2_selection));

  std::string output_name = gtk_editable_get_text(GTK_EDITABLE(self->output_kernel_name));

  if (output_name.empty() || output_name.find_first_of("\\/") != std::string::npos) {
    util::debug(" combined IR filename is empty or has illegal characters.");

    gtk_widget_add_css_class(GTK_WIDGET(self->output_kernel_name), "error");

    gtk_widget_grab_focus(GTK_WIDGET(self->output_kernel_name));

    gtk_spinner_stop(self->spinner);
  } else {
    // Truncate filename if longer than 100 characters

    if (output_name.size() > 100U) {
      output_name.resize(100U);
    }

    gtk_widget_remove_css_class(GTK_WIDGET(self->output_kernel_name), "error");

    /*
      The current code convolving the impulse responses is doing direct convolution. It can be very slow depending on
      the size of each kernel. So we do not want to do it in the main thread.
    */

    self->data->mythreads.emplace_back(  // Using emplace_back here makes sense
        [=]() { combine_kernels(self, kernel_1_name, kernel_2_name, output_name); });
  }
}

void dispose(GObject* object) {
  auto* self = EE_CONVOLVER_MENU_COMBINE(object);

  for (auto& t : self->data->mythreads) {
    t.join();
  }

  self->data->mythreads.clear();

  g_object_unref(self->app_settings);

  util::debug("disposed");

  G_OBJECT_CLASS(convolver_menu_combine_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_CONVOLVER_MENU_COMBINE(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(convolver_menu_combine_parent_class)->finalize(object);
}

void convolver_menu_combine_class_init(ConvolverMenuCombineClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::convolver_menu_combine_ui);

  gtk_widget_class_bind_template_child(widget_class, ConvolverMenuCombine, string_list_1);
  gtk_widget_class_bind_template_child(widget_class, ConvolverMenuCombine, string_list_2);
  gtk_widget_class_bind_template_child(widget_class, ConvolverMenuCombine, dropdown_kernel_1);
  gtk_widget_class_bind_template_child(widget_class, ConvolverMenuCombine, dropdown_kernel_2);
  gtk_widget_class_bind_template_child(widget_class, ConvolverMenuCombine, output_kernel_name);
  gtk_widget_class_bind_template_child(widget_class, ConvolverMenuCombine, spinner);

  gtk_widget_class_bind_template_callback(widget_class, on_combine_kernels);
}

void convolver_menu_combine_init(ConvolverMenuCombine* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  for (const auto& name : util::get_files_name(irs_dir, irs_ext)) {
    gtk_string_list_append(self->string_list_1, name.c_str());
    gtk_string_list_append(self->string_list_2, name.c_str());
  }

  self->app_settings = g_settings_new(tags::app::id);

  g_settings_bind(self->app_settings, "autohide-popovers", self, "autohide", G_SETTINGS_BIND_DEFAULT);
}

auto create() -> ConvolverMenuCombine* {
  return static_cast<ConvolverMenuCombine*>(g_object_new(EE_TYPE_CONVOLVER_MENU_COMBINE, nullptr));
}

}  // namespace ui::convolver_menu_combine
