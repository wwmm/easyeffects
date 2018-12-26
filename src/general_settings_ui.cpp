#include "general_settings_ui.hpp"
#include <giomm/file.h>
#include <glibmm.h>
#include <boost/filesystem.hpp>
#include "util.hpp"

namespace {

gboolean priority_type_enum_to_int(GValue* value,
                                   GVariant* variant,
                                   gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("Niceness")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("Real Time")) {
    g_value_set_int(value, 1);
  } else if (v == std::string("None")) {
    g_value_set_int(value, 2);
  }

  return true;
}

GVariant* int_to_priority_type_enum(const GValue* value,
                                    const GVariantType* expected_type,
                                    gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("Niceness");
  } else if (v == 1) {
    return g_variant_new_string("Real Time");
  } else {
    return g_variant_new_string("None");
  }
}

}  // namespace

GeneralSettingsUi::GeneralSettingsUi(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& builder,
    const Glib::RefPtr<Gio::Settings>& refSettings,
    Application* application)
    : Gtk::Grid(cobject), settings(refSettings), app(application) {
  // loading glade widgets

  builder->get_widget("theme_switch", theme_switch);
  builder->get_widget("enable_autostart", enable_autostart);
  builder->get_widget("enable_all_apps", enable_all_apps);
  builder->get_widget("reset_settings", reset_settings);
  builder->get_widget("about_button", about_button);
  builder->get_widget("enable_realtime", enable_realtime);
  builder->get_widget("enable_high_priority", enable_high_priority);
  builder->get_widget("realtime_priority", realtime_priority);
  builder->get_widget("niceness", niceness);
  builder->get_widget("priority_type", priority_type);

  get_object(builder, "adjustment_priority", adjustment_priority);
  get_object(builder, "adjustment_niceness", adjustment_niceness);

  // signals connection

  enable_autostart->signal_state_set().connect(
      sigc::mem_fun(*this, &GeneralSettingsUi::on_enable_autostart), false);

  reset_settings->signal_clicked().connect(
      sigc::mem_fun(*this, &GeneralSettingsUi::on_reset_settings));

  about_button->signal_clicked().connect(
      [=]() { app->activate_action("about"); });

  connections.push_back(
      settings->signal_changed("priority-type").connect([&](auto key) {
        app->sie->set_null_pipeline();
        app->soe->set_null_pipeline();

        app->sie->update_pipeline_state();
        app->soe->update_pipeline_state();
      }));

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("use-dark-theme", theme_switch, "active", flag);
  settings->bind("enable-all-apps", enable_all_apps, "active", flag);
  settings->bind("enable-realtime", enable_realtime, "active", flag);
  settings->bind("enable-high-priority", enable_high_priority, "active", flag);
  settings->bind("realtime-priority", adjustment_priority.get(), "value", flag);
  settings->bind("niceness", adjustment_niceness.get(), "value", flag);
  settings->bind("enable-realtime", realtime_priority, "sensitive", flag);
  settings->bind("enable-high-priority", niceness, "sensitive", flag);

  g_settings_bind_with_mapping(
      settings->gobj(), "priority-type", priority_type->gobj(), "active",
      G_SETTINGS_BIND_DEFAULT, priority_type_enum_to_int,
      int_to_priority_type_enum, nullptr, nullptr);

  init_autostart_switch();
}

GeneralSettingsUi::~GeneralSettingsUi() {
  for (auto c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

void GeneralSettingsUi::add_to_stack(Gtk::Stack* stack, Application* app) {
  auto builder = Gtk::Builder::create_from_resource(
      "/com/github/wwmm/pulseeffects/ui/general_settings.glade");

  auto settings = Gio::Settings::create("com.github.wwmm.pulseeffects");

  GeneralSettingsUi* ui;

  builder->get_widget_derived("widgets_grid", ui, settings, app);

  stack->add(*ui, "general_spectrum", _("General"));
}

void GeneralSettingsUi::init_autostart_switch() {
  auto path =
      Glib::get_user_config_dir() + "/autostart/pulseeffects-service.desktop";

  try {
    auto file = Gio::File::create_for_path(path);

    if (file->query_exists()) {
      enable_autostart->set_active(true);
    } else {
      enable_autostart->set_active(false);
    }
  } catch (const Glib::Exception& ex) {
    util::warning(log_tag + ex.what());
  }
}

bool GeneralSettingsUi::on_enable_autostart(bool state) {
  boost::filesystem::path autostart_dir{Glib::get_user_config_dir() +
                                        "/autostart"};

  if (!boost::filesystem::is_directory(autostart_dir)) {
    boost::filesystem::create_directories(autostart_dir);
  }

  boost::filesystem::path autostart_file{
      Glib::get_user_config_dir() + "/autostart/pulseeffects-service.desktop"};

  if (state) {
    if (!boost::filesystem::exists(autostart_file)) {
      boost::filesystem::ofstream ofs{autostart_file};

      ofs << "[Desktop Entry]\n";
      ofs << "Name=PulseEffects\n";
      ofs << "Comment=PulseEffects Service\n";
      ofs << "Exec=pulseeffects --gapplication-service\n";
      ofs << "Icon=pulseeffects\n";
      ofs << "StartupNotify=false\n";
      ofs << "Terminal=false\n";
      ofs << "Type=Application\n";

      ofs.close();

      util::debug(log_tag + "autostart file created");
    }
  } else {
    if (boost::filesystem::exists(autostart_file)) {
      boost::filesystem::remove(autostart_file);

      util::debug(log_tag + "autostart file removed");
    }
  }

  return false;
}

void GeneralSettingsUi::on_reset_settings() {
  settings->reset("");
}
