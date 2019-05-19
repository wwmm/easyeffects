#ifndef PULSE_INFO_UI_HPP
#define PULSE_INFO_UI_HPP

#include <giomm/settings.h>
#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/label.h>
#include <gtkmm/stack.h>
#include "pulse_manager.hpp"

class PulseInfoUi : public Gtk::Box {
 public:
  PulseInfoUi(BaseObjectType* cobject,
              const Glib::RefPtr<Gtk::Builder>& builder,
              PulseManager* pm_ptr);

  virtual ~PulseInfoUi();

  static PulseInfoUi* add_to_stack(Gtk::Stack* stack, PulseManager* pm);

 private:
  std::string log_tag = "pulse_info: ";

  PulseManager* pm;

  Gtk::Label *server_name, *server_version, *default_sink, *default_source,
      *protocol, *server_sample_format, *server_rate, *server_channels,
      *server_channel_mapping;

  std::vector<sigc::connection> connections;

  void update_server_info();
};

#endif
