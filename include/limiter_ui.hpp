#ifndef LIMITER_UI_HPP
#define LIMITER_UI_HPP

#include <giomm/settings.h>
#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/grid.h>

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
};

#endif
