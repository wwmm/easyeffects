#ifndef CALIBRATION_MIC_UI_HPP
#define CALIBRATION_MIC_UI_HPP

#include <gtkmm/builder.h>
#include <gtkmm/grid.h>
#include <gtkmm/switch.h>
#include "calibration_mic.hpp"

class CalibrationMicUi : public Gtk::Grid {
   public:
    CalibrationMicUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& refBuilder);

    virtual ~CalibrationMicUi();

    static std::shared_ptr<CalibrationMicUi> create();

    std::unique_ptr<CalibrationMic> cm;

   private:
    std::string log_tag = "calibration_mic_ui: ";

    Glib::RefPtr<Gtk::Builder> builder;
};

#endif
