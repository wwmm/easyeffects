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
            std::shared_ptr<AppInfo> info,
            PulseManager* pulse_manager);
  AppInfoUi(const AppInfoUi&) = delete;
  auto operator=(const AppInfoUi&) -> AppInfoUi& = delete;
  AppInfoUi(const AppInfoUi&&) = delete;
  auto operator=(const AppInfoUi &&) -> AppInfoUi& = delete;
  ~AppInfoUi() override;

  Gtk::Switch* enable = nullptr;
  Gtk::Image* app_icon = nullptr;
  Gtk::Label* app_name = nullptr;
  Gtk::Scale* volume = nullptr;
  Gtk::ToggleButton* mute = nullptr;
  Gtk::ToggleButton* blacklist = nullptr;
  Gtk::Image* mute_icon = nullptr;
  Gtk::Label* format = nullptr;
  Gtk::Label* rate = nullptr;
  Gtk::Label* channels = nullptr;
  Gtk::Label* resampler = nullptr;
  Gtk::Label* buffer = nullptr;
  Gtk::Label* latency = nullptr;
  Gtk::Label* state = nullptr;

  std::shared_ptr<AppInfo> app_info;

  void update(const std::shared_ptr<AppInfo>& info);

 private:
  std::string log_tag = "app_info_ui: ";

  bool running = true;

  sigc::connection enable_connection;
  sigc::connection volume_connection;
  sigc::connection mute_connection;
  sigc::connection blacklist_connection;
  sigc::connection timeout_connection;

  PulseManager* pm = nullptr;

  void init_widgets();

  void connect_signals();

  static auto latency_to_str(uint value) -> std::string;

  auto on_enable_app(bool state) -> bool;

  void on_volume_changed();

  void on_mute();
};

#endif
