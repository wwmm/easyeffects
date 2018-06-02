#ifndef CALIBRATION_SIGNALS_UI_HPP
#define CALIBRATION_SIGNALS_UI_HPP

#include <gtkmm/builder.h>
#include <gtkmm/grid.h>
#include <gtkmm/switch.h>

class CalibrationSignalsUi : public Gtk::Grid {
   public:
    CalibrationSignalsUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& refBuilder);

    virtual ~CalibrationSignalsUi();

    static std::shared_ptr<CalibrationSignalsUi> create();

   private:
    std::string log_tag = "calibration_signals_ui: ";

    Glib::RefPtr<Gtk::Builder> builder;
};

#endif
