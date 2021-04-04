/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "convolver_ui.hpp"

ConvolverUi::ConvolverUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& schema,
                         const std::string& schema_path)
    : Gtk::Box(cobject),
      PluginUiBase(builder, schema, schema_path),
      irs_dir(Glib::get_user_config_dir() + "/PulseEffects/irs"),
      spectrum_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.spectrum")) {
  name = plugin_name::convolver;

  // loading builder widgets

  input_gain = builder->get_widget<Gtk::Scale>("input_gain");
  output_gain = builder->get_widget<Gtk::Scale>("output_gain");

  ir_width = builder->get_widget<Gtk::SpinButton>("ir_width");

  listview = builder->get_widget<Gtk::ListView>("listview");

  scrolled_window = builder->get_widget<Gtk::ScrolledWindow>("scrolled_window");

  import = builder->get_widget<Gtk::Button>("import");

  show_fft = builder->get_widget<Gtk::ToggleButton>("show_fft");

  label_sampling_rate = builder->get_widget<Gtk::Label>("label_sampling_rate");
  label_samples = builder->get_widget<Gtk::Label>("label_samples");
  label_duration = builder->get_widget<Gtk::Label>("label_duration");
  label_file_name = builder->get_widget<Gtk::Label>("label_file_name");

  // builder->get_widget("irs_menu_button", irs_menu_button);
  // builder->get_widget("left_plot", left_plot);
  // builder->get_widget("right_plot", right_plot);

  // font.set_family("Monospace");
  // font.set_weight(Pango::WEIGHT_BOLD);

  // drawing area callbacks

  // left_plot->signal_draw().connect(sigc::mem_fun(*this, &ConvolverUi::on_left_draw));
  // left_plot->signal_motion_notify_event().connect(sigc::mem_fun(*this, &ConvolverUi::on_left_motion_notify_event));
  // left_plot->signal_enter_notify_event().connect(sigc::mem_fun(*this, &ConvolverUi::on_mouse_enter_notify_event));
  // left_plot->signal_leave_notify_event().connect(sigc::mem_fun(*this, &ConvolverUi::on_mouse_leave_notify_event));

  // right_plot->signal_draw().connect(sigc::mem_fun(*this, &ConvolverUi::on_right_draw));
  // right_plot->signal_motion_notify_event().connect(sigc::mem_fun(*this, &ConvolverUi::on_right_motion_notify_event));
  // right_plot->signal_enter_notify_event().connect(sigc::mem_fun(*this, &ConvolverUi::on_mouse_enter_notify_event));
  // right_plot->signal_leave_notify_event().connect(sigc::mem_fun(*this, &ConvolverUi::on_mouse_leave_notify_event));

  // impulse response import and selection callbacks

  // irs_menu_button->signal_clicked().connect(sigc::mem_fun(*this, &ConvolverUi::on_irs_menu_button_clicked));

  import->signal_clicked().connect(sigc::mem_fun(*this, &ConvolverUi::on_import_irs_clicked));

  // show fft toggle button callback

  show_fft->signal_toggled().connect([=, this]() {
    show_fft_spectrum = show_fft->get_active();
    //   left_plot->queue_draw();
    //   right_plot->queue_draw();
  });

  // gsettings bindings

  settings->bind("input-gain", input_gain->get_adjustment().get(), "value");
  settings->bind("output-gain", output_gain->get_adjustment().get(), "value");
  settings->bind("ir-width", ir_width->get_adjustment().get(), "value");

  // irs dir

  auto dir_exists = std::filesystem::is_directory(irs_dir);

  if (!dir_exists) {
    if (std::filesystem::create_directories(irs_dir)) {
      util::debug(log_tag + "irs directory created: " + irs_dir.string());
    } else {
      util::warning(log_tag + "failed to create irs directory: " + irs_dir.string());
    }
  } else {
    util::debug(log_tag + "irs directory already exists: " + irs_dir.string());
  }

  // reading current configured irs file

  auto f = [=, this]() {
    std::lock_guard<std::mutex> lock(lock_guard_irs_info);

    get_irs_info();
  };

  auto future = std::async(std::launch::async, f);

  futures.emplace_back(std::move(future));

  /* this is necessary to update the interface with the irs info when a preset
     is loaded
  */

  connections.emplace_back(settings->signal_changed("kernel-path").connect([=, this](auto key) {
    auto f = [=, this]() {
      std::lock_guard<std::mutex> lock(lock_guard_irs_info);

      get_irs_info();
    };

    auto future = std::async(std::launch::async, f);

    futures.emplace_back(std::move(future));
  }));
}

ConvolverUi::~ConvolverUi() {
  futures.clear();

  util::debug(name + " ui destroyed");
}

auto ConvolverUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> ConvolverUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/convolver.ui");

  auto* ui = Gtk::Builder::get_widget_derived<ConvolverUi>(builder, "top_box", "com.github.wwmm.pulseeffects.convolver",
                                                           schema_path + "convolver/");

  auto stack_page = stack->add(*ui, plugin_name::convolver);

  return ui;
}

void ConvolverUi::reset() {
  settings->reset("state");

  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("kernel-path");

  settings->reset("ir-width");
}

auto ConvolverUi::get_irs_names() -> std::vector<std::string> {
  std::filesystem::directory_iterator it{irs_dir};
  std::vector<std::string> names;

  while (it != std::filesystem::directory_iterator{}) {
    if (std::filesystem::is_regular_file(it->status())) {
      if (it->path().extension().string() == ".irs") {
        names.emplace_back(it->path().stem().string());
      }
    }

    it++;
  }

  return names;
}

void ConvolverUi::import_irs_file(const std::string& file_path) {
  std::filesystem::path p{file_path};

  if (std::filesystem::is_regular_file(p)) {
    SndfileHandle file = SndfileHandle(file_path);

    if (file.channels() != 2 || file.frames() == 0) {
      util::warning(log_tag + " Only stereo impulse files are supported!");
      util::warning(log_tag + file_path + " loading failed");

      return;
    }

    auto out_path = irs_dir / p.filename();

    out_path.replace_extension(".irs");

    std::filesystem::copy_file(p, out_path, std::filesystem::copy_options::overwrite_existing);

    util::debug(log_tag + "imported irs file to: " + out_path.string());
  } else {
    util::warning(log_tag + p.string() + " is not a file!");
  }
}

void ConvolverUi::remove_irs_file(const std::string& name) {
  auto irs_file = irs_dir / std::filesystem::path{name + ".irs"};

  if (std::filesystem::exists(irs_file)) {
    std::filesystem::remove(irs_file);

    util::debug(log_tag + "removed irs file: " + irs_file.string());
  }
}

// void ConvolverUi::populate_irs_listbox() {
//   auto children = irs_listbox->get_children();

//   for (const auto& c : children) {
//     irs_listbox->remove(*c);
//   }

//   auto names = get_irs_names();

//   for (const auto& name : names) {
//     auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/irs_row.glade");

//     Gtk::ListBoxRow* row = nullptr;
//     Gtk::Button* remove_btn = nullptr;
//     Gtk::Button* apply_btn = nullptr;
//     Gtk::Label* label = nullptr;

//     b->get_widget("irs_row", row);
//     b->get_widget("remove", remove_btn);
//     b->get_widget("apply", apply_btn);
//     b->get_widget("name", label);

//     row->set_name(name);
//     label->set_text(name);

//     connections.emplace_back(remove_btn->signal_clicked().connect([=, this]() {
//       remove_irs_file(name);
//       populate_irs_listbox();
//     }));

//     connections.emplace_back(apply_btn->signal_clicked().connect([=, this]() {
//       auto irs_file = irs_dir / std::filesystem::path{row->get_name() + ".irs"};

//       settings->set_string("kernel-path", irs_file.string());
//     }));

//     irs_listbox->add(*row);
//     irs_listbox->show_all();
//   }
// }

void ConvolverUi::on_irs_menu_button_clicked() {
  // const float scaling_factor = 0.7F;

  // int height = static_cast<int>(scaling_factor * static_cast<float>(this->get_toplevel()->get_allocated_height()));

  // irs_scrolled_window->set_max_content_height(height);

  // populate_irs_listbox();
}

void ConvolverUi::on_import_irs_clicked() {
  Glib::RefPtr<Gtk::FileChooserNative> dialog;

  if (transient_window != nullptr) {
    dialog = Gtk::FileChooserNative::create(_("Import Impulse File"), *transient_window, Gtk::FileChooser::Action::OPEN,
                                            _("Open"), _("Cancel"));
  } else {
    dialog = Gtk::FileChooserNative::create(_("Import Impulse File"), Gtk::FileChooser::Action::OPEN, _("Open"),
                                            _("Cancel"));
  }

  auto dialog_filter = Gtk::FileFilter::create();

  dialog_filter->set_name(_("Impulse Response"));
  dialog_filter->add_pattern("*.irs");
  dialog_filter->add_pattern("*.wav");

  dialog->add_filter(dialog_filter);

  dialog->signal_response().connect([=, this](auto response_id) {
    switch (response_id) {
      case Gtk::ResponseType::ACCEPT: {
        import_irs_file(dialog->get_file()->get_path());

        // populate_irs_listbox();

        break;
      }
      default:
        break;
    }
  });

  dialog->set_modal(true);
  dialog->show();
}

void ConvolverUi::get_irs_info() {
  auto path = settings->get_string("kernel-path");

  SndfileHandle file = SndfileHandle(path);

  if (file.channels() != 2 || file.frames() == 0) {
    // warning user that there is a problem

    Glib::signal_idle().connect_once([=, this]() {
      label_sampling_rate->set_text(_("Failed"));
      label_samples->set_text(_("Failed"));

      label_duration->set_text(_("Failed"));

      label_file_name->set_text(_("Could Not Load The Impulse File"));
    });

    return;
  }

  uint frames_in = file.frames();
  uint total_frames_in = file.channels() * frames_in;
  uint rate = file.samplerate();

  std::vector<float> kernel(total_frames_in);

  file.readf(kernel.data(), frames_in);

  // build plot time axis

  float dt = 1.0F / rate;
  float duration = (static_cast<float>(frames_in) - 1.0F) * dt;
  uint max_points = (frames_in > max_plot_points) ? max_plot_points : frames_in;
  float plot_dt = duration / max_points;

  time_axis.resize(max_points);
  time_axis.shrink_to_fit();

  for (uint n = 0U; n < max_points; n++) {
    time_axis[n] = n * plot_dt;
  }

  max_time = *std::max_element(time_axis.begin(), time_axis.end());

  // deinterleaving channels

  left_mag.resize(frames_in);
  right_mag.resize(frames_in);

  // ensure that the fft can be computed

  if (left_mag.size() % 2U != 0U) {
    left_mag.emplace_back(0.0F);
  }

  if (right_mag.size() % 2U != 0U) {
    right_mag.emplace_back(0.0F);
  }

  left_mag.shrink_to_fit();
  right_mag.shrink_to_fit();

  for (uint n = 0U; n < frames_in; n++) {
    left_mag[n] = kernel[2U * n];
    right_mag[n] = kernel[2U * n + 1U];
  }

  get_irs_spectrum(rate);

  /*interpolating because we can not plot all the data in the irs file. It
    would be too slow
  */

  // try {
  //   boost::math::interpolators::cardinal_cubic_b_spline<float> spline_L(left_mag.begin(), left_mag.end(), 0.0F, dt);

  //   boost::math::interpolators::cardinal_cubic_b_spline<float> spline_R(right_mag.begin(), right_mag.end(), 0.0F,
  //   dt);

  //   left_mag.resize(max_points);
  //   right_mag.resize(max_points);

  //   left_mag.shrink_to_fit();
  //   right_mag.shrink_to_fit();

  //   for (uint n = 0U; n < max_points; n++) {
  //     left_mag[n] = spline_L(time_axis[n]);
  //     right_mag[n] = spline_R(time_axis[n]);
  //   }
  // } catch (const std::exception& e) {
  //   util::debug(std::string("Message from thrown exception was: ") + e.what());
  // }

  // find min and max values

  min_left = *std::min_element(left_mag.begin(), left_mag.end());
  max_left = *std::max_element(left_mag.begin(), left_mag.end());
  min_right = *std::min_element(right_mag.begin(), right_mag.end());
  max_right = *std::max_element(right_mag.begin(), right_mag.end());

  // rescaling between 0 and 1

  for (uint n = 0U; n < max_points; n++) {
    left_mag[n] = (left_mag[n] - min_left) / (max_left - min_left);
    right_mag[n] = (right_mag[n] - min_right) / (max_right - min_right);
  }

  // updating interface with ir file info

  // Glib::signal_idle().connect_once([=, this]() {
  //   label_sampling_rate->set_text(std::to_string(rate) + " Hz");
  //   label_samples->set_text(std::to_string(frames_in));

  //   label_duration->set_text(level_to_localized_string(duration, 3) + " s");

  //   auto fpath = std::filesystem::path{path};

  //   label_file_name->set_text(fpath.stem().string());

  //   left_plot->queue_draw();
  //   right_plot->queue_draw();
  // });
}

void ConvolverUi::get_irs_spectrum(const int& rate) {
  // uint nfft = left_mag.size();  // right_mag.size() should have the same value

  // GstFFTF32* fft_ctx = gst_fft_f32_new(nfft, 0);
  // auto* freqdata_l = g_new0(GstFFTF32Complex, nfft / 2U + 1U);
  // auto* freqdata_r = g_new0(GstFFTF32Complex, nfft / 2U + 1U);

  // std::vector<float> tmp_l;
  // std::vector<float> tmp_r;

  // tmp_l.resize(nfft);
  // tmp_r.resize(nfft);

  // tmp_l.shrink_to_fit();
  // tmp_r.shrink_to_fit();

  // std::copy(left_mag.begin(), left_mag.end(), tmp_l.begin());
  // std::copy(right_mag.begin(), right_mag.end(), tmp_r.begin());

  // gst_fft_f32_fft(fft_ctx, tmp_l.data(), freqdata_l);
  // gst_fft_f32_fft(fft_ctx, tmp_r.data(), freqdata_r);

  // left_spectrum.resize(nfft / 2U + 1U);
  // right_spectrum.resize(nfft / 2U + 1U);

  // for (uint i = 0U; i < nfft / 2U + 1U; i++) {
  //   float v_l = 0.0F;
  //   float v_r = 0.0F;

  //   // left
  //   v_l = freqdata_l[i].r * freqdata_l[i].r;
  //   v_l += freqdata_l[i].i * freqdata_l[i].i;
  //   v_l = std::sqrt(v_l);

  //   left_spectrum[i] = v_l;

  //   // right
  //   v_r = freqdata_r[i].r * freqdata_r[i].r;
  //   v_r += freqdata_r[i].i * freqdata_r[i].i;
  //   v_r = std::sqrt(v_r);

  //   right_spectrum[i] = v_r;
  // }

  // uint max_points = std::min(static_cast<uint>(left_spectrum.size()), max_plot_points);

  // fft_min_freq = 1.0F;
  // fft_max_freq = 0.5F * static_cast<float>(rate);

  // freq_axis = util::logspace(log10(fft_min_freq), log10(fft_max_freq), max_points);

  // /*interpolating because we can not plot all the data in the irs file. It
  //   would be too slow
  // */

  // try {
  //   float dF = 0.5F * static_cast<float>(rate) / left_spectrum.size();

  //   boost::math::interpolators::cardinal_cubic_b_spline<float> spline_L(left_spectrum.begin(), left_spectrum.end(),
  //                                                                       0.0F, dF);

  //   boost::math::interpolators::cardinal_cubic_b_spline<float> spline_R(right_spectrum.begin(), right_spectrum.end(),
  //                                                                       0.0F, dF);

  //   left_spectrum.resize(max_points);
  //   right_spectrum.resize(max_points);

  //   left_spectrum.shrink_to_fit();
  //   right_spectrum.shrink_to_fit();

  //   for (uint n = 0U; n < max_points; n++) {
  //     left_spectrum[n] = spline_L(freq_axis[n]);
  //     right_spectrum[n] = spline_R(freq_axis[n]);
  //   }
  // } catch (const std::exception& e) {
  //   util::debug(std::string("Message from thrown exception was: ") + e.what());
  // }

  // // find min and max values

  // fft_min_left = *std::min_element(left_spectrum.begin(), left_spectrum.end());
  // fft_max_left = *std::max_element(left_spectrum.begin(), left_spectrum.end());
  // fft_min_right = *std::min_element(right_spectrum.begin(), right_spectrum.end());
  // fft_max_right = *std::max_element(right_spectrum.begin(), right_spectrum.end());

  // // rescaling between 0 and 1

  // for (unsigned int n = 0U; n < left_spectrum.size(); n++) {
  //   left_spectrum[n] = (left_spectrum[n] - fft_min_left) / (fft_max_left - fft_min_left);
  //   right_spectrum[n] = (right_spectrum[n] - fft_min_right) / (fft_max_right - fft_min_right);
  // }

  // gst_fft_f32_free(fft_ctx);
  // g_free(freqdata_l);
  // g_free(freqdata_r);
}

void ConvolverUi::draw_channel(Gtk::DrawingArea* da,
                               const Cairo::RefPtr<Cairo::Context>& ctx,
                               const std::vector<float>& magnitudes) {
  auto n_bars = magnitudes.size();

  if (n_bars > 0U) {
    auto allocation = da->get_allocation();
    auto width = allocation.get_width();
    auto height = allocation.get_height();
    auto n_bars = magnitudes.size();
    auto x = util::linspace(0.0F, static_cast<float>(width), n_bars);

    for (uint n = 0U; n < n_bars - 1U; n++) {
      auto bar_height = magnitudes[n] * height;

      ctx->move_to(x[n], height - bar_height);

      bar_height = magnitudes[n + 1U] * height;

      ctx->line_to(x[n + 1U], height - bar_height);
    }

    if (spectrum_settings->get_boolean("use-custom-color")) {
      Glib::Variant<std::vector<double>> v;
      spectrum_settings->get_value("color", v);
      auto rgba = v.get();

      ctx->set_source_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);
    } else {
      auto color = Gdk::RGBA();
      auto style_ctx = da->get_style_context();

      style_ctx->lookup_color("theme_selected_bg_color", color);

      ctx->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), 1.0);
    }

    ctx->set_line_width(2.0);
    ctx->stroke();

    // if (mouse_inside) {
    //   std::string msg;

    //   if (show_fft_spectrum) {
    //     msg.append(level_to_localized_string(mouse_freq, 0) + " Hz, ");
    //     msg.append(level_to_localized_string(mouse_intensity, 3));
    //   } else {
    //     msg.append(level_to_localized_string(mouse_time, 3) + " s, ");
    //     msg.append(level_to_localized_string(mouse_intensity, 3));
    //   }

    //   int text_width = 0;
    //   int text_height = 0;
    //   auto layout = create_pango_layout(msg);
    //   layout->set_font_description(font);
    //   layout->get_pixel_size(text_width, text_height);

    //   ctx->move_to(width - text_width, 0);

    //   layout->show_in_cairo_context(ctx);
    // }
  }
}

// void ConvolverUi::update_mouse_info_L(GdkEventMotion* event) {
//   auto allocation = left_plot->get_allocation();

//   auto width = allocation.get_width();
//   auto height = allocation.get_height();

//   if (show_fft_spectrum) {
//     float fft_min_freq_log = log10(fft_min_freq);
//     float fft_max_freq_log = log10(fft_max_freq);
//     float mouse_freq_log =
//         static_cast<float>(event->x) / width * (fft_max_freq_log - fft_min_freq_log) + fft_min_freq_log;

//     mouse_freq = std::pow(10.0F, mouse_freq_log);  // exp10 does not exist on FreeBSD

//     mouse_intensity = (height - static_cast<float>(event->y)) / height * (fft_max_left - fft_min_left) +
//     fft_min_left;
//   } else {
//     mouse_time = static_cast<float>(event->x) * max_time / width;

//     mouse_intensity = (height - static_cast<float>(event->y)) / height * (max_left - min_left) + min_left;
//   }
// }

// void ConvolverUi::update_mouse_info_R(GdkEventMotion* event) {
//   auto allocation = right_plot->get_allocation();

//   auto width = allocation.get_width();
//   auto height = allocation.get_height();

//   if (show_fft_spectrum) {
//     float fft_min_freq_log = log10(fft_min_freq);
//     float fft_max_freq_log = log10(fft_max_freq);
//     float mouse_freq_log =
//         static_cast<float>(event->x) / width * (fft_max_freq_log - fft_min_freq_log) + fft_min_freq_log;

//     mouse_freq = std::pow(10.0F, mouse_freq_log);  // exp10 does not exist on FreeBSD

//     mouse_intensity =
//         (height - static_cast<float>(event->y)) / height * (fft_max_right - fft_min_right) + fft_min_right;
//   } else {
//     mouse_time = static_cast<float>(event->x) * max_time / width;

//     mouse_intensity = static_cast<float>(event->y) / height * (max_right - min_right) + min_right;
//   }
// }

auto ConvolverUi::on_left_draw(const Cairo::RefPtr<Cairo::Context>& ctx) -> bool {
  std::lock_guard<std::mutex> lock(lock_guard_irs_info);

  ctx->paint();

  if (show_fft_spectrum) {
    draw_channel(left_plot, ctx, left_spectrum);
  } else {
    draw_channel(left_plot, ctx, left_mag);
  }

  return false;
}

// auto ConvolverUi::on_left_motion_notify_event(GdkEventMotion* event) -> bool {
//   update_mouse_info_L(event);

//   left_plot->queue_draw();

//   return false;
// }

auto ConvolverUi::on_right_draw(const Cairo::RefPtr<Cairo::Context>& ctx) -> bool {
  std::lock_guard<std::mutex> lock(lock_guard_irs_info);

  ctx->paint();

  if (show_fft_spectrum) {
    draw_channel(right_plot, ctx, right_spectrum);
  } else {
    draw_channel(right_plot, ctx, right_mag);
  }

  return false;
}

// auto ConvolverUi::on_right_motion_notify_event(GdkEventMotion* event) -> bool {
//   update_mouse_info_R(event);

//   right_plot->queue_draw();

//   return false;
// }

// auto ConvolverUi::on_mouse_enter_notify_event(GdkEventCrossing* event) -> bool {
//   mouse_inside = true;
//   return false;
// }

// auto ConvolverUi::on_mouse_leave_notify_event(GdkEventCrossing* event) -> bool {
//   mouse_inside = false;
//   return false;
// }
