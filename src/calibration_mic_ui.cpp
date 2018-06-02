#include "calibration_mic_ui.hpp"

CalibrationMicUi::CalibrationMicUi(BaseObjectType* cobject,
                                   const Glib::RefPtr<Gtk::Builder>& refBuilder)
    : Gtk::Grid(cobject),
      cm(std::make_unique<CalibrationMic>()),
      builder(refBuilder) {}

CalibrationMicUi::~CalibrationMicUi() {}

std::shared_ptr<CalibrationMicUi> CalibrationMicUi::create() {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/calibration_mic.glade");

    CalibrationMicUi* window = nullptr;

    builder->get_widget_derived("widgets_grid", window);

    return std::shared_ptr<CalibrationMicUi>(window);
}
