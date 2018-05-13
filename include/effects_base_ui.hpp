#ifndef EFFECTS_BASE_UI_HPP
#define EFFECTS_BASE_UI_HPP

#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/listbox.h>
#include <gtkmm/stack.h>
#include <memory>
#include <vector>
#include "app_info_ui.hpp"
#include "pulse_manager.hpp"

class EffectsBaseUi : public Gtk::Box {
   public:
    EffectsBaseUi(BaseObjectType* cobject,
                  const Glib::RefPtr<Gtk::Builder>& refBuilder,
                  std::shared_ptr<PulseManager> pulse_manager);

    virtual ~EffectsBaseUi();

    void on_app_added(std::shared_ptr<AppInfo> app_info);
    void on_app_changed(std::shared_ptr<AppInfo> app_info);
    void on_app_removed(uint idx);

   protected:
    Gtk::ListBox* listbox;
    Gtk::Stack* stack;

    template <typename T>
    void add_to_listbox(T p) {
        auto row = new Gtk::ListBoxRow();

        row->add(*p->listbox_control);
        row->set_name(p->name);
        row->set_margin_top(6);
        row->set_margin_bottom(6);

        listbox->add(*row);
    }

   private:
    Glib::RefPtr<Gtk::Builder> builder;

    Gtk::Box* apps_box;

    std::shared_ptr<PulseManager> pm;

    std::vector<std::unique_ptr<AppInfoUi>> apps_list;
};

#endif
