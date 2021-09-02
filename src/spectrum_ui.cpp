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

#include "spectrum_ui.hpp"

SpectrumUi::SpectrumUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::DrawingArea(cobject), settings(Gio::Settings::create("com.github.wwmm.easyeffects.spectrum")) {
  plot = std::make_unique<Plot>(this);

  // signals connection

  connections.emplace_back(settings->signal_changed("color").connect([&](const auto& key) { init_color(); }));

  connections.emplace_back(
      settings->signal_changed("color-axis-labels").connect([&](const auto& key) { init_frequency_labels_color(); }));

  connections.emplace_back(
      settings->signal_changed("height").connect([&](const auto& key) { set_content_height(settings->get_int("height")); }));

  connections.emplace_back(settings->signal_changed("n-points").connect([&](const auto& key) { init_frequency_axis(); }));

  connections.emplace_back(
      settings->signal_changed("minimum-frequency").connect([&](const auto& key) { init_frequency_axis(); }));

  connections.emplace_back(
      settings->signal_changed("maximum-frequency").connect([&](const auto& key) { init_frequency_axis(); }));

  connections.emplace_back(settings->signal_changed("type").connect([&](const auto& key) { init_type(); }));

  connections.emplace_back(
      settings->signal_changed("fill").connect([&](const auto& key) { plot->set_fill_bars(settings->get_boolean(key)); }));

  connections.emplace_back(settings->signal_changed("show-bar-border").connect([&](const auto& key) {
    plot->set_draw_bar_border(settings->get_boolean(key));
  }));

  connections.emplace_back(settings->signal_changed("line-width").connect([&](const auto& key) {
    plot->set_line_width(static_cast<float>(settings->get_double("line-width")));
  }));

  settings->bind("show", this, "visible", Gio::Settings::BindFlags::GET);

  init_color();
  init_frequency_labels_color();
  init_type();

  plot->set_fill_bars(settings->get_boolean("fill"));

  plot->set_draw_bar_border(settings->get_boolean("show-bar-border"));

  plot->set_line_width(static_cast<float>(settings->get_double("line-width")));

  plot->set_x_unit("Hz");
  plot->set_y_unit("dB");

  plot->set_n_x_decimals(0);
  plot->set_n_y_decimals(1);

  set_content_height(settings->get_int("height"));
}

SpectrumUi::~SpectrumUi() {
  for (auto& c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

auto SpectrumUi::add_to_box(Gtk::Box* box) -> SpectrumUi* {
  const auto& builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/spectrum.ui");

  auto* const ui = Gtk::Builder::get_widget_derived<SpectrumUi>(builder, "drawing_area");

  box->append(*ui);

  return ui;
}

void SpectrumUi::on_new_spectrum(uint rate, uint n_bands, std::vector<float> magnitudes) {
  if (!settings->get_boolean("show")) {
    return;
  }

  if (this->rate != rate || this->n_bands != n_bands) {
    this->rate = rate;
    this->n_bands = n_bands;

    init_frequency_axis();
  }

  std::ranges::fill(spectrum_mag, 0.0F);
  std::ranges::fill(spectrum_bin_count, 0U);

  // reducing the amount of data so we can plot them

  for (size_t j = 0U, y = spectrum_freqs.size(); j < y; j++) {
    for (size_t n = 0U, m = spectrum_x_axis.size(); n < m; n++) {
      if (n > 0U) {
        if (spectrum_freqs[j] <= spectrum_x_axis[n] && spectrum_freqs[j] > spectrum_x_axis[n - 1U]) {
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

  for (size_t n = 0U, m = spectrum_bin_count.size(); n < m; n++) {
    if (spectrum_bin_count[n] == 0U && n > 0U) {
      spectrum_mag[n] = spectrum_mag[n - 1U];

      // doing a very poor interpolation for the bins without any count

      // int count = 1;

      // while (count + n < spectrum_bin_count.size()) {
      //   if (spectrum_bin_count[count + n] != 0) {
      //     break;
      //   }

      //   count++;
      // }

      // float last_mag = spectrum_mag[n + count];

      // float delta = (last_mag - spectrum_mag[n - 1]) / static_cast<float>(count);

      // for (int k = 0; k < count; k++) {
      //   spectrum_mag[n + k] = spectrum_mag[n - 1] + k * delta;

      //   spectrum_bin_count[n + k] = 1;
      // }
    }
  }

  std::ranges::for_each(spectrum_mag, [](auto& v) {
    v = 10.0F * std::log10(v);

    if (!std::isinf(v)) {
      v = (v > util::minimum_db_level) ? v : util::minimum_db_level;
    } else {
      v = util::minimum_db_level;
    }
  });

  plot->set_data(spectrum_x_axis, spectrum_mag);
}

void SpectrumUi::init_color() {
  Glib::Variant<std::vector<double>> v;

  settings->get_value("color", v);

  const auto& rgba = v.get();

  plot->set_color(rgba[0], rgba[1], rgba[2], rgba[3]);
}

void SpectrumUi::init_type() {
  if (settings->get_string("type") == "Bars") {
    plot->set_plot_type(PlotType::bar);
  } else if (settings->get_string("type") == "Lines") {
    plot->set_plot_type(PlotType::line);
  }
}

void SpectrumUi::init_frequency_axis() {
  spectrum_freqs.resize(n_bands);

  for (uint n = 0U; n < n_bands; n++) {
    spectrum_freqs[n] = 0.5F * rate * static_cast<float>(n) / static_cast<float>(n_bands);
  }

  if (!spectrum_freqs.empty()) {
    spectrum_x_axis = util::logspace(std::log10(static_cast<float>(settings->get_int("minimum-frequency"))),
                                     std::log10(static_cast<float>(settings->get_int("maximum-frequency"))),
                                     settings->get_int("n-points"));

    spectrum_mag.resize(spectrum_x_axis.size());

    spectrum_bin_count.resize(spectrum_x_axis.size());
  }
}

void SpectrumUi::init_frequency_labels_color() {
  Glib::Variant<std::vector<double>> v;

  settings->get_value("color-axis-labels", v);

  const auto& rgba = v.get();

  plot->set_axis_labels_color(rgba[0], rgba[1], rgba[2], rgba[3]);
}
