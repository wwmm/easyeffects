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
