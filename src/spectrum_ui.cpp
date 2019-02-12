#include "spectrum_ui.hpp"
#include "util.hpp"

SpectrumUi::SpectrumUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       Application* application)
    : Gtk::Grid(cobject),
      settings(Gio::Settings::create("com.github.wwmm.pulseeffects.spectrum")),
      app(application) {
  // loading glade widgets

  builder->get_widget("spectrum", spectrum);

  // signals connection

  spectrum->signal_draw().connect(
      sigc::mem_fun(*this, &SpectrumUi::on_spectrum_draw));
  spectrum->signal_enter_notify_event().connect(
      sigc::mem_fun(*this, &SpectrumUi::on_spectrum_enter_notify_event));
  spectrum->signal_leave_notify_event().connect(
      sigc::mem_fun(*this, &SpectrumUi::on_spectrum_leave_notify_event));
  spectrum->signal_motion_notify_event().connect(
      sigc::mem_fun(*this, &SpectrumUi::on_spectrum_motion_notify_event));

  connections.push_back(settings->signal_changed("color").connect(
      [&](auto key) { init_color(); }));

  connections.push_back(
      settings->signal_changed("background-color").connect([&](auto key) {
        init_background_color();
      }));

  connections.push_back(
      settings->signal_changed("height").connect([&](auto key) {
        auto v = settings->get_int("height");

        spectrum->set_size_request(-1, v);
      }));

  auto flag_get = Gio::SettingsBindFlags::SETTINGS_BIND_GET;

  settings->bind("show", this, "visible", flag_get);

  init_color();
  init_background_color();

  spectrum->set_size_request(-1, settings->get_int("height"));
}

SpectrumUi::~SpectrumUi() {
  app->sie->disable_spectrum();
  app->soe->disable_spectrum();

  for (auto c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

SpectrumUi* SpectrumUi::add_to_box(Gtk::Box* box, Application* app) {
  auto builder = Gtk::Builder::create_from_resource(
      "/com/github/wwmm/pulseeffects/ui/spectrum.glade");

  SpectrumUi* ui;

  builder->get_widget_derived("widgets_grid", ui, app);

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

bool SpectrumUi::on_spectrum_draw(const Cairo::RefPtr<Cairo::Context>& ctx) {
  if (settings->get_boolean("use-custom-color")) {
    ctx->set_source_rgb(background_color.get_red(),
                        background_color.get_green(),
                        background_color.get_blue());
  }

  ctx->paint_with_alpha(background_color.get_alpha());

  auto n_bars = spectrum_mag.size();

  if (n_bars > 0) {
    auto allocation = spectrum->get_allocation();
    float width = allocation.get_width();
    auto height = allocation.get_height();
    auto n_bars = spectrum_mag.size();
    auto line_width = settings->get_double("line-width");
    auto x = util::linspace(line_width, width - line_width, n_bars);
    double scale = settings->get_double("scale");
    double exponent = settings->get_double("exponent");

    auto draw_border = settings->get_boolean("show-bar-border");

    for (uint n = 0; n < n_bars; n++) {
      auto bar_height =
          height * std::min(1., std::pow(scale * spectrum_mag[n], exponent));

      if (draw_border) {
        ctx->rectangle(x[n], height - bar_height, width / n_bars - line_width,
                       bar_height);
      } else {
        ctx->rectangle(x[n], height - bar_height, width / n_bars, bar_height);
      }
    }

    // ctx->move_to(0, height);
    //
    // for (uint n = 0; n < n_bars - 1; n++) {
    //   auto bar_height = spectrum_mag[n] * height;
    //
    //   ctx->line_to(x[n], height - bar_height);
    // }
    //
    // ctx->line_to(width, height);
    //
    // ctx->move_to(width, height);
    //
    // ctx->close_path();

    if (settings->get_boolean("use-custom-color")) {
      ctx->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(),
                           color.get_alpha());
    } else {
      auto color = Gdk::RGBA();
      auto style_ctx = spectrum->get_style_context();

      style_ctx->lookup_color("theme_selected_bg_color", color);

      ctx->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(),
                           1.0);
    }

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

      int text_width;
      int text_height;
      auto layout = create_pango_layout(msg.str());
      layout->set_font_description(font);
      layout->get_pixel_size(text_width, text_height);

      ctx->move_to(width - text_width, 0);

      layout->show_in_cairo_context(ctx);
    }
  }

  return false;
}

bool SpectrumUi::on_spectrum_enter_notify_event(GdkEventCrossing* event) {
  mouse_inside = true;
  return false;
}

bool SpectrumUi::on_spectrum_leave_notify_event(GdkEventCrossing* event) {
  mouse_inside = false;
  return false;
}

bool SpectrumUi::on_spectrum_motion_notify_event(GdkEventMotion* event) {
  auto allocation = spectrum->get_allocation();

  auto width = allocation.get_width();
  auto height = allocation.get_height();

  // frequency axis is logarithmic
  // 20 Hz = 10^(1.3), 20000 Hz = 10^(4.3)

  mouse_freq = pow(10, 1.3 + event->x * 3.0 / width);

  // intensity scale is in decibel
  // minimum intensity is -120 dB and maximum is 0 dB

  mouse_intensity = -event->y * 120 / height;

  spectrum->queue_draw();

  return false;
}

void SpectrumUi::init_color() {
  Glib::Variant<std::vector<double>> v;

  settings->get_value("color", v);

  auto rgba = v.get();

  color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);
}

void SpectrumUi::init_background_color() {
  Glib::Variant<std::vector<double>> v;

  settings->get_value("background-color", v);

  auto rgba = v.get();

  background_color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);
}
