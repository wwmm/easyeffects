#ifndef CALIBRATION_UI_HPP
#define CALIBRATION_UI_HPP

#include <gtkmm/builder.h>
#include <gtkmm/window.h>

class CalibrationUi : public Gtk::Window {
   public:
    CalibrationUi(BaseObjectType* cobject,
                  const Glib::RefPtr<Gtk::Builder>& refBuilder);

    virtual ~CalibrationUi();

    static std::unique_ptr<CalibrationUi> create();

   private:
    Glib::RefPtr<Gtk::Builder> builder;
};

#endif
