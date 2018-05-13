#ifndef LIMITER_UI_HPP
#define LIMITER_UI_HPP

#include <giomm/settings.h>
#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/grid.h>
#include <gtkmm/image.h>
#include <gtkmm/switch.h>

class LimiterUi : public Gtk::Grid {
   public:
    LimiterUi(BaseObjectType* cobject,
              const Glib::RefPtr<Gtk::Builder>& refBuilder,
              std::string settings_name);
    ~LimiterUi();

    static LimiterUi* create(std::string settings_name);

    std::string name = "limiter";

    Gtk::Box* listbox_control;

   private:
    Glib::RefPtr<Gtk::Builder> builder;
    Glib::RefPtr<Gio::Settings> settings;

    Gtk::Switch* limiter_enable;
    Gtk::Box* controls;
    Gtk::Image* img_state;
};

#endif
