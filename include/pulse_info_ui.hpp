#ifndef PULSE_INFO_UI_HPP
#define PULSE_INFO_UI_HPP

#include <giomm/settings.h>
#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/stack.h>

class PulseInfoUi : public Gtk::Box {
 public:
  PulseInfoUi(BaseObjectType* cobject,
              const Glib::RefPtr<Gtk::Builder>& refBuilder);

  virtual ~PulseInfoUi();

  static PulseInfoUi* add_to_stack(Gtk::Stack* stack);

 protected:
  std::string log_tag = "pulse_info: ";

 private:
};

#endif
