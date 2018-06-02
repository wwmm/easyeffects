#ifndef CALIBRATION_SIGNALS_UI_HPP
#define CALIBRATION_SIGNALS_UI_HPP

#include <gtkmm/builder.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include <gtkmm/scale.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/switch.h>
#include "calibration_signals.hpp"

class CalibrationSignalsUi : public Gtk::Grid {
   public:
    CalibrationSignalsUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& refBuilder);

    virtual ~CalibrationSignalsUi();

    static std::shared_ptr<CalibrationSignalsUi> create();

    std::unique_ptr<CalibrationSignals> cs;

   private:
    std::string log_tag = "calibration_signals_ui: ";

    Glib::RefPtr<Gtk::Builder> builder;

    Gtk::Switch* enable;
    Gtk::Scale* volume;
    Gtk::SpinButton* frequency;
    Gtk::ComboBoxText* wave;
};

#endif
