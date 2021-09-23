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

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <giomm.h>
#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <memory>
#include "config.h"
#include "pipe_manager.hpp"
#include "presets_manager.hpp"
#include "stream_input_effects.hpp"
#include "stream_output_effects.hpp"

class Application : public Gtk::Application {
 public:
  Application();
  Application(const Application&) = delete;
  auto operator=(const Application&) -> Application& = delete;
  Application(const Application&&) = delete;
  auto operator=(const Application&&) -> Application& = delete;
  ~Application() override;

  static auto create() -> Glib::RefPtr<Application>;
  Glib::RefPtr<Gio::Settings> settings;
  Glib::RefPtr<Gio::Settings> soe_settings;
  Glib::RefPtr<Gio::Settings> sie_settings;

  std::unique_ptr<PipeManager> pm;
  std::unique_ptr<StreamOutputEffects> soe;
  std::unique_ptr<StreamInputEffects> sie;
  std::unique_ptr<PresetsManager> presets_manager;

 protected:
  auto on_command_line(const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) -> int override;

  auto on_handle_local_options(const Glib::RefPtr<Glib::VariantDict>& options) -> int override;

  void on_startup() override;
  void on_activate() override;

 private:
  inline static const std::string log_tag = "application: ";

  bool running_as_service = false;

  void create_actions();
  void update_bypass_state(const Glib::ustring& key);
};

#endif
