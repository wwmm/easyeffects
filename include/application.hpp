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

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <giomm/settings.h>
#include <gtkmm/application.h>
#include <memory>
#include "pipe_manager.hpp"
#include "presets_manager.hpp"
#include "sink_input_effects.hpp"
#include "source_output_effects.hpp"

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

  std::unique_ptr<PipeManager> pm;
  std::unique_ptr<SinkInputEffects> sie;
  std::unique_ptr<SourceOutputEffects> soe;
  std::unique_ptr<PresetsManager> presets_manager;

 protected:
  auto on_command_line(const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) -> int override;

  auto on_handle_local_options(const Glib::RefPtr<Glib::VariantDict>& options) -> int;

  void on_startup() override;
  void on_activate() override;

 private:
  std::string log_tag = "application: ";
  std::string last_sink_dev_name, last_source_dev_name;

  bool running_as_service = false;

  void create_actions();
  void update_bypass_state(const std::string& key);
};

#endif
