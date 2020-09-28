#include "effects_base_ui.hpp"
#include <glibmm/i18n.h>
#include "plugin_ui_base.hpp"

EffectsBaseUi::EffectsBaseUi(const Glib::RefPtr<Gtk::Builder>& builder,
                             Glib::RefPtr<Gio::Settings> refSettings,
                             PulseManager* pulse_manager)
    : settings(std::move(refSettings)), pm(pulse_manager) {
  // loading glade widgets

  builder->get_widget("stack", stack);
  builder->get_widget("listbox", listbox);
  builder->get_widget("apps_box", apps_box);
  builder->get_widget("placeholder_spectrum", placeholder_spectrum);

  auto b_app_button_row = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/app_button_row.glade");

  b_app_button_row->get_widget("app_button_row", app_button_row);
  b_app_button_row->get_widget("app_input_icon", app_input_icon);
  b_app_button_row->get_widget("app_output_icon", app_output_icon);
  b_app_button_row->get_widget("global_level_meter_grid", global_level_meter_grid);
  b_app_button_row->get_widget("global_output_level_left", global_output_level_left);
  b_app_button_row->get_widget("global_output_level_right", global_output_level_right);
  b_app_button_row->get_widget("saturation_icon", saturation_icon);

  // spectrum

  spectrum_ui = SpectrumUi::add_to_box(placeholder_spectrum);

  // setting up plugin list box

  auto* row = Gtk::manage(new Gtk::ListBoxRow());

  row->set_name("applications");
  row->set_margin_top(6);
  row->set_margin_bottom(6);
  row->set_margin_right(6);
  row->set_margin_left(6);

  row->add(*app_button_row);

  listbox->add(*row);

  // plugin rows connections

  listbox->signal_row_activated().connect([&](auto row) { stack->set_visible_child(row->get_name()); });

  listbox->set_sort_func(sigc::mem_fun(*this, &EffectsBaseUi::on_listbox_sort));

  connections.emplace_back(settings->signal_changed("plugins").connect([=](auto key) { listbox->invalidate_sort(); }));
}

EffectsBaseUi::~EffectsBaseUi() {
  for (auto& c : connections) {
    c.disconnect();
  }
}

void EffectsBaseUi::on_app_changed(const std::shared_ptr<AppInfo>& app_info) {
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
      auto* appui = apps_list[n];

      apps_box->remove(*appui);

      apps_list.erase(it);

      break;
    }
  }
}

auto EffectsBaseUi::on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) -> int {
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
  }

  if (idx1 > idx2) {
    return 1;
  }

  return 0;
}

void EffectsBaseUi::on_new_output_level_db(const std::array<double, 2>& peak) {
  auto left = peak[0];
  auto right = peak[1];

  // show the grid only if something is playing/recording

  if (left <= -100.0 && right <= -100.0) {
    global_level_meter_grid->set_visible(false);

    return;
  }

  global_level_meter_grid->set_visible(true);

  global_output_level_left->set_text(PluginUiBase::level_to_str_showpos(left, 0));

  global_output_level_right->set_text(PluginUiBase::level_to_str_showpos(right, 0));

  // saturation icon notification

  if (left > 0.0 || right > 0.0) {
    saturation_icon->set_visible(true);
  } else {
    saturation_icon->set_visible(false);
  }
}
