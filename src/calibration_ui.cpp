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

#include "calibration_ui.hpp"

CalibrationUi::CalibrationUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::Window(cobject) {
  // set locale (workaround for #849)

  try {
    global_locale = std::locale("");
  } catch (const std::exception& e) {
    global_locale = std::locale();
  }

  // loading glade widgets

  builder->get_widget("stack", stack);
  builder->get_widget("spectrum", spectrum);
  builder->get_widget("headerbar", headerbar);

  spectrum->signal_draw().connect(sigc::mem_fun(*this, &CalibrationUi::on_spectrum_draw));
  spectrum->signal_enter_notify_event().connect(sigc::mem_fun(*this, &CalibrationUi::on_spectrum_enter_notify_event));
  spectrum->signal_leave_notify_event().connect(sigc::mem_fun(*this, &CalibrationUi::on_spectrum_leave_notify_event));
  spectrum->signal_motion_notify_event().connect(sigc::mem_fun(*this, &CalibrationUi::on_spectrum_motion_notify_event));

  stack->connect_property_changed("visible-child",
                                  sigc::mem_fun(*this, &CalibrationUi::on_stack_visible_child_changed));

  auto builder_signals =
      Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/calibration_signals.glade");
  auto builder_mic = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/calibration_mic.glade");

  builder_signals->get_widget_derived("widgets_grid", calibration_signals_ui);
  builder_mic->get_widget_derived("widgets_grid", calibration_mic_ui);

  stack->add(*calibration_signals_ui, "signals");
  stack->child_property_icon_name(*calibration_signals_ui).set_value("pulseeffects-sine-symbolic");

  stack->add(*calibration_mic_ui, "mic");
  stack->child_property_icon_name(*calibration_mic_ui).set_value("audio-input-microphone-symbolic");

  // default spectrum connection

  spectrum_connection =
      calibration_signals_ui->cs->new_spectrum.connect(sigc::mem_fun(*this, &CalibrationUi::on_new_spectrum));

  headerbar->set_subtitle(_("Test Signals"));
}

CalibrationUi::~CalibrationUi() {
  util::debug(log_tag + "destroyed");
}

auto CalibrationUi::create() -> CalibrationUi* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/calibration.glade");

  CalibrationUi* window = nullptr;

  builder->get_widget_derived("window", window);

  return window;
}

void CalibrationUi::set_source_monitor_name(const std::string& name) {
  calibration_mic_ui->cm->set_source_monitor_name(name);
}

void CalibrationUi::on_new_spectrum(const std::vector<float>& magnitudes) {
  spectrum_mag = magnitudes;

  min_mag = *std::min_element(spectrum_mag.begin(), spectrum_mag.end());
  max_mag = *std::max_element(spectrum_mag.begin(), spectrum_mag.end());

  spectrum->queue_draw();
}

auto CalibrationUi::on_spectrum_draw(const Cairo::RefPtr<Cairo::Context>& ctx) -> bool {
  ctx->paint();

  auto n_bars = spectrum_mag.size();

  if (n_bars > 0) {
    auto allocation = spectrum->get_allocation();
    auto width = allocation.get_width();
    auto height = allocation.get_height();
    auto n_bars = spectrum_mag.size();
    auto x = util::linspace(0.0F, static_cast<float>(width), n_bars);

    for (uint n = 0U; n < n_bars - 1U; n++) {
      auto bar_height = spectrum_mag[n] * height;

      ctx->move_to(x[n], height - bar_height);

      bar_height = spectrum_mag[n + 1U] * height;

      ctx->line_to(x[n + 1U], height - bar_height);
    }

    auto color = Gdk::RGBA();
    auto style_ctx = spectrum->get_style_context();

    style_ctx->lookup_color("theme_selected_bg_color", color);

    ctx->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), 1.0);

    ctx->set_line_width(1.5);
    ctx->stroke();

    if (mouse_inside) {
      std::ostringstream msg;

      msg.imbue(global_locale);
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

      ctx->move_to(width - text_width, 0);

      layout->show_in_cairo_context(ctx);
    }
  }

  return false;
}

auto CalibrationUi::on_spectrum_enter_notify_event(GdkEventCrossing* event) -> bool {
  mouse_inside = true;
  return false;
}

auto CalibrationUi::on_spectrum_leave_notify_event(GdkEventCrossing* event) -> bool {
  mouse_inside = false;
  return false;
}

auto CalibrationUi::on_spectrum_motion_notify_event(GdkEventMotion* event) -> bool {
  auto allocation = spectrum->get_allocation();

  auto width = allocation.get_width();
  auto height = allocation.get_height();

  // frequency axis is logarithmic
  // 20 Hz = 10^(1.3), 20000 Hz = 10^(4.3)

  mouse_freq = pow(10, 1.3 + event->x * 3.0 / width);

  // intensity scale is in decibel
  // minimum intensity is -120 dB and maximum is 0 dB

  mouse_intensity = max_mag - event->y * (max_mag - min_mag) / height;

  spectrum->queue_draw();

  return false;
}

void CalibrationUi::on_stack_visible_child_changed() {
  auto name = stack->get_visible_child_name();

  if (name == std::string("signals")) {
    spectrum_connection.disconnect();

    headerbar->set_subtitle(_("Test Signals"));

    spectrum_connection =
        calibration_signals_ui->cs->new_spectrum.connect(sigc::mem_fun(*this, &CalibrationUi::on_new_spectrum));
  } else if (name == std::string("mic")) {
    spectrum_connection.disconnect();

    headerbar->set_subtitle(_("Calibration Microphone"));

    spectrum_connection =
        calibration_mic_ui->cm->new_spectrum.connect(sigc::mem_fun(*this, &CalibrationUi::on_new_spectrum));
  }

  spectrum_mag.clear();
  spectrum->queue_draw();
}
