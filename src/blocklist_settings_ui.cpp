/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "blocklist_settings_ui.hpp"

// static attributes initialization
BlocklistSettingsUi* BlocklistSettingsUi::thisPtr = nullptr;
Gtk::ListBox* BlocklistSettingsUi::blocklist_in_listbox = nullptr;
Gtk::ListBox* BlocklistSettingsUi::blocklist_out_listbox = nullptr;
std::vector<sigc::connection> BlocklistSettingsUi::connections;

// constructor
BlocklistSettingsUi::BlocklistSettingsUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::Grid(cobject), settings(Gio::Settings::create("com.github.wwmm.pulseeffects")) {
  if (thisPtr == nullptr) {
    thisPtr = this;
  }

  // loading glade widgets

  show_blocklisted_apps = builder->get_widget<Gtk::Switch>("show_blocklisted_apps");
  blocklist_in_scrolled_window = builder->get_widget<Gtk::ScrolledWindow>("blocklist_in_scrolled_window");
  blocklist_out_scrolled_window = builder->get_widget<Gtk::ScrolledWindow>("blocklist_out_scrolled_window");
  add_blocklist_in = builder->get_widget<Gtk::Button>("add_blocklist_in");
  add_blocklist_out = builder->get_widget<Gtk::Button>("add_blocklist_out");
  blocklist_in_listbox = builder->get_widget<Gtk::ListBox>("blocklist_in_listbox");
  blocklist_out_listbox = builder->get_widget<Gtk::ListBox>("blocklist_out_listbox");
  blocklist_in_name = builder->get_widget<Gtk::Entry>("blocklist_in_name");
  blocklist_out_name = builder->get_widget<Gtk::Entry>("blocklist_out_name");

  // signals connection

  add_blocklist_in->signal_clicked().connect([=]() {
    if (add_new_entry(blocklist_in_name->get_text(), PresetType::input)) {
      blocklist_in_name->set_text("");
    }
  });

  add_blocklist_out->signal_clicked().connect([=]() {
    if (add_new_entry(blocklist_out_name->get_text(), PresetType::output)) {
      blocklist_out_name->set_text("");
    }
  });

  auto settings = Gio::Settings::create("com.github.wwmm.pulseeffects");

  settings->bind("show-blocklisted-apps", show_blocklisted_apps, "active");

  populate_blocklist_in_listbox();
  populate_blocklist_out_listbox();
}

BlocklistSettingsUi::~BlocklistSettingsUi() {
  for (auto& c : connections) {
    c.disconnect();
  }

  thisPtr = nullptr;

  util::debug(log_tag + "destroyed");
}

auto BlocklistSettingsUi::get_blocklisted_apps_visibility() -> bool {
  Glib::RefPtr<Gio::Settings> settingsPtr;

  if (thisPtr == nullptr) {
    settingsPtr = Gio::Settings::create("com.github.wwmm.pulseeffects");
  } else {
    settingsPtr = thisPtr->settings;
  }

  return settingsPtr->get_boolean("show-blocklisted-apps");
}

auto BlocklistSettingsUi::add_new_entry(const Glib::ustring& name, PresetType preset_type) -> bool {
  if (name.empty()) {
    return false;
  }

  Glib::RefPtr<Gio::Settings> settingsPtr;

  if (thisPtr == nullptr) {
    settingsPtr = Gio::Settings::create("com.github.wwmm.pulseeffects");
  } else {
    settingsPtr = thisPtr->settings;
  }

  const auto* blocklist_preset_type = (preset_type == PresetType::output) ? "blocklist-out" : "blocklist-in";

  std::vector<Glib::ustring> bl = settingsPtr->get_string_array(blocklist_preset_type);

  // Check if the entry is already added
  for (const auto& str : bl) {
    if (name == str) {
      util::debug("blocklist_settings_ui: entry already present in the list");
      return false;
    }
  }

  bl.emplace_back(name);

  settingsPtr->set_string_array(blocklist_preset_type, bl);

  switch (preset_type) {
    case PresetType::output:
      populate_blocklist_out_listbox();
      break;
    case PresetType::input:
      populate_blocklist_in_listbox();
      break;
  }

  util::debug("blocklist_settings_ui: new entry has been added to the blocklist");
  return true;
}

void BlocklistSettingsUi::remove_entry(const Glib::ustring& name, PresetType preset_type) {
  Glib::RefPtr<Gio::Settings> settingsPtr;

  if (thisPtr == nullptr) {
    settingsPtr = Gio::Settings::create("com.github.wwmm.pulseeffects");
  } else {
    settingsPtr = thisPtr->settings;
  }

  const auto* blocklist_preset_type = (preset_type == PresetType::output) ? "blocklist-out" : "blocklist-in";

  std::vector<Glib::ustring> bl = settingsPtr->get_string_array(blocklist_preset_type);

  bl.erase(std::remove_if(bl.begin(), bl.end(), [=](auto& a) { return a == name; }), bl.end());

  settingsPtr->set_string_array(blocklist_preset_type, bl);

  switch (preset_type) {
    case PresetType::output:
      populate_blocklist_out_listbox();
      break;
    case PresetType::input:
      populate_blocklist_in_listbox();
      break;
  }

  util::debug("blocklist_settings_ui: an entry has been removed from the blocklist");
}

auto BlocklistSettingsUi::app_is_blocklisted(const Glib::ustring& name, PresetType preset_type) -> bool {
  Glib::RefPtr<Gio::Settings> settingsPtr;

  if (thisPtr == nullptr) {
    settingsPtr = Gio::Settings::create("com.github.wwmm.pulseeffects");
  } else {
    settingsPtr = thisPtr->settings;
  }

  const auto* blocklist_preset_type = (preset_type == PresetType::output) ? "blocklist-out" : "blocklist-in";

  std::vector<Glib::ustring> bl = settingsPtr->get_string_array(blocklist_preset_type);

  return std::find(std::begin(bl), std::end(bl), name) != std::end(bl);
}

void BlocklistSettingsUi::add_to_stack(Gtk::Stack* stack) {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/blocklist_settings.glade");

  auto* ui = Gtk::Builder::get_widget_derived<BlocklistSettingsUi>(builder, "widgets_grid");

  stack->add(*ui, "settings_blocklist", _("Blocklist"));
}

void BlocklistSettingsUi::populate_blocklist_in_listbox() {
  Glib::RefPtr<Gio::Settings> settingsPtr;

  if (thisPtr == nullptr) {
    settingsPtr = Gio::Settings::create("com.github.wwmm.pulseeffects");
  } else {
    settingsPtr = thisPtr->settings;
  }

  // auto children = blocklist_in_listbox->get_children();

  // for (const auto& c : children) {
  //   blocklist_in_listbox->remove(*c);
  // }

  // std::vector<std::string> names = settingsPtr->get_string_array("blocklist-in");

  // for (const auto& name : names) {
  //   auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/blocklist_row.glade");

  //   Gtk::ListBoxRow* row = nullptr;
  //   Gtk::Button* remove_btn = nullptr;
  //   Gtk::Label* label = nullptr;

  //   b->get_widget("blocklist_row", row);
  //   b->get_widget("remove", remove_btn);
  //   b->get_widget("name", label);

  //   row->set_name(name);
  //   label->set_text(name);

  //   connections.emplace_back(remove_btn->signal_clicked().connect([=]() { remove_entry(name, PresetType::input); }));

  //   blocklist_in_listbox->add(*row);
  //   blocklist_in_listbox->show_all();
  // }
}

void BlocklistSettingsUi::populate_blocklist_out_listbox() {
  Glib::RefPtr<Gio::Settings> settingsPtr;

  if (thisPtr == nullptr) {
    settingsPtr = Gio::Settings::create("com.github.wwmm.pulseeffects");
  } else {
    settingsPtr = thisPtr->settings;
  }

  // auto children = blocklist_out_listbox->get_children();

  // for (const auto& c : children) {
  //   blocklist_out_listbox->remove(*c);
  // }

  // std::vector<std::string> names = settingsPtr->get_string_array("blocklist-out");

  // for (const auto& name : names) {
  //   auto b = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/blocklist_row.glade");

  //   Gtk::ListBoxRow* row;
  //   Gtk::Button* remove_btn;
  //   Gtk::Label* label;

  //   b->get_widget("blocklist_row", row);
  //   b->get_widget("remove", remove_btn);
  //   b->get_widget("name", label);

  //   row->set_name(name);
  //   label->set_text(name);

  //   connections.emplace_back(remove_btn->signal_clicked().connect([=]() { remove_entry(name, PresetType::output);
  //   }));

  //   blocklist_out_listbox->add(*row);
  //   blocklist_out_listbox->show_all();
  // }
}
