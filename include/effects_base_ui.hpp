#ifndef EFFECTS_BASE_UI_HPP
#define EFFECTS_BASE_UI_HPP

#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/listbox.h>
#include <gtkmm/stack.h>
#include <memory>

class EffectsBaseUi : public Gtk::Box {
   public:
    EffectsBaseUi(BaseObjectType* cobject,
                  const Glib::RefPtr<Gtk::Builder>& refBuilder);

    virtual ~EffectsBaseUi();

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

   private:
    Glib::RefPtr<Gtk::Builder> builder;

    Gtk::Stack* stack;

    Gtk::Box* apps_box;
};

#endif
