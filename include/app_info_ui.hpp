#ifndef APP_INFO_UI_HPP
#define APP_INFO_UI_HPP

#include <gtkmm/builder.h>
#include <gtkmm/grid.h>

class AppInfoUi : public Gtk::Grid {
   public:
    AppInfoUi(BaseObjectType* cobject,
              const Glib::RefPtr<Gtk::Builder>& refBuilder);

    virtual ~AppInfoUi();

    static std::unique_ptr<AppInfoUi> create();
};

#endif
