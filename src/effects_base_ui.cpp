#include "effects_base_ui.hpp"
#include <glibmm/i18n.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>

EffectsBaseUi::EffectsBaseUi(const Glib::RefPtr<Gtk::Builder>& builder,
                             const Glib::RefPtr<Gio::Settings>& refSettings,
                             PulseManager* pulse_manager)
    : settings(refSettings), pm(pulse_manager) {
  // loading glade widgets

  builder->get_widget("stack", stack);
  builder->get_widget("listbox", listbox);
  builder->get_widget("apps_box", apps_box);
  builder->get_widget("placeholder_spectrum", placeholder_spectrum);

  spectrum_ui = SpectrumUi::add_to_box(placeholder_spectrum);

  auto row = Gtk::manage(new Gtk::ListBoxRow());

  row->set_name("applications");
  row->set_margin_top(6);
  row->set_margin_bottom(6);
  row->set_margin_right(6);
  row->set_margin_left(6);

  auto row_label = Gtk::manage(new Gtk::Label(_("Applications")));

  row_label->set_halign(Gtk::Align::ALIGN_START);

  row->add(*row_label);

  listbox->add(*row);

  listbox->signal_row_activated().connect([&](auto row) { stack->set_visible_child(row->get_name()); });

  listbox->set_sort_func(sigc::mem_fun(*this, &EffectsBaseUi::on_listbox_sort));

  connections.push_back(settings->signal_changed("plugins").connect([=](auto key) { listbox->invalidate_sort(); }));
}

EffectsBaseUi::~EffectsBaseUi() {
  for (auto c : connections) {
    c.disconnect();
  }
}

void EffectsBaseUi::on_app_added(std::shared_ptr<AppInfo> app_info) {
  for (auto a : apps_list) {
    if (a->app_info->index == app_info->index) {
      // do not add the same app two times in the interface
      return;
    }
  }

  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/app_info.glade");

  AppInfoUi* appui = nullptr;

  builder->get_widget_derived("widgets_grid", appui, app_info, pm);

  apps_box->add(*appui);

  apps_list.push_back(appui);
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
      auto appui = apps_list[n];

      apps_box->remove(*appui);

      apps_list.erase(it);

      break;
    }
  }
}

int EffectsBaseUi::on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) {
  auto name1 = row1->get_name();
  auto name2 = row2->get_name();

  auto order = Glib::Variant<std::vector<std::string>>();

  settings->get_value("plugins", order);

  auto vorder = order.get();

  auto r1 = std::find(std::begin(vorder), std::end(vorder), name1);
  auto r2 = std::find(std::begin(vorder), std::end(vorder), name2);

  auto idx1 = r1 - vorder.begin();
  auto idx2 = r2 - vorder.begin();

  // we do not want the applications row to be moved

  if (name1 == std::string("applications")) {
    return -1;
  }

  if (name2 == std::string("applications")) {
    return 1;
  }

  if (idx1 < idx2) {
    return -1;
  } else if (idx1 > idx2) {
    return 1;
  } else {
    return 0;
  }
}
