#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <giomm/settings.h>
#include <gtkmm/application.h>
#include <memory>
#include "presets_manager.hpp"
#include "pulse_manager.hpp"
#include "sink_input_effects.hpp"
#include "source_output_effects.hpp"

class Application : public Gtk::Application {
 public:
  Application();
  ~Application();

  static Glib::RefPtr<Application> create();
  Glib::RefPtr<Gio::Settings> settings;

  std::unique_ptr<PulseManager> pm;
  std::unique_ptr<SinkInputEffects> sie;
  std::unique_ptr<SourceOutputEffects> soe;
  std::unique_ptr<PresetsManager> presets_manager;

 protected:
  int on_command_line(const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) override;

  int on_handle_local_options(const Glib::RefPtr<Glib::VariantDict>& options);

  void on_startup() override;
  void on_activate() override;

 private:
  std::string log_tag = "application: ";
  std::string last_sink_dev_name, last_source_dev_name;

  bool running_as_service = false;

  void create_actions();
};

#endif
