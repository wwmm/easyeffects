#ifndef SCALE_HELPER_HPP
#define SCALE_HELPER_HPP

#include <gtkmm.h>
#include <sstream>

inline auto prepare_scale(Gtk::Scale* scale, const std::string& unit) {
  scale->set_format_value_func([=](double value) {
    std::ostringstream str;

    str.precision(scale->get_digits());

    str << std::fixed << scale->get_adjustment()->get_value() << " " << unit;

    return str.str();
  });
}

#endif