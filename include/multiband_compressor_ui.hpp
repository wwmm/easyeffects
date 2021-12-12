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

#pragma once

#include <adwaita.h>
#include "effects_base.hpp"
#include "node_info_holder.hpp"
#include "ui_helpers.hpp"

#pragma once

#include <adwaita.h>
#include "effects_base.hpp"
#include "ui_helpers.hpp"

namespace ui::multiband_compressor_box {

G_BEGIN_DECLS

#define EE_TYPE_MULTIBAND_COMPRESSOR_BOX (multiband_compressor_box_get_type())

G_DECLARE_FINAL_TYPE(MultibandCompressorBox, multiband_compressor_box, EE, MULTIBAND_COMPRESSOR_BOX, GtkBox)

G_END_DECLS

auto create() -> MultibandCompressorBox*;

void setup(MultibandCompressorBox* self,
           std::shared_ptr<MultibandCompressor> multiband_compressor,
           const std::string& schema_path,
           PipeManager* pm);

}  // namespace ui::multiband_compressor_box

#include "info_holders.hpp"
#include "plugin_ui_base.hpp"

class MultibandCompressorUi : public Gtk::Box, public PluginUiBase {
 public:
  MultibandCompressorUi(BaseObjectType* cobject,
                        const Glib::RefPtr<Gtk::Builder>& builder,
                        const std::string& schema,
                        const std::string& schema_path);
  MultibandCompressorUi(const MultibandCompressorUi&) = delete;
  auto operator=(const MultibandCompressorUi&) -> MultibandCompressorUi& = delete;
  MultibandCompressorUi(const MultibandCompressorUi&&) = delete;
  auto operator=(const MultibandCompressorUi&&) -> MultibandCompressorUi& = delete;
  ~MultibandCompressorUi() override;

  static constexpr uint n_bands = 8U;

  static auto add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> MultibandCompressorUi*;

  void on_new_frequency_range(const std::array<float, n_bands>& values);

  void on_new_envelope(const std::array<float, n_bands>& values);

  void on_new_curve(const std::array<float, n_bands>& values);

  void on_new_reduction(const std::array<float, n_bands>& values);

 private:
  Gtk::ComboBoxText *compressor_mode = nullptr, *envelope_boost = nullptr;

  Gtk::Stack* stack = nullptr;

  Gtk::ListBox* listbox = nullptr;

  std::array<Gtk::Label*, n_bands> bands_end = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

  std::array<Gtk::Label*, n_bands> bands_gain_label = {nullptr, nullptr, nullptr, nullptr,
                                                       nullptr, nullptr, nullptr, nullptr};

  std::array<Gtk::Label*, n_bands> bands_envelope_label = {nullptr, nullptr, nullptr, nullptr,
                                                           nullptr, nullptr, nullptr, nullptr};

  std::array<Gtk::Label*, n_bands> bands_curve_label = {nullptr, nullptr, nullptr, nullptr,
                                                        nullptr, nullptr, nullptr, nullptr};

  Gtk::DropDown* dropdown_input_devices = nullptr;

  Glib::RefPtr<Gio::ListStore<NodeInfoHolder>> input_devices_model;

  PipeManager* pm = nullptr;

  void prepare_bands();

  void set_dropdown_input_devices_sensitivity();
};
