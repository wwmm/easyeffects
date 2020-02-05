#ifndef PULSE_INFO_UI_HPP
#define PULSE_INFO_UI_HPP

#include <giomm/settings.h>
#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/label.h>
#include <gtkmm/listbox.h>
#include <gtkmm/stack.h>
#include "pulse_manager.hpp"

class PulseInfoUi : public Gtk::Box {
 public:
  PulseInfoUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, PulseManager* pm_ptr);
  PulseInfoUi(const PulseInfoUi&) = delete;
  auto operator=(const PulseInfoUi&) -> PulseInfoUi& = delete;
  PulseInfoUi(const PulseInfoUi&&) = delete;
  auto operator=(const PulseInfoUi &&) -> PulseInfoUi& = delete;
  ~PulseInfoUi() override;

  static auto add_to_stack(Gtk::Stack* stack, PulseManager* pm) -> PulseInfoUi*;

 private:
  std::string log_tag = "pulse_info: ";

  PulseManager* pm = nullptr;
  Gtk::Stack* stack = nullptr;

  Gtk::Label *server_name = nullptr, *server_version = nullptr, *default_sink = nullptr, *default_source = nullptr,
             *protocol = nullptr, *server_sample_format = nullptr, *server_rate = nullptr, *server_channels = nullptr,
             *server_channel_mapping = nullptr, *config_file = nullptr;

  Gtk::ListBox *listbox_modules = nullptr, *listbox_clients = nullptr, *listbox_config = nullptr,
               *listbox_resamplers = nullptr;

  std::vector<sigc::connection> connections;

  void update_server_info();
  void get_pulse_conf();
  void get_resamplers();

  static auto on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) -> int;
  void on_stack_visible_child_changed();
};

#endif
