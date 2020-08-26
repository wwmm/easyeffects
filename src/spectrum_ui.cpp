#include "spectrum_ui.hpp"
#include "util.hpp"

SpectrumUi::SpectrumUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::Grid(cobject), settings(Gio::Settings::create("com.github.wwmm.pulseeffects.spectrum")) {
  // loading glade widgets

  builder->get_widget("spectrum", spectrum);

  // signals connection

  spectrum->signal_draw().connect(sigc::mem_fun(*this, &SpectrumUi::on_spectrum_draw));
  spectrum->signal_enter_notify_event().connect(sigc::mem_fun(*this, &SpectrumUi::on_spectrum_enter_notify_event));
  spectrum->signal_leave_notify_event().connect(sigc::mem_fun(*this, &SpectrumUi::on_spectrum_leave_notify_event));
  spectrum->signal_motion_notify_event().connect(sigc::mem_fun(*this, &SpectrumUi::on_spectrum_motion_notify_event));

  connections.emplace_back(settings->signal_changed("use-custom-color").connect([&](auto key) {
    init_color();
    init_frequency_labels_color();
    init_gradient_color();
  }));

  connections.emplace_back(settings->signal_changed("color").connect([&](auto key) { init_color(); }));

  connections.emplace_back(
      settings->signal_changed("color-axis-labels").connect([&](auto key) { init_frequency_labels_color(); }));

  connections.emplace_back(
      settings->signal_changed("gradient-color").connect([&](auto key) { init_gradient_color(); }));

  connections.emplace_back(settings->signal_changed("height").connect([&](auto key) {
    auto v = settings->get_int("height");

    spectrum->set_size_request(-1, v);
  }));

  auto flag_get = Gio::SettingsBindFlags::SETTINGS_BIND_GET;

  settings->bind("show", this, "visible", flag_get);

  init_color();
  init_frequency_labels_color();
  init_gradient_color();

  spectrum->set_size_request(-1, settings->get_int("height"));
}

SpectrumUi::~SpectrumUi() {
  for (auto& c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

auto SpectrumUi::add_to_box(Gtk::Box* box) -> SpectrumUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/spectrum.glade");

  SpectrumUi* ui = nullptr;

  builder->get_widget_derived("widgets_grid", ui);

  box->add(*ui);

  return ui;
}

void SpectrumUi::clear_spectrum() {
  spectrum_mag.resize(0);

  spectrum->queue_draw();
}

void SpectrumUi::on_new_spectrum(const std::vector<float>& magnitudes) {
  spectrum_mag = magnitudes;

  spectrum->queue_draw();
}

auto SpectrumUi::on_spectrum_draw(const Cairo::RefPtr<Cairo::Context>& ctx) -> bool {
  ctx->paint();

  auto n_points = spectrum_mag.size();

  if (n_points > 0u) {
    auto allocation = spectrum->get_allocation();
    auto width = allocation.get_width();
    auto height = allocation.get_height();
    auto line_width = static_cast<float>(settings->get_double("line-width"));
    auto objects_x = util::linspace(line_width, width - line_width, n_points);
    auto draw_border = settings->get_boolean("show-bar-border");
    auto use_gradient = settings->get_boolean("use-gradient");
    auto spectrum_type = settings->get_enum("type");

    if (!settings->get_boolean("use-custom-color")) {
      auto style_ctx = spectrum->get_style_context();

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
      float max_mag = *std::max_element(spectrum_mag.begin(), spectrum_mag.end());
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
      for (uint n = 0u; n < n_points; n++) {
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

      for (uint n = 0u; n < n_points - 1u; n++) {
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

    if (mouse_inside) {
      std::ostringstream msg;

      msg.precision(0);
      msg << std::fixed << mouse_freq << " Hz, ";
      msg << std::fixed << mouse_intensity << " dB";

      Pango::FontDescription font;
      font.set_family("Monospace");
      font.set_weight(Pango::WEIGHT_BOLD);

      int text_width = 0;
      int text_height = 0;
      auto layout = create_pango_layout(msg.str());
      layout->set_font_description(font);
      layout->get_pixel_size(text_width, text_height);

      ctx->move_to(static_cast<double>(width - static_cast<float>(text_width)), 0);

      layout->show_in_cairo_context(ctx);
    }
  }

  return false;
}

auto SpectrumUi::on_spectrum_enter_notify_event(GdkEventCrossing* event) -> bool {
  mouse_inside = true;
  return false;
}

auto SpectrumUi::on_spectrum_leave_notify_event(GdkEventCrossing* event) -> bool {
  mouse_inside = false;
  return false;
}

auto SpectrumUi::on_spectrum_motion_notify_event(GdkEventMotion* event) -> bool {
  auto allocation = spectrum->get_allocation();

  auto width = allocation.get_width();
  auto height = allocation.get_height();
  int usable_height = height - axis_height;

  if (event->y < usable_height) {
    double min_freq_log = log10(static_cast<double>(settings->get_int("minimum-frequency")));
    double max_freq_log = log10(static_cast<double>(settings->get_int("maximum-frequency")));
    double mouse_freq_log =
        event->x / static_cast<double>(width) * (max_freq_log - min_freq_log) + min_freq_log;

    mouse_freq = std::pow(10.0, mouse_freq_log);  // exp10 does not exist on FreeBSD

    // intensity scale is in decibel
    // minimum intensity is -120 dB and maximum is 0 dB

    mouse_intensity = -event->y * 120.0 / usable_height;

    spectrum->queue_draw();
  }

  return false;
}

void SpectrumUi::init_color() {
  Glib::Variant<std::vector<double>> v;

  settings->get_value("color", v);

  auto rgba = v.get();

  color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);
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

  auto freq_labels = util::logspace(static_cast<float>(log10(min_spectrum_freq)),
                                    static_cast<float>(log10(max_spectrum_freq)), n_freq_labels);

  ctx->set_source_rgba(color_frequency_axis_labels.get_red(), color_frequency_axis_labels.get_green(),
                       color_frequency_axis_labels.get_blue(), color_frequency_axis_labels.get_alpha());

  /*
    we stop the loop at freq_labels.size() - 1 because there is no space left in the window to show the last label. It
    would start to be drawn at the border of the window.
  */

  for (size_t n = 0u; n < freq_labels.size() - 1u; n++) {
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
    font.set_weight(Pango::WEIGHT_BOLD);

    int text_width = 0;
    int text_height = 0;
    auto layout = create_pango_layout(msg.str());
    layout->set_font_description(font);
    layout->get_pixel_size(text_width, text_height);

    ctx->move_to(n * freq_labels_offset, static_cast<double>(height - text_height));

    layout->show_in_cairo_context(ctx);

    if (n == freq_labels.size() - 2u) {
      return text_height;
    }
  }

  return 0;
}
