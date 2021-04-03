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

#include "spectrum_ui.hpp"

SpectrumUi::SpectrumUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::DrawingArea(cobject),
      settings(Gio::Settings::create("com.github.wwmm.pulseeffects.spectrum")),
      controller_motion(Gtk::EventControllerMotion::create()) {
  plot = std::make_unique<Plot>(this);

  // set_draw_func(sigc::mem_fun(*this, &SpectrumUi::on_draw));

  // signals connection

  // add_controller(controller_motion);

  // controller_motion->signal_motion().connect([=, this](const double& x, const double& y) {
  //   int width = get_width();
  //   int height = get_height();
  //   int usable_height = height - axis_height;

  //   if (y < usable_height) {
  //     double min_freq_log = log10(static_cast<double>(settings->get_int("minimum-frequency")));
  //     double max_freq_log = log10(static_cast<double>(settings->get_int("maximum-frequency")));
  //     double mouse_freq_log = x / static_cast<double>(width) * (max_freq_log - min_freq_log) + min_freq_log;

  //     mouse_freq = std::pow(10.0, mouse_freq_log);  // exp10 does not exist on FreeBSD

  //     // intensity scale is in decibel

  //     mouse_intensity = y * util::minimum_db_level / usable_height;

  //     queue_draw();
  //   }
  // });

  // connections.emplace_back(settings->signal_changed("use-custom-color").connect([&](auto key) {
  //   init_color();
  //   init_frequency_labels_color();
  //   init_gradient_color();
  // }));

  // connections.emplace_back(settings->signal_changed("color").connect([&](auto key) { init_color(); }));

  // connections.emplace_back(
  //     settings->signal_changed("color-axis-labels").connect([&](auto key) { init_frequency_labels_color(); }));

  // connections.emplace_back(
  //     settings->signal_changed("gradient-color").connect([&](auto key) { init_gradient_color(); }));

  // connections.emplace_back(
  //     settings->signal_changed("height").connect([&](auto key) { set_content_height(settings->get_int("height"));
  //     }));

  // connections.emplace_back(settings->signal_changed("n-points").connect([&](auto key) { init_frequency_axis(); }));

  // connections.emplace_back(
  //     settings->signal_changed("minimum-frequency").connect([&](auto key) { init_frequency_axis(); }));

  // connections.emplace_back(
  //     settings->signal_changed("maximum-frequency").connect([&](auto key) { init_frequency_axis(); }));

  settings->bind("show", this, "visible", Gio::Settings::BindFlags::GET);

  init_color();
  init_frequency_labels_color();
  init_gradient_color();

  set_content_height(settings->get_int("height"));
}

SpectrumUi::~SpectrumUi() {
  for (auto& c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

auto SpectrumUi::add_to_box(Gtk::Box* box) -> SpectrumUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/spectrum.ui");

  auto* ui = Gtk::Builder::get_widget_derived<SpectrumUi>(builder, "drawing_area");

  box->append(*ui);

  return ui;
}

void SpectrumUi::clear_spectrum() {
  spectrum_mag.resize(0);

  queue_draw();
}

void SpectrumUi::on_new_spectrum(const uint& rate, const uint& n_bands, const std::vector<float>& magnitudes) {
  if (!settings->get_boolean("show")) {
    return;
  }

  if (this->rate != rate || this->n_bands != n_bands) {
    this->rate = rate;
    this->n_bands = n_bands;

    init_frequency_axis();
  }

  std::ranges::fill(spectrum_mag, 0.0F);
  std::ranges::fill(spectrum_bin_count, 0);

  // reducing the amount of data so we can plot them

  for (size_t j = 0; j < spectrum_freqs.size(); j++) {
    for (size_t n = 0; n < spectrum_x_axis.size(); n++) {
      if (n > 0) {
        if (spectrum_freqs[j] <= spectrum_x_axis[n] && spectrum_freqs[j] > spectrum_x_axis[n - 1]) {
          spectrum_mag[n] += magnitudes[j];

          spectrum_bin_count[n]++;
        }
      } else {
        if (spectrum_freqs[j] <= spectrum_x_axis[n]) {
          spectrum_mag[n] += magnitudes[j];

          spectrum_bin_count[n]++;
        }
      }
    }
  }

  for (size_t n = 0; n < spectrum_bin_count.size(); n++) {
    if (spectrum_bin_count[n] == 0 && n > 0) {
      spectrum_mag[n] = spectrum_mag[n - 1];
    }
  }

  std::ranges::for_each(spectrum_mag, [](auto& v) {
    v = 10.0F * log10f(v);

    if (!std::isinf(v)) {
      v = (v > util::minimum_db_level) ? v : util::minimum_db_level;
    } else {
      v = util::minimum_db_level;
    }
  });

  plot->set_data(spectrum_x_axis, spectrum_mag);
}

void SpectrumUi::on_draw(const Cairo::RefPtr<Cairo::Context>& ctx, const int& width, const int& height) {
  ctx->paint();

  auto n_points = spectrum_mag.size();

  if (n_points > 0U) {
    auto line_width = static_cast<int>(settings->get_double("line-width"));
    auto objects_x = util::linspace(line_width, static_cast<float>(width) - line_width, n_points);
    auto draw_border = settings->get_boolean("show-bar-border");
    auto use_gradient = settings->get_boolean("use-gradient");
    auto spectrum_type = settings->get_enum("type");

    if (!settings->get_boolean("use-custom-color")) {
      auto style_ctx = get_style_context();

      style_ctx->lookup_color("theme_selected_bg_color", color);
      style_ctx->lookup_color("theme_selected_fg_color", color_frequency_axis_labels);

      if (use_gradient) {
        gradient_color = color;
        gradient_color.set_alpha(0.7);
      }
    }

    axis_height = draw_frequency_axis(ctx, width, height);

    int usable_height = height - axis_height;

    if (use_gradient) {
      float max_mag = std::ranges::max(spectrum_mag);
      double max_bar_height = static_cast<double>(usable_height) * max_mag;

      auto gradient = Cairo::LinearGradient::create(0.0, usable_height - max_bar_height, 0, usable_height);

      gradient->add_color_stop_rgba(0.15, color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());

      gradient->add_color_stop_rgba(1.0, gradient_color.get_red(), gradient_color.get_green(),
                                    gradient_color.get_blue(), gradient_color.get_alpha());

      ctx->set_source(gradient);
    } else {
      ctx->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());
    }

    if (spectrum_type == 0) {  // Bars
      for (uint n = 0U; n < n_points; n++) {
        double bar_height = static_cast<double>(usable_height) * spectrum_mag[n];

        if (draw_border) {
          ctx->rectangle(objects_x[n], static_cast<double>(usable_height) - bar_height,
                         static_cast<double>(width) / n_points - line_width, bar_height);
        } else {
          ctx->rectangle(objects_x[n], static_cast<double>(usable_height) - bar_height,
                         static_cast<double>(width) / n_points, bar_height);
        }
      }
    } else if (spectrum_type == 1) {  // Lines
      ctx->move_to(0, usable_height);

      for (uint n = 0U; n < n_points - 1U; n++) {
        auto bar_height = spectrum_mag[n] * static_cast<float>(usable_height);

        ctx->line_to(objects_x[n], static_cast<float>(usable_height) - bar_height);
      }

      ctx->line_to(width, usable_height);

      ctx->move_to(width, usable_height);

      ctx->close_path();
    }

    // ctx->set_antialias(Cairo::Antialias::ANTIALIAS_SUBPIXEL);

    ctx->set_line_width(line_width);

    if (settings->get_boolean("fill")) {
      ctx->fill();
    } else {
      ctx->stroke();
    }

    if (controller_motion->contains_pointer()) {
      std::ostringstream msg;

      msg.precision(0);

      msg << std::fixed << mouse_freq << " Hz, ";
      msg << std::fixed << mouse_intensity << " dB";

      Pango::FontDescription font;
      font.set_family("Monospace");
      font.set_weight(Pango::Weight::BOLD);

      int text_width = 0;
      int text_height = 0;
      auto layout = create_pango_layout(msg.str());
      layout->set_font_description(font);
      layout->get_pixel_size(text_width, text_height);

      ctx->move_to(static_cast<double>(static_cast<float>(width) - static_cast<float>(text_width)), 0);

      layout->show_in_cairo_context(ctx);
    }
  }
}

void SpectrumUi::init_color() {
  Glib::Variant<std::vector<double>> v;

  settings->get_value("color", v);

  auto rgba = v.get();

  color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);
}

void SpectrumUi::init_frequency_axis() {
  spectrum_freqs.resize(n_bands);

  for (uint n = 0; n < n_bands; n++) {
    spectrum_freqs[n] = 0.5F * rate * n / n_bands;
  }

  if (!spectrum_freqs.empty()) {
    auto npoints = settings->get_int("n-points");

    spectrum_x_axis = util::logspace(log10f(static_cast<float>(settings->get_int("minimum-frequency"))),
                                     log10f(static_cast<float>(settings->get_int("maximum-frequency"))), npoints);

    spectrum_mag.resize(npoints);

    spectrum_bin_count.resize(npoints);

    spline_f0 = spectrum_freqs[0];
    spline_df = spectrum_freqs[1] - spectrum_freqs[0];
  }
}

void SpectrumUi::init_frequency_labels_color() {
  Glib::Variant<std::vector<double>> v;

  settings->get_value("color-axis-labels", v);

  auto rgba = v.get();

  color_frequency_axis_labels.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);
}

void SpectrumUi::init_gradient_color() {
  Glib::Variant<std::vector<double>> v;

  settings->get_value("gradient-color", v);

  auto rgba = v.get();

  gradient_color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);
}

auto SpectrumUi::draw_frequency_axis(const Cairo::RefPtr<Cairo::Context>& ctx, const int& width, const int& height)
    -> int {
  int min_spectrum_freq = settings->get_int("minimum-frequency");
  int max_spectrum_freq = settings->get_int("maximum-frequency");
  int n_freq_labels = 10;
  double freq_labels_offset = width / static_cast<double>(n_freq_labels);

  auto freq_labels = util::logspace(log10f(static_cast<float>(min_spectrum_freq)),
                                    log10f(static_cast<float>(max_spectrum_freq)), n_freq_labels);

  ctx->set_source_rgba(color_frequency_axis_labels.get_red(), color_frequency_axis_labels.get_green(),
                       color_frequency_axis_labels.get_blue(), color_frequency_axis_labels.get_alpha());

  /*
    we stop the loop at freq_labels.size() - 1 because there is no space left in the window to show the last label. It
    would start to be drawn at the border of the window.
  */

  for (size_t n = 0U; n < freq_labels.size() - 1U; n++) {
    std::ostringstream msg;

    auto label = freq_labels[n];

    if (label < 1000.0) {
      msg.precision(0);
      msg << std::fixed << label << "Hz";
    } else if (label > 1000.0) {
      msg.precision(1);
      msg << std::fixed << label / 1000.0 << "kHz";
    }

    Pango::FontDescription font;
    font.set_family("Monospace");
    font.set_weight(Pango::Weight::BOLD);

    int text_width = 0;
    int text_height = 0;
    auto layout = create_pango_layout(msg.str());
    layout->set_font_description(font);
    layout->get_pixel_size(text_width, text_height);

    ctx->move_to(n * freq_labels_offset, static_cast<double>(height - text_height));

    layout->show_in_cairo_context(ctx);

    if (n == freq_labels.size() - 2U) {
      return text_height;
    }
  }

  return 0;
}
