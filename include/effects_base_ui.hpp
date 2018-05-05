#ifndef EFFECTS_BASE_UI_HPP
#define EFFECTS_BASE_UI_HPP

#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/listbox.h>
#include <gtkmm/stack.h>
#include <memory>
#include <vector>
#include "pulse_manager.hpp"

class EffectsBaseUi : public Gtk::Box {
   public:
    EffectsBaseUi(BaseObjectType* cobject,
                  const Glib::RefPtr<Gtk::Builder>& refBuilder);

    virtual ~EffectsBaseUi();

    void on_app_added(std::shared_ptr<AppInfo> app_info);
    void on_app_changed(std::shared_ptr<AppInfo> app_info);
    void on_app_removed(uint idx);

   protected:
    Gtk::ListBox* listbox;

    template <typename T>
    void add_to_listbox(T p) {
        auto row = new Gtk::ListBoxRow();

        row->add(p->ui_listbox_control);
        row->set_name(p->name);
        row->set_margin_top(6);
        row->set_margin_bottom(6);

        listbox->add(*row);
    }

    virtual void on_enable_app(bool state) {}

    virtual void on_volume_changed() {}

    virtual void on_mute() {}

   private:
    Glib::RefPtr<Gtk::Builder> builder;

    Gtk::Stack* stack;

    Gtk::Box* apps_box;

    std::vector<std::shared_ptr<AppInfo>> apps_list;
};

#endif
