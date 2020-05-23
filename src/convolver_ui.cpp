#include "convolver_ui.hpp"
#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gst/fft/gstfftf32.h>
#include <boost/math/interpolators/cardinal_cubic_b_spline.hpp>
#include <sndfile.hh>
#include "sigc++/functors/ptr_fun.h"

ConvolverUi::ConvolverUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& settings_name)
    : Gtk::Grid(cobject),
      PluginUiBase(builder, settings_name),
      irs_dir(Glib::get_user_config_dir() + "/PulseEffects/irs"),
      spectrum_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.spectrum")) {
  name = "convolver";

  // loading glade widgets

  builder->get_widget("irs_listbox", irs_listbox);
  builder->get_widget("irs_menu_button", irs_menu_button);
  builder->get_widget("irs_scrolled_window", irs_scrolled_window);
  builder->get_widget("import_irs", import_irs);
  builder->get_widget("left_plot", left_plot);
  builder->get_widget("right_plot", right_plot);
  builder->get_widget("file_name", label_file_name);
  builder->get_widget("sampling_rate", label_sampling_rate);
  builder->get_widget("samples", label_samples);
  builder->get_widget("duration", label_duration);
  builder->get_widget("show_fft", show_fft);
  builder->get_widget("plugin_reset", reset_button);

  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);
  get_object(builder, "ir_width", ir_width);

  font.set_family("Monospace");
  font.set_weight(Pango::WEIGHT_BOLD);

  // drawing area callbacks

  left_plot->signal_draw().connect(sigc::mem_fun(*this, &ConvolverUi::on_left_draw));
  left_plot->signal_motion_notify_event().connect(sigc::mem_fun(*this, &ConvolverUi::on_left_motion_notify_event));
  left_plot->signal_enter_notify_event().connect(sigc::mem_fun(*this, &ConvolverUi::on_mouse_enter_notify_event));
  left_plot->signal_leave_notify_event().connect(sigc::mem_fun(*this, &ConvolverUi::on_mouse_leave_notify_event));

  right_plot->signal_draw().connect(sigc::mem_fun(*this, &ConvolverUi::on_right_draw));
  right_plot->signal_motion_notify_event().connect(sigc::mem_fun(*this, &ConvolverUi::on_right_motion_notify_event));
  right_plot->signal_enter_notify_event().connect(sigc::mem_fun(*this, &ConvolverUi::on_mouse_enter_notify_event));
  right_plot->signal_leave_notify_event().connect(sigc::mem_fun(*this, &ConvolverUi::on_mouse_leave_notify_event));

  // impulse response import and selection callbacks

  irs_menu_button->signal_clicked().connect(sigc::mem_fun(*this, &ConvolverUi::on_irs_menu_button_clicked));

  irs_listbox->set_sort_func(sigc::ptr_fun(&ConvolverUi::on_listbox_sort));

  import_irs->signal_clicked().connect(sigc::mem_fun(*this, &ConvolverUi::on_import_irs_clicked));

  // show fft toggle button callback

  show_fft->signal_toggled().connect([=]() {
    show_fft_spectrum = show_fft->get_active();
    left_plot->queue_draw();
    right_plot->queue_draw();
  });

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);
  settings->bind("ir-width", ir_width.get(), "value", flag);

  settings->set_boolean("post-messages", true);

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });

  // irs dir

  auto dir_exists = boost::filesystem::is_directory(irs_dir);

  if (!dir_exists) {
    if (boost::filesystem::create_directories(irs_dir)) {
      util::debug(log_tag + "irs directory created: " + irs_dir.string());
    } else {
      util::warning(log_tag + "failed to create irs directory: " + irs_dir.string());
    }
  } else {
    util::debug(log_tag + "irs directory already exists: " + irs_dir.string());
  }

  // reading current configured irs file

  auto f = [=]() {
    std::lock_guard<std::mutex> lock(lock_guard_irs_info);
    get_irs_info();
  };

  auto future = std::async(std::launch::async, f);

  futures.push_back(std::move(future));

  /* this is necessary to update the interface with the irs info when a preset
     is loaded
  */

  connections.push_back(settings->signal_changed("kernel-path").connect([=](auto key) {
    auto f = [=]() {
      std::lock_guard<std::mutex> lock(lock_guard_irs_info);
      get_irs_info();
    };

    auto future = std::async(std::launch::async, f);

    futures.push_back(std::move(future));
  }));
}

ConvolverUi::~ConvolverUi() {
  futures.clear();

  util::debug(name + " ui destroyed");
}

void ConvolverUi::reset() {
  try {
    std::string section = (preset_type == PresetType::output) ? "output" : "input";

    update_default_key<bool>(settings, "state", section + ".convolver.state");

    update_default_key<double>(settings, "input-gain", section + ".convolver.input-gain");

    update_default_key<double>(settings, "output-gain", section + ".convolver.output-gain");

    update_default_string_key(settings, "kernel-path", section + ".convolver.kernel-path");

    update_default_key<int>(settings, "ir-width", section + ".convolver.ir-width");

    util::debug(name + " plugin: successfully reset");
  } catch (std::exception& e) {
    util::debug(name + " plugin: an error occurred during reset process");
  }
}

auto ConvolverUi::get_irs_names() -> std::vector<std::string> {
  boost::filesystem::directory_iterator it{irs_dir};
  std::vector<std::string> names;

  while (it != boost::filesystem::directory_iterator{}) {
    if (boost::filesystem::is_regular_file(it->status())) {
      if (it->path().extension().string() == ".irs") {
        names.push_back(it->path().stem().string());
      }
    }

    it++;
  }

  return names;
}

void ConvolverUi::import_irs_file(const std::string& file_path) {
  boost::filesystem::path p{file_path};

  if (boost::filesystem::is_regular_file(p)) {
    SndfileHandle file = SndfileHandle(file_path);

    if (file.channels() != 2 || file.frames() == 0) {
      util::warning(log_tag + " Only stereo impulse files are supported!");
      util::warning(log_tag + file_path + " loading failed");

      return;
    }

    auto out_path = irs_dir / p.filename();

    out_path.replace_extension(".irs");

    boost::filesystem::copy_file(p, out_path, boost::filesystem::copy_option::overwrite_if_exists);

    util::debug(log_tag + "imported irs file to: " + out_path.string());
  } else {
    util::warning(log_tag + p.string() + " is not a file!");
  }
}

void ConvolverUi::remove_irs_file(const std::string& name) {
  auto irs_file = irs_dir / boost::filesystem::path{name + ".irs"};

  if (boost::filesystem::exists(irs_file)) {
    boost::filesystem::remove(irs_file);

    util::debug(log_tag + "removed irs file: " + irs_file.string());
  }
}

auto ConvolverUi::on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) -> int {
  auto name1 = row1->get_name();
  auto name2 = row2->get_name();

  std::vector<std::string> names = {name1, name2};

  std::sort(names.begin(), names.end());

  if (name1 == names[0]) {
    return -1;
  }

  if (name2 == names[0]) {
    return 1;
  }

  return 0;
}

void ConvolverUi::populate_irs_listbox() {
  auto children = irs_listbox->get_children();

  for (auto c : children) {
    irs_listbox->remove(*c);
  }

  auto names = get_irs_names();

  for (const auto& name : names) {
    auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/irs_row.glade");

    Gtk::ListBoxRow* row;
    Gtk::Button* remove_btn;
    Gtk::Button* apply_btn;
    Gtk::Label* label;

    b->get_widget("irs_row", row);
    b->get_widget("remove", remove_btn);
    b->get_widget("apply", apply_btn);
    b->get_widget("name", label);

    row->set_name(name);
    label->set_text(name);

    connections.push_back(remove_btn->signal_clicked().connect([=]() {
      remove_irs_file(name);
      populate_irs_listbox();
    }));

    connections.push_back(apply_btn->signal_clicked().connect([=]() {
      auto irs_file = irs_dir / boost::filesystem::path{row->get_name() + ".irs"};

      settings->set_string("kernel-path", irs_file.string());
    }));

    irs_listbox->add(*row);
    irs_listbox->show_all();
  }
}

void ConvolverUi::on_irs_menu_button_clicked() {
  const float scaling_factor = 0.7F;

  int height = static_cast<int>(scaling_factor * static_cast<float>(this->get_toplevel()->get_allocated_height()));

  irs_scrolled_window->set_max_content_height(height);

  populate_irs_listbox();
}

void ConvolverUi::on_import_irs_clicked() {
  // gtkmm 3.22 does not have FileChooseNative so we have to use C api :-(

  gint res;

  auto dialog = gtk_file_chooser_native_new(_("Import Impulse File"), (GtkWindow*)this->get_toplevel()->gobj(),
                                            GTK_FILE_CHOOSER_ACTION_OPEN, _("Open"), _("Cancel"));

  auto filter = gtk_file_filter_new();

  gtk_file_filter_set_name(filter, _("Impulse Response"));
  gtk_file_filter_add_pattern(filter, "*.irs");
  gtk_file_filter_add_pattern(filter, "*.wav");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

  res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(dialog));

  if (res == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser* chooser = GTK_FILE_CHOOSER(dialog);

    auto file_list = gtk_file_chooser_get_filenames(chooser);

    g_slist_foreach(
        file_list,
        [](auto data, auto user_data) {
          auto cui = static_cast<ConvolverUi*>(user_data);

          auto file_path = static_cast<char*>(data);

          cui->import_irs_file(file_path);
        },
        this);

    g_slist_free(file_list);
  }

  g_object_unref(dialog);

  populate_irs_listbox();
}

void ConvolverUi::get_irs_info() {
  auto path = settings->get_string("kernel-path");

  SndfileHandle file = SndfileHandle(path);

  if (file.channels() != 2 || file.frames() == 0) {
    // warning user that there is a problem

    Glib::signal_idle().connect_once([=]() {
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
  float duration = (static_cast<float>(frames_in) - 1) * dt;
  uint max_points = (frames_in > max_plot_points) ? max_plot_points : frames_in;
  float plot_dt = duration / max_points;

  time_axis.resize(max_points);
  time_axis.shrink_to_fit();

  for (uint n = 0; n < max_points; n++) {
    time_axis[n] = n * plot_dt;
  }

  max_time = *std::max_element(time_axis.begin(), time_axis.end());

  // deinterleaving channels

  left_mag.resize(frames_in);
  right_mag.resize(frames_in);

  // ensure that the fft can be computed
  if (left_mag.size() % 2 != 0)
    left_mag.push_back(0);
  if (right_mag.size() % 2 != 0)
    right_mag.push_back(0);

  left_mag.shrink_to_fit();
  right_mag.shrink_to_fit();

  for (uint n = 0; n < frames_in; n++) {
    left_mag[n] = kernel[2 * n];
    right_mag[n] = kernel[2 * n + 1];
  }

  get_irs_spectrum(rate);

  /*interpolating because we can not plot all the data in the irs file. It
    would be too slow
  */

  try {
    boost::math::interpolators::cardinal_cubic_b_spline<float> spline_L(left_mag.begin(), left_mag.end(), 0.0F, dt);

    boost::math::interpolators::cardinal_cubic_b_spline<float> spline_R(right_mag.begin(), right_mag.end(), 0.0F, dt);

    left_mag.resize(max_points);
    right_mag.resize(max_points);

    left_mag.shrink_to_fit();
    right_mag.shrink_to_fit();

    for (uint n = 0; n < max_points; n++) {
      left_mag[n] = spline_L(time_axis[n]);
      right_mag[n] = spline_R(time_axis[n]);
    }
  } catch (const std::exception& e) {
    util::debug(std::string("Message from thrown exception was: ") + e.what());
  }

  // find min and max values

  min_left = *std::min_element(left_mag.begin(), left_mag.end());
  max_left = *std::max_element(left_mag.begin(), left_mag.end());
  min_right = *std::min_element(right_mag.begin(), right_mag.end());
  max_right = *std::max_element(right_mag.begin(), right_mag.end());

  // rescaling between 0 and 1

  for (uint n = 0; n < max_points; n++) {
    left_mag[n] = (left_mag[n] - min_left) / (max_left - min_left);
    right_mag[n] = (right_mag[n] - min_right) / (max_right - min_right);
  }

  // updating interface with ir file info

  Glib::signal_idle().connect_once([=]() {
    label_sampling_rate->set_text(std::to_string(rate) + " Hz");
    label_samples->set_text(std::to_string(frames_in));

    std::ostringstream msg;

    msg.precision(3);
    msg << duration << " s";

    label_duration->set_text(msg.str());

    auto fpath = boost::filesystem::path{path};

    label_file_name->set_text(fpath.stem().string());

    left_plot->queue_draw();
    right_plot->queue_draw();
  });
}

void ConvolverUi::get_irs_spectrum(const int& rate) {
  int nfft = left_mag.size();  // right_mag.size() should have the same value

  GstFFTF32* fft_ctx = gst_fft_f32_new(nfft, 0);
  auto* freqdata_l = g_new0(GstFFTF32Complex, nfft / 2 + 1);
  auto* freqdata_r = g_new0(GstFFTF32Complex, nfft / 2 + 1);

  std::vector<float> tmp_l;
  std::vector<float> tmp_r;

  tmp_l.resize(nfft);
  tmp_r.resize(nfft);

  tmp_l.shrink_to_fit();
  tmp_r.shrink_to_fit();

  std::copy(left_mag.begin(), left_mag.end(), tmp_l.begin());
  std::copy(right_mag.begin(), right_mag.end(), tmp_r.begin());

  gst_fft_f32_fft(fft_ctx, tmp_l.data(), freqdata_l);
  gst_fft_f32_fft(fft_ctx, tmp_r.data(), freqdata_r);

  left_spectrum.resize(nfft / 2 + 1);
  right_spectrum.resize(nfft / 2 + 1);

  for (int i = 0; i < nfft / 2 + 1; i++) {
    float v_l;
    float v_r;

    // left
    v_l = freqdata_l[i].r * freqdata_l[i].r;
    v_l += freqdata_l[i].i * freqdata_l[i].i;
    v_l = std::sqrt(v_l);
    v_l /= static_cast<float>(nfft * nfft);

    left_spectrum[i] = v_l;

    // right
    v_r = freqdata_r[i].r * freqdata_r[i].r;
    v_r += freqdata_r[i].i * freqdata_r[i].i;
    v_r = std::sqrt(v_r);
    v_r /= static_cast<float>(nfft * nfft);

    right_spectrum[i] = v_r;
  }

  uint max_points = std::min((uint)left_spectrum.size(), max_plot_points);

  fft_min_freq = static_cast<float>(rate) * (0.5F * 0 + 0.25F) / left_spectrum.size();
  fft_max_freq = static_cast<float>(rate) * (0.5F * (left_spectrum.size() - 1.0F) + 0.25F) / left_spectrum.size();

  freq_axis = util::logspace(log10(fft_min_freq), log10(fft_max_freq), max_points);

  /*interpolating because we can not plot all the data in the irs file. It
    would be too slow
  */

  try {
    float dF = 0.5F * static_cast<float>(rate) / left_spectrum.size();

    boost::math::interpolators::cardinal_cubic_b_spline<float> spline_L(left_spectrum.begin(), left_spectrum.end(),
                                                                        0.0F, dF);

    boost::math::interpolators::cardinal_cubic_b_spline<float> spline_R(right_spectrum.begin(), right_spectrum.end(),
                                                                        0.0F, dF);

    left_spectrum.resize(max_points);
    right_spectrum.resize(max_points);

    left_spectrum.shrink_to_fit();
    right_spectrum.shrink_to_fit();

    for (uint n = 0; n < max_points; n++) {
      left_spectrum[n] = spline_L(freq_axis[n]);
      right_spectrum[n] = spline_R(freq_axis[n]);
    }
  } catch (const std::exception& e) {
    util::debug(std::string("Message from thrown exception was: ") + e.what());
  }

  // find min and max values

  fft_min_left = *std::min_element(left_spectrum.begin(), left_spectrum.end());
  fft_max_left = *std::max_element(left_spectrum.begin(), left_spectrum.end());
  fft_min_right = *std::min_element(right_spectrum.begin(), right_spectrum.end());
  fft_max_right = *std::max_element(right_spectrum.begin(), right_spectrum.end());

  // rescaling between 0 and 1

  for (unsigned int n = 0; n < left_spectrum.size(); n++) {
    left_spectrum[n] = (left_spectrum[n] - fft_min_left) / (fft_max_left - fft_min_left);
    right_spectrum[n] = (right_spectrum[n] - fft_min_right) / (fft_max_right - fft_min_right);
  }

  gst_fft_f32_free(fft_ctx);
  g_free(freqdata_l);
  g_free(freqdata_r);
}

void ConvolverUi::draw_channel(Gtk::DrawingArea* da,
                               const Cairo::RefPtr<Cairo::Context>& ctx,
                               const std::vector<float>& magnitudes) {
  auto n_bars = magnitudes.size();

  if (n_bars > 0) {
    auto allocation = da->get_allocation();
    auto width = allocation.get_width();
    auto height = allocation.get_height();
    auto n_bars = magnitudes.size();
    auto x = util::linspace(0, width, n_bars);

    for (uint n = 0; n < n_bars - 1; n++) {
      auto bar_height = magnitudes[n] * height;

      ctx->move_to(x[n], height - bar_height);

      bar_height = magnitudes[n + 1] * height;

      ctx->line_to(x[n + 1], height - bar_height);
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

    if (mouse_inside) {
      std::ostringstream msg;

      if (show_fft_spectrum) {
        msg.precision(0);
        msg << std::fixed << mouse_freq << " Hz, ";
      } else {
        msg.precision(3);
        msg << std::fixed << mouse_time << " s, ";
      }

      msg.precision(3);
      msg << std::fixed << mouse_intensity;

      int text_width;
      int text_height;
      auto layout = create_pango_layout(msg.str());
      layout->set_font_description(font);
      layout->get_pixel_size(text_width, text_height);

      ctx->move_to(width - text_width, 0);

      layout->show_in_cairo_context(ctx);
    }
  }
}

void ConvolverUi::update_mouse_info_L(GdkEventMotion* event) {
  auto allocation = left_plot->get_allocation();

  auto width = allocation.get_width();
  auto height = allocation.get_height();

  if (show_fft_spectrum) {
    mouse_freq = static_cast<float>(event->x) * fft_max_freq / width;

    mouse_intensity = (height - static_cast<float>(event->y)) / height * (fft_max_left - fft_min_left) + fft_min_left;
  } else {
    mouse_time = static_cast<float>(event->x) * max_time / width;

    mouse_intensity = (height - static_cast<float>(event->y)) / height * (max_left - min_left) + min_left;
  }
}

void ConvolverUi::update_mouse_info_R(GdkEventMotion* event) {
  auto allocation = left_plot->get_allocation();

  auto width = allocation.get_width();
  auto height = allocation.get_height();

  if (show_fft_spectrum) {
    mouse_freq = static_cast<float>(event->x) * fft_max_freq / width;

    mouse_intensity = (height - static_cast<float>(event->y)) / height * (fft_max_right - fft_min_right) + fft_min_right;
  } else {
    mouse_time = static_cast<float>(event->x) * max_time / width;

    mouse_intensity = static_cast<float>(event->y) / height * (max_right - min_right) + min_right;
  }
}

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

auto ConvolverUi::on_left_motion_notify_event(GdkEventMotion* event) -> bool {
  update_mouse_info_L(event);

  left_plot->queue_draw();

  return false;
}

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

auto ConvolverUi::on_right_motion_notify_event(GdkEventMotion* event) -> bool {
  update_mouse_info_R(event);

  right_plot->queue_draw();

  return false;
}

auto ConvolverUi::on_mouse_enter_notify_event(GdkEventCrossing* event) -> bool {
  mouse_inside = true;
  return false;
}

auto ConvolverUi::on_mouse_leave_notify_event(GdkEventCrossing* event) -> bool {
  mouse_inside = false;
  return false;
}
