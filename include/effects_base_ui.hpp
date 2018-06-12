#ifndef EFFECTS_BASE_UI_HPP
#define EFFECTS_BASE_UI_HPP

#include <giomm/settings.h>
#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/listbox.h>
#include <gtkmm/stack.h>
#include <memory>
#include <vector>
#include "app_info_ui.hpp"
#include "pulse_manager.hpp"

class EffectsBaseUi {
   public:
    EffectsBaseUi(const Glib::RefPtr<Gtk::Builder>& refBuilder,
                  const Glib::RefPtr<Gio::Settings>& refSettings,
                  const std::shared_ptr<PulseManager>& pulse_manager);

    virtual ~EffectsBaseUi();

    void on_app_added(std::shared_ptr<AppInfo> app_info);
    void on_app_changed(std::shared_ptr<AppInfo> app_info);
    void on_app_removed(uint idx);

   protected:
    Glib::RefPtr<Gio::Settings> settings;
    Gtk::ListBox* listbox;
    Gtk::Stack* stack;

    std::vector<sigc::connection> connections;

    template <typename T>
    void add_to_listbox(T p) {
        auto row = Gtk::manage(new Gtk::ListBoxRow());

        row->add(*p->listbox_control);
        row->set_name(p->name);
        row->set_margin_top(6);
        row->set_margin_bottom(6);

        listbox->add(*row);
    }

   private:
    Gtk::Box* apps_box;

    std::shared_ptr<PulseManager> pm;

    std::vector<std::unique_ptr<AppInfoUi>> apps_list;

    int on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2);
};

#endif
