#ifndef APP_INFO_UI_HPP
#define APP_INFO_UI_HPP

#include <gtkmm/builder.h>
#include <gtkmm/grid.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/levelbar.h>
#include <gtkmm/scale.h>
#include <gtkmm/switch.h>
#include <gtkmm/togglebutton.h>
#include "pulse_manager.hpp"

class AppInfoUi : public Gtk::Grid {
   public:
    AppInfoUi(BaseObjectType* cobject,
              const Glib::RefPtr<Gtk::Builder>& refBuilder,
              std::shared_ptr<AppInfo> app_info,
              std::shared_ptr<PulseManager> pm);

    virtual ~AppInfoUi();

    static std::unique_ptr<AppInfoUi> create(
        std::shared_ptr<AppInfo> info,
        std::shared_ptr<PulseManager> pulse_manager);

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
    Gtk::LevelBar* level;

    std::shared_ptr<AppInfo> app_info;

    void update(std::shared_ptr<AppInfo> info);

   private:
    std::string log_tag = "app_info_ui.cpp: ";

    Glib::RefPtr<Gtk::Builder> builder;

    std::shared_ptr<PulseManager> pm;

    std::string latency_to_str(uint value);

    void init_widgets();

    bool on_enable_app(bool state);

    void on_volume_changed();

    void on_mute();
};

#endif
