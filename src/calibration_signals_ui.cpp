#include "calibration_signals_ui.hpp"

CalibrationSignalsUi::CalibrationSignalsUi(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder)
    : Gtk::Grid(cobject),
      builder(refBuilder),
      cs(std::make_unique<CalibrationSignals>()) {
    // loading glade widgets

    builder->get_widget("enable", enable);
    builder->get_widget("volume", volume);
    builder->get_widget("frequency", frequency);
    builder->get_widget("wave", wave);

    enable->signal_state_set().connect(
        [=](bool status) {
            if (status) {
                cs->start();
            } else {
                cs->stop();
            }

            return false;
        },
        false);

    g_object_bind_property(wave->gobj(), "active", cs->source, "wave",
                           G_BINDING_DEFAULT);

    frequency->signal_value_changed().connect(
        [=]() { cs->set_freq(frequency->get_value()); });

    volume->signal_value_changed().connect(
        [=]() { cs->set_volume(volume->get_value()); });

    // default values

    wave->set_active(0);  // sine
    frequency->set_value(1000);
    volume->set_value(1.0);
}

CalibrationSignalsUi::~CalibrationSignalsUi() {}

std::shared_ptr<CalibrationSignalsUi> CalibrationSignalsUi::create() {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/calibration_signals.glade");

    CalibrationSignalsUi* window = nullptr;

    builder->get_widget_derived("widgets_grid", window);

    return std::shared_ptr<CalibrationSignalsUi>(window);
}
