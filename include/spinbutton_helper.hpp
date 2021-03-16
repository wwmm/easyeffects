#ifndef SPINBUTTON_HELPER_HPP
#define SPINBUTTON_HELPER_HPP

#include <gtkmm.h>
#include <sstream>

inline auto parse_spinbutton_output(Gtk::SpinButton* button, const std::string unit) -> bool {
  std::ostringstream str;

  str.precision(button->get_digits());

  str << std::fixed << button->get_adjustment()->get_value() << " " << unit;

  button->set_text(str.str());

  return true;
}

inline auto parse_spinbutton_input(Gtk::SpinButton* button, double& new_value) {
  std::istringstream str(button->get_text());

  if (str >> new_value) {
    return 1;
  }

  return GTK_INPUT_ERROR;
}

#endif