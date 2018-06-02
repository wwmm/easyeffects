#include "calibration_ui.hpp"

CalibrationUi::CalibrationUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& refBuilder)
    : Gtk::Window(cobject), builder(refBuilder) {}

CalibrationUi::~CalibrationUi() {}

std::unique_ptr<CalibrationUi> CalibrationUi::create() {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/calibration.glade");

    CalibrationUi* window = nullptr;

    builder->get_widget_derived("window", window);

    return std::unique_ptr<CalibrationUi>(window);
}
