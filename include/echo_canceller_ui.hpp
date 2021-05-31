/*
 *  Copyright © 2017-2020 Wellington Wallace
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

#ifndef ECHO_CANCELLER_UI_HPP
#define ECHO_CANCELLER_UI_HPP

#include "plugin_ui_base.hpp"

class EchoCancellerUi : public Gtk::Box, public PluginUiBase {
 public:
  EchoCancellerUi(BaseObjectType* cobject,
                  const Glib::RefPtr<Gtk::Builder>& builder,
                  const std::string& schema,
                  const std::string& schema_path);
  EchoCancellerUi(const EchoCancellerUi&) = delete;
  auto operator=(const EchoCancellerUi&) -> EchoCancellerUi& = delete;
  EchoCancellerUi(const EchoCancellerUi&&) = delete;
  auto operator=(const EchoCancellerUi&&) -> EchoCancellerUi& = delete;
  ~EchoCancellerUi() override;

  static auto add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> EchoCancellerUi*;

  void reset() override;

 private:
  Gtk::SpinButton *fcut = nullptr, *feed = nullptr;

  Gtk::Scale *input_gain = nullptr, *output_gain = nullptr;
};

#endif
