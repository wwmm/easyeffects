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

   private:
    Glib::RefPtr<Gtk::Builder> builder;

    Gtk::Stack* stack;

    Gtk::Box* apps_box;
};

#endif
