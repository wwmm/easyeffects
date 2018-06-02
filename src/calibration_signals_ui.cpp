#include "calibration_signals_ui.hpp"

CalibrationSignalsUi::CalibrationSignalsUi(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder)
    : Gtk::Grid(cobject), builder(refBuilder) {
    // loading glade widgets

    builder->get_widget("enable", enable);
    builder->get_widget("volume", volume);
    builder->get_widget("frequency", frequency);
}

CalibrationSignalsUi::~CalibrationSignalsUi() {}

std::shared_ptr<CalibrationSignalsUi> CalibrationSignalsUi::create() {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/calibration_signals.glade");

    CalibrationSignalsUi* window = nullptr;

    builder->get_widget_derived("widgets_grid", window);

    return std::shared_ptr<CalibrationSignalsUi>(window);
}
