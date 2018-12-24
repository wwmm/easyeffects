#include "spectrum_ui.hpp"
#include "util.hpp"

SpectrumUi::SpectrumUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       const Glib::RefPtr<Gio::Settings>& refSettings,
                       Application* application)
    : Gtk::Grid(cobject), settings(refSettings), app(application) {
  // loading glade widgets

  builder->get_widget("show_spectrum", show_spectrum);
  builder->get_widget("spectrum_fill", spectrum_fill);
  builder->get_widget("spectrum_box", spectrum_box);
  builder->get_widget("spectrum", spectrum);
  builder->get_widget("spectrum_color_button", spectrum_color_button);

  get_object(builder, "spectrum_n_points", spectrum_n_points);
  get_object(builder, "spectrum_height", spectrum_height);
  get_object(builder, "spectrum_scale", spectrum_scale);
  get_object(builder, "spectrum_exponent", spectrum_exponent);
  get_object(builder, "spectrum_sampling_freq", spectrum_sampling_freq);

  // signals connection

  show_spectrum->signal_state_set().connect(
      sigc::mem_fun(*this, &SpectrumUi::on_show_spectrum), false);

  spectrum->signal_draw().connect(
      sigc::mem_fun(*this, &SpectrumUi::on_spectrum_draw));
  spectrum->signal_enter_notify_event().connect(
      sigc::mem_fun(*this, &SpectrumUi::on_spectrum_enter_notify_event));
  spectrum->signal_leave_notify_event().connect(
      sigc::mem_fun(*this, &SpectrumUi::on_spectrum_leave_notify_event));
  spectrum->signal_motion_notify_event().connect(
      sigc::mem_fun(*this, &SpectrumUi::on_spectrum_motion_notify_event));

  connections.push_back(
      settings->signal_changed("spectrum-color").connect([&](auto key) {
        Glib::Variant<std::vector<double>> v;
        settings->get_value("spectrum-color", v);
        auto rgba = v.get();
        spectrum_color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);
        spectrum_color_button->set_rgba(spectrum_color);
      }));

  spectrum_color_button->signal_color_set().connect([=]() {
    spectrum_color = spectrum_color_button->get_rgba();
    auto v = Glib::Variant<std::vector<double>>::create(std::vector<double>{
        spectrum_color.get_red(), spectrum_color.get_green(),
        spectrum_color.get_blue(), spectrum_color.get_alpha()});
    settings->set_value("spectrum-color", v);
  });

  use_custom_color->signal_state_set().connect(
      sigc::mem_fun(*this, &SpectrumUi::on_use_custom_color), false);

  spectrum_height->signal_value_changed().connect(
      [=]() { spectrum->set_size_request(-1, spectrum_height->get_value()); });

  spectrum_sampling_freq->signal_value_changed().connect(
      sigc::mem_fun(*this, &SpectrumUi::on_spectrum_sampling_freq_set), false);
}

SpectrumUi::~SpectrumUi() {
  for (auto c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

void SpectrumUi::clear_spectrum() {
  spectrum_mag.resize(0);

  spectrum->queue_draw();
}

bool SpectrumUi::on_show_spectrum(bool state) {
  if (state) {
    app->sie->enable_spectrum();
    app->soe->enable_spectrum();
  } else {
    app->sie->disable_spectrum();
    app->soe->disable_spectrum();
  }

  return false;
}

bool SpectrumUi::on_use_custom_color(bool state) {
  if (state) {
    Glib::Variant<std::vector<double>> v;

    settings->get_value("spectrum-color", v);

    auto rgba = v.get();

    spectrum_color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);

    spectrum_color_button->set_rgba(spectrum_color);
  }

  return false;
}

void SpectrumUi::on_new_spectrum(const std::vector<float>& magnitudes) {
  spectrum_mag = magnitudes;

  spectrum->queue_draw();
}

bool SpectrumUi::on_spectrum_draw(const Cairo::RefPtr<Cairo::Context>& ctx) {
  ctx->paint();

  auto n_bars = spectrum_mag.size();

  if (n_bars > 0) {
    auto allocation = spectrum->get_allocation();
    auto width = allocation.get_width();
    auto height = allocation.get_height();
    auto n_bars = spectrum_mag.size();
    auto x = util::linspace(0, width, n_bars);
    double scale = spectrum_scale.get()->get_value();
    double exponent = spectrum_exponent.get()->get_value();

    for (uint n = 0; n < n_bars; n++) {
      auto bar_height =
          height * std::min(1., std::pow(scale * spectrum_mag[n], exponent));

      ctx->rectangle(x[n], height - bar_height, width / n_bars, bar_height);
    }

    if (settings->get_boolean("use-custom-color")) {
      ctx->set_source_rgba(spectrum_color.get_red(), spectrum_color.get_green(),
                           spectrum_color.get_blue(),
                           spectrum_color.get_alpha());
    } else {
      auto color = Gdk::RGBA();
      auto style_ctx = spectrum->get_style_context();

      style_ctx->lookup_color("theme_selected_bg_color", color);

      ctx->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(),
                           1.0);
    }

    ctx->set_line_width(1.1);

    if (spectrum_fill->get_active())
      ctx->fill();
    else
      ctx->stroke();

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

void SpectrumUi::on_spectrum_sampling_freq_set() {
  app->sie->update_spectrum_interval(spectrum_sampling_freq->get_value());
  app->soe->update_spectrum_interval(spectrum_sampling_freq->get_value());
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
