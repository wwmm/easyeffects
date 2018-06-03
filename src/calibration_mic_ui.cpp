#include "calibration_mic_ui.hpp"

CalibrationMicUi::CalibrationMicUi(BaseObjectType* cobject,
                                   const Glib::RefPtr<Gtk::Builder>& refBuilder)
    : Gtk::Grid(cobject),
      cm(std::make_unique<CalibrationMic>()),
      builder(refBuilder) {
    // loading glade widgets

    builder->get_widget("measure_noise", measure_noise);
    builder->get_widget("subtract_noise", subtract_noise);
    builder->get_widget("time_window", time_window);
    builder->get_widget("spinner", spinner);

    measure_noise->signal_clicked().connect([=]() {
        cm->measure_noise = true;
        spinner->start();
    });

    subtract_noise->signal_toggled().connect(
        [=]() { cm->subtract_noise = subtract_noise->get_active(); });

    time_window->signal_value_changed().connect(
        [=]() { cm->set_window(time_window->get_value()); });

    cm->noise_measured.connect([=]() { spinner->stop(); });

    time_window->set_value(2.0);
}

CalibrationMicUi::~CalibrationMicUi() {}

std::shared_ptr<CalibrationMicUi> CalibrationMicUi::create() {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/calibration_mic.glade");

    CalibrationMicUi* window = nullptr;

    builder->get_widget_derived("widgets_grid", window);

    return std::shared_ptr<CalibrationMicUi>(window);
}
