#include <glibmm/i18n.h>
#include <gtkmm/label.h>
#include "effects_base_ui.hpp"

EffectsBaseUi::EffectsBaseUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& refBuilder)
    : Gtk::Box(cobject), builder(refBuilder) {
    // loading glade widgets

    builder->get_widget("stack", stack);
    builder->get_widget("listbox", listbox);
    builder->get_widget("apps_box", apps_box);

    auto row = Gtk::ListBoxRow();

    row.set_name("applications");
    row.set_margin_top(6);
    row.set_margin_bottom(6);

    auto row_label = Gtk::Label(std::string("<b>") + _("Applications") +
                                std::string("</b>"));

    row_label.set_halign(Gtk::Align::ALIGN_START);
    row_label.set_use_markup(true);

    row.add(row_label);

    listbox->add(row);
}
