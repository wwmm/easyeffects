#ifndef LIMITER_UI_HPP
#define LIMITER_UI_HPP

#include <giomm/settings.h>
#include <gtkmm/builder.h>
#include <gtkmm/grid.h>

class LimiterUi : public Gtk::Grid {
   public:
    LimiterUi(BaseObjectType* cobject,
              const Glib::RefPtr<Gtk::Builder>& refBuilder,
              Glib::RefPtr<Gio::Settings> refSettings);
    ~LimiterUi();

    static std::unique_ptr<LimiterUi> create(std::string settings_name);

   private:
    Glib::RefPtr<Gtk::Builder> builder;
    Glib::RefPtr<Gio::Settings> settings;
};

#endif
