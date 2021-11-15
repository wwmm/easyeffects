#include "ui_helpers.hpp"

namespace ui {

using namespace std::string_literals;

auto parse_spinbutton_output(GtkSpinButton* button, const char* unit) -> bool {
  auto* adjustment = gtk_spin_button_get_adjustment(button);
  auto value = gtk_adjustment_get_value(adjustment);
  auto precision = gtk_spin_button_get_digits(button);

  // format string: 0 = value, 1 = precision, 2 = unit
  auto text = fmt::format("{0:.{1}f}{2}", value, precision, ((unit != nullptr) ? " "s + unit : ""));

  gtk_editable_set_text(GTK_EDITABLE(button), text.c_str());

  return true;
}

auto parse_spinbutton_input(GtkSpinButton* button, double* new_value) -> int {
  std::istringstream str(gtk_editable_get_text(GTK_EDITABLE(button)));

  if (auto min = 0.0, max = 0.0; str >> *new_value) {
    gtk_spin_button_get_range(button, &min, &max);

    *new_value = std::clamp(*new_value, min, max);

    return 1;
  }

  return GTK_INPUT_ERROR;
}

void prepare_spinbutton(GtkSpinButton* button, const char* unit) {
  g_object_set_data(G_OBJECT(button), "unit", const_cast<char*>(unit));

  g_signal_connect(button, "output", G_CALLBACK(+[](GtkSpinButton* button, gpointer user_data) {
                     auto unit = static_cast<char*>(g_object_get_data(G_OBJECT(button), "unit"));

                     return parse_spinbutton_output(button, unit);
                   }),
                   nullptr);

  g_signal_connect(button, "input", G_CALLBACK(+[](GtkSpinButton* button, gdouble* new_value, gpointer user_data) {
                     return parse_spinbutton_input(button, new_value);
                   }),
                   nullptr);
}

}  // namespace ui