#include <glibmm/i18n.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include "effects_base_ui.hpp"

EffectsBaseUi::EffectsBaseUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& refBuilder,
                             std::shared_ptr<PulseManager> pulse_manager)
    : Gtk::Box(cobject), builder(refBuilder), pm(pulse_manager) {
    // loading glade widgets

    builder->get_widget("stack", stack);
    builder->get_widget("listbox", listbox);
    builder->get_widget("apps_box", apps_box);

    auto row = new Gtk::ListBoxRow();

    row->set_name("applications");
    row->set_margin_top(6);
    row->set_margin_bottom(6);

    auto row_label = new Gtk::Label(std::string("<b>") + _("Applications") +
                                    std::string("</b>"));

    row_label->set_halign(Gtk::Align::ALIGN_START);
    row_label->set_use_markup(true);

    row->add(*row_label);

    listbox->add(*row);

    listbox->signal_row_activated().connect(
        [&](auto row) { stack->set_visible_child(row->get_name()); });
}

EffectsBaseUi::~EffectsBaseUi() {}

void EffectsBaseUi::on_app_added(std::shared_ptr<AppInfo> app_info) {
    auto appui = AppInfoUi::create(app_info, pm);

    apps_box->add(*appui);

    apps_list.push_back(move(appui));
}

void EffectsBaseUi::on_app_changed(std::shared_ptr<AppInfo> app_info) {
    for (auto it = apps_list.begin(); it != apps_list.end(); it++) {
        auto n = it - apps_list.begin();

        if (apps_list[n]->app_info->index == app_info->index) {
            apps_list[n]->update(app_info);
        }
    }
}

void EffectsBaseUi::on_app_removed(uint idx) {
    for (auto it = apps_list.begin(); it != apps_list.end(); it++) {
        auto n = it - apps_list.begin();

        if (apps_list[n]->app_info->index == idx) {
            auto appui = move(apps_list[n]);

            apps_list.erase(it);

            apps_box->remove(*appui);

            break;
        }
    }
}
