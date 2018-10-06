#ifndef APP_INFO_UI_HPP
#define APP_INFO_UI_HPP

#include <gtkmm/builder.h>
#include <gtkmm/grid.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>
#include <gtkmm/switch.h>
#include <gtkmm/togglebutton.h>
#include "pulse_manager.hpp"

class AppInfoUi : public Gtk::Grid {
 public:
  AppInfoUi(BaseObjectType* cobject,
            const Glib::RefPtr<Gtk::Builder>& builder,
            const std::shared_ptr<AppInfo>& app_info,
            PulseManager* pm);

  virtual ~AppInfoUi();

  Gtk::Switch* enable;
  Gtk::Image* app_icon;
  Gtk::Label* app_name;
  Gtk::Scale* volume;
  Gtk::ToggleButton* mute;
  Gtk::Image* mute_icon;
  Gtk::Label* format;
  Gtk::Label* rate;
  Gtk::Label* channels;
  Gtk::Label* resampler;
  Gtk::Label* buffer;
  Gtk::Label* latency;
  Gtk::Label* state;

  std::shared_ptr<AppInfo> app_info;

  void update(std::shared_ptr<AppInfo> info);

 private:
  std::string log_tag = "app_info_ui: ";

  bool running = true;

  sigc::connection enable_connection;
  sigc::connection volume_connection;
  sigc::connection mute_connection;
  sigc::connection timeout_connection;

  PulseManager* pm;

  void init_widgets();

  void connect_signals();

  std::string latency_to_str(uint value);

  bool on_enable_app(bool state);

  void on_volume_changed();

  void on_mute();
};

#endif
