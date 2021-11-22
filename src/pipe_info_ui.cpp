/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "pipe_info_ui.hpp"

PipeInfoUi::PipeInfoUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       PipeManager* pm_ptr,
                       PresetsManager* presets_manager)
    : Gtk::Box(cobject),
      pm(pm_ptr),
      presets_manager(presets_manager),
      ts(std::make_unique<TestSignals>(pm_ptr)),
      sie_settings(Gio::Settings::create("com.github.wwmm.easyeffects.streaminputs")),
      soe_settings(Gio::Settings::create("com.github.wwmm.easyeffects.streamoutputs")),
      input_devices_model(Gio::ListStore<NodeInfoHolder>::create()),
      output_devices_model(Gio::ListStore<NodeInfoHolder>::create()),
      autoloading_output_model(Gio::ListStore<PresetsAutoloadingHolder>::create()),
      autoloading_input_model(Gio::ListStore<PresetsAutoloadingHolder>::create()),
      output_presets_string_list(Gtk::StringList::create({"initial_value"})),
      input_presets_string_list(Gtk::StringList::create({"initial_value"})) {
  setup_dropdown_devices(dropdown_input_devices, input_devices_model);
  setup_dropdown_devices(dropdown_output_devices, output_devices_model);

  setup_dropdown_devices(dropdown_autoloading_input_devices, input_devices_model);
  setup_dropdown_devices(dropdown_autoloading_output_devices, output_devices_model);

  setup_dropdown_presets(PresetType::input, input_presets_string_list);
  setup_dropdown_presets(PresetType::output, output_presets_string_list);

  dropdown_input_devices->property_selected_item().signal_changed().connect([=, this]() {
    if (dropdown_input_devices->get_selected_item() == nullptr) {
      return;
    }

    auto holder = std::dynamic_pointer_cast<NodeInfoHolder>(dropdown_input_devices->get_selected_item());

    sie_settings->set_string("input-device", holder->name);
  });

  dropdown_output_devices->property_selected_item().signal_changed().connect([=, this]() {
    if (dropdown_output_devices->get_selected_item() == nullptr) {
      return;
    }

    auto holder = std::dynamic_pointer_cast<NodeInfoHolder>(dropdown_output_devices->get_selected_item());

    soe_settings->set_string("output-device", holder->name);
  });

  // setting the displayed entry to the right value

  {
    auto holder_selected = std::dynamic_pointer_cast<NodeInfoHolder>(dropdown_input_devices->get_selected_item());

    if (holder_selected != nullptr) {
      const auto input_device_name = sie_settings->get_string("input-device").raw();

      if (holder_selected->name != input_device_name) {
        for (guint n = 0U; n < input_devices_model->get_n_items(); n++) {
          if (input_devices_model->get_item(n)->name == input_device_name) {
            dropdown_input_devices->set_selected(n);
          }
        }
      }
    }
  }

  {
    auto holder_selected = std::dynamic_pointer_cast<NodeInfoHolder>(dropdown_output_devices->get_selected_item());

    if (holder_selected != nullptr) {
      const auto output_device_name = soe_settings->get_string("output-device").raw();

      if (holder_selected->name != output_device_name) {
        for (guint n = 0U; n < output_devices_model->get_n_items(); n++) {
          if (output_devices_model->get_item(n)->name == output_device_name) {
            dropdown_output_devices->set_selected(n);
          }
        }
      }
    }
  }

  // use_default_input->property_active().signal_changed().connect([=, this]() {
  //   if (use_default_input->get_active()) {
  //     sie_settings->set_string("input-device", pm->default_input_device.name);

  //     auto holder = std::dynamic_pointer_cast<NodeInfoHolder>(dropdown_input_devices->get_selected_item());

  //     if (holder != nullptr) {
  //       if (holder->name != pm->default_input_device.name) {
  //         for (guint n = 0U; n < input_devices_model->get_n_items(); n++) {
  //           if (input_devices_model->get_item(n)->name == pm->default_input_device.name) {
  //             dropdown_input_devices->set_selected(n);

  //             break;
  //           }
  //         }
  //       }
  //     }
  //   }
  // });

  // use_default_output->property_active().signal_changed().connect([=, this]() {
  //   if (use_default_output->get_active()) {
  //     soe_settings->set_string("output-device", pm->default_output_device.name);

  //     auto holder_selected = std::dynamic_pointer_cast<NodeInfoHolder>(dropdown_output_devices->get_selected_item());

  //     if (holder_selected != nullptr) {
  //       if (holder_selected->name != pm->default_output_device.name) {
  //         for (guint n = 0U; n < output_devices_model->get_n_items(); n++) {
  //           if (output_devices_model->get_item(n)->name == pm->default_output_device.name) {
  //             dropdown_output_devices->set_selected(n);

  //             break;
  //           }
  //         }
  //       }
  //     }
  //   }
  // });

  autoloading_add_output_profile->signal_clicked().connect([=, this]() {
    if (dropdown_autoloading_output_devices->get_selected_item() == nullptr) {
      return;
    }

    auto holder = std::dynamic_pointer_cast<NodeInfoHolder>(dropdown_autoloading_output_devices->get_selected_item());

    std::string device_profile;

    for (const auto& device : pm->list_devices) {
      if (device.id == holder->device_id) {
        device_profile = device.output_route_name;

        break;
      }
    }

    // first we remove any autoloading profile associated to the target device so that our ui is updated

    for (guint n = 0; n < autoloading_output_model->get_n_items(); n++) {
      const auto item = autoloading_output_model->get_item(n);

      if (holder->name == item->device && device_profile == item->device_profile) {
        presets_manager->remove_autoload(PresetType::output, item->preset_name, item->device, item->device_profile);

        break;
      }
    }

    const auto preset_name =
        dropdown_autoloading_output_presets->get_selected_item()->get_property<Glib::ustring>("string").raw();

    presets_manager->add_autoload(PresetType::output, preset_name, holder->name, device_profile);
  });

  autoloading_add_input_profile->signal_clicked().connect([=, this]() {
    if (dropdown_autoloading_input_devices->get_selected_item() == nullptr) {
      return;
    }

    auto holder = std::dynamic_pointer_cast<NodeInfoHolder>(dropdown_autoloading_input_devices->get_selected_item());

    std::string device_profile;

    for (const auto& device : pm->list_devices) {
      if (device.id == holder->device_id) {
        device_profile = device.input_route_name;

        break;
      }
    }

    // first we remove any autoloading profile associated to the target device so that our ui is updated

    for (guint n = 0; n < autoloading_input_model->get_n_items(); n++) {
      const auto item = autoloading_input_model->get_item(n);

      if (holder->name == item->device && device_profile == item->device_profile) {
        presets_manager->remove_autoload(PresetType::input, item->preset_name, item->device, item->device_profile);

        break;
      }
    }

    const auto preset_name =
        dropdown_autoloading_input_presets->get_selected_item()->get_property<Glib::ustring>("string").raw();

    presets_manager->add_autoload(PresetType::input, preset_name, holder->name, device_profile);
  });
}

PipeInfoUi::~PipeInfoUi() {
  for (auto& c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

auto PipeInfoUi::add_to_stack(Gtk::Stack* stack, PipeManager* pm, PresetsManager* presets_manager) -> PipeInfoUi* {
  const auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/pipe_info.ui");

  auto* const ui = Gtk::Builder::get_widget_derived<PipeInfoUi>(builder, "top_box", pm, presets_manager);

  stack->add(*ui, "pipe_info");

  return ui;
}

void PipeInfoUi::setup_dropdown_devices(Gtk::DropDown* dropdown,
                                        const Glib::RefPtr<Gio::ListStore<NodeInfoHolder>>& model) {
  // setting the dropdown model and factory

  const auto selection_model = Gtk::SingleSelection::create(model);

  dropdown->set_model(selection_model);

  auto factory = Gtk::SignalListItemFactory::create();

  dropdown->set_factory(factory);

  // setting the factory callbacks

  factory->signal_setup().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const box = Gtk::make_managed<Gtk::Box>();
    auto* const label = Gtk::make_managed<Gtk::Label>();
    auto* const icon = Gtk::make_managed<Gtk::Image>();

    label->set_hexpand(true);
    label->set_halign(Gtk::Align::START);

    icon->set_from_icon_name("emblem-system-symbolic");

    box->set_spacing(6);
    box->append(*icon);
    box->append(*label);

    // setting list_item data

    list_item->set_data("name", label);
    list_item->set_data("icon", icon);

    list_item->set_child(*box);
  });

  factory->signal_bind().connect([=, this](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const label = static_cast<Gtk::Label*>(list_item->get_data("name"));
    auto* const icon = static_cast<Gtk::Image*>(list_item->get_data("icon"));

    auto holder = std::dynamic_pointer_cast<NodeInfoHolder>(list_item->get_item());

    if (holder->media_class == pm->media_class_sink) {
      icon->set_from_icon_name("audio-card-symbolic");
    } else if (holder->media_class == pm->media_class_source) {
      icon->set_from_icon_name("audio-input-microphone-symbolic");
    }

    label->set_name(holder->name);
    label->set_text(holder->name);
  });
}

void PipeInfoUi::setup_dropdown_presets(PresetType preset_type, const Glib::RefPtr<Gtk::StringList>& string_list) {
  Gtk::DropDown* dropdown = nullptr;

  switch (preset_type) {
    case PresetType::input:
      dropdown = dropdown_autoloading_input_presets;

      break;
    case PresetType::output:
      dropdown = dropdown_autoloading_output_presets;

      break;
  }

  string_list->remove(0);

  for (const auto& name : presets_manager->get_names(preset_type)) {
    string_list->append(name);
  }

  // sorter

  const auto sorter =
      Gtk::StringSorter::create(Gtk::PropertyExpression<Glib::ustring>::create(GTK_TYPE_STRING_OBJECT, "string"));

  const auto sort_list_model = Gtk::SortListModel::create(string_list, sorter);

  // setting the dropdown model and factory

  const auto selection_model = Gtk::SingleSelection::create(sort_list_model);

  dropdown->set_model(selection_model);

  auto factory = Gtk::SignalListItemFactory::create();

  dropdown->set_factory(factory);

  // setting the factory callbacks

  factory->signal_setup().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const box = Gtk::make_managed<Gtk::Box>();
    auto* const label = Gtk::make_managed<Gtk::Label>();
    auto* const icon = Gtk::make_managed<Gtk::Image>();

    label->set_hexpand(true);
    label->set_halign(Gtk::Align::START);

    icon->set_from_icon_name("emblem-system-symbolic");

    box->set_spacing(6);
    box->append(*icon);
    box->append(*label);

    // setting list_item data

    list_item->set_data("name", label);

    list_item->set_child(*box);
  });

  factory->signal_bind().connect([=](const Glib::RefPtr<Gtk::ListItem>& list_item) {
    auto* const label = static_cast<Gtk::Label*>(list_item->get_data("name"));

    const auto name = list_item->get_item()->get_property<Glib::ustring>("string");

    label->set_name(name);
    label->set_text(name);
  });
}
