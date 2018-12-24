#include "application_ui.hpp"
#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/listboxrow.h>
#include <gtkmm/settings.h>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <future>
#include "util.hpp"

namespace {

gboolean blocksize_enum_to_int(GValue* value,
                               GVariant* variant,
                               gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("64")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("128")) {
    g_value_set_int(value, 1);
  } else if (v == std::string("256")) {
    g_value_set_int(value, 2);
  } else if (v == std::string("512")) {
    g_value_set_int(value, 3);
  } else if (v == std::string("1024")) {
    g_value_set_int(value, 4);
  } else if (v == std::string("2048")) {
    g_value_set_int(value, 5);
  } else if (v == std::string("4096")) {
    g_value_set_int(value, 6);
  }

  return true;
}

GVariant* int_to_blocksize_enum(const GValue* value,
                                const GVariantType* expected_type,
                                gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("64");
  } else if (v == 1) {
    return g_variant_new_string("128");
  } else if (v == 2) {
    return g_variant_new_string("256");
  } else if (v == 3) {
    return g_variant_new_string("512");
  } else if (v == 4) {
    return g_variant_new_string("1024");
  } else if (v == 5) {
    return g_variant_new_string("2048");
  } else {
    return g_variant_new_string("4096");
  }
}

}  // namespace

ApplicationUi::ApplicationUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& builder,
                             Application* application)
    : Gtk::ApplicationWindow(cobject),
      app(application),
      settings(app->settings) {
  apply_css_style("listbox.css");

  Gtk::IconTheme::get_default()->add_resource_path(
      "/com/github/wwmm/pulseeffects/icons");

  // loading glade widgets

  builder->get_widget("theme_switch", theme_switch);
  builder->get_widget("enable_autostart", enable_autostart);
  builder->get_widget("enable_all_apps", enable_all_apps);
  builder->get_widget("use_default_sink", use_default_sink);
  builder->get_widget("use_default_source", use_default_source);
  builder->get_widget("input_device", input_device);
  builder->get_widget("output_device", output_device);
  builder->get_widget("reset_settings", reset_settings);
  builder->get_widget("placeholder_spectrum", placeholder_spectrum);
  builder->get_widget("stack", stack);
  builder->get_widget("stack_menu_settings", stack_menu_settings);
  builder->get_widget("presets_listbox", presets_listbox);
  builder->get_widget("presets_menu_button", presets_menu_button);
  builder->get_widget("presets_menu_label", presets_menu_label);
  builder->get_widget("presets_scrolled_window", presets_scrolled_window);
  builder->get_widget("preset_name", preset_name);
  builder->get_widget("add_preset", add_preset);
  builder->get_widget("import_preset", import_preset);
  builder->get_widget("calibration_button", calibration_button);
  builder->get_widget("blocksize_in", blocksize_in);
  builder->get_widget("blocksize_out", blocksize_out);
  builder->get_widget("headerbar", headerbar);
  builder->get_widget("help_button", help_button);
  builder->get_widget("headerbar_icon1", headerbar_icon1);
  builder->get_widget("headerbar_icon2", headerbar_icon2);
  builder->get_widget("headerbar_info", headerbar_info);
  builder->get_widget("blacklist_in_scrolled_window",
                      blacklist_in_scrolled_window);
  builder->get_widget("blacklist_out_scrolled_window",
                      blacklist_out_scrolled_window);
  builder->get_widget("add_blacklist_in", add_blacklist_in);
  builder->get_widget("add_blacklist_out", add_blacklist_out);
  builder->get_widget("blacklist_in_listbox", blacklist_in_listbox);
  builder->get_widget("blacklist_out_listbox", blacklist_out_listbox);
  builder->get_widget("blacklist_in_name", blacklist_in_name);
  builder->get_widget("blacklist_out_name", blacklist_out_name);
  builder->get_widget("about_button", about_button);

  get_object(builder, "buffer_in", buffer_in);
  get_object(builder, "buffer_out", buffer_out);
  get_object(builder, "latency_in", latency_in);
  get_object(builder, "latency_out", latency_out);
  get_object(builder, "sink_list", sink_list);
  get_object(builder, "source_list", source_list);

  // signals connection

  enable_autostart->signal_state_set().connect(
      sigc::mem_fun(*this, &ApplicationUi::on_enable_autostart), false);

  reset_settings->signal_clicked().connect(
      sigc::mem_fun(*this, &ApplicationUi::on_reset_settings));

  // pulseaudio device selection

  use_default_sink->signal_toggled().connect(
      sigc::mem_fun(*this, &ApplicationUi::on_use_default_sink_toggled));
  use_default_source->signal_toggled().connect(
      sigc::mem_fun(*this, &ApplicationUi::on_use_default_source_toggled));

  input_device->signal_changed().connect(
      sigc::mem_fun(*this, &ApplicationUi::on_input_device_changed));
  output_device->signal_changed().connect(
      sigc::mem_fun(*this, &ApplicationUi::on_output_device_changed));

  stack->connect_property_changed(
      "visible-child",
      sigc::mem_fun(*this, &ApplicationUi::on_stack_visible_child_changed));

  // presets widgets callbacks

  presets_menu_button->signal_clicked().connect(
      sigc::mem_fun(*this, &ApplicationUi::on_presets_menu_button_clicked));

  presets_listbox->set_sort_func(
      sigc::mem_fun(*this, &ApplicationUi::on_listbox_sort));

  presets_listbox->signal_row_activated().connect([=](auto row) {
    presets_menu_label->set_text(row->get_name());
    settings->set_string("last-used-preset", row->get_name());
    app->presets_manager->load(row->get_name());
  });

  add_preset->signal_clicked().connect([=]() {
    auto name = preset_name->get_text();
    if (!name.empty()) {
      std::string illegalChars = "\\/\0";

      for (auto it = name.begin(); it < name.end(); ++it) {
        bool found = illegalChars.find(*it) != std::string::npos;
        if (found) {
          preset_name->set_text("");
          return;
        }
      }

      app->presets_manager->add(name);
      preset_name->set_text("");
      populate_presets_listbox();
    }
  });

  import_preset->signal_clicked().connect(
      sigc::mem_fun(*this, &ApplicationUi::on_import_preset_clicked));

  // blacklist widgets callbacks

  blacklist_in_listbox->set_sort_func(
      sigc::mem_fun(*this, &ApplicationUi::on_listbox_sort));

  blacklist_out_listbox->set_sort_func(
      sigc::mem_fun(*this, &ApplicationUi::on_listbox_sort));

  add_blacklist_in->signal_clicked().connect([=]() {
    auto name = blacklist_in_name->get_text();

    if (!name.empty()) {
      std::vector<std::string> bl = settings->get_string_array("blacklist-in");
      bl.push_back(name);
      settings->set_string_array("blacklist-in", bl);
      blacklist_in_name->set_text("");
      populate_blacklist_in_listbox();
    }
  });

  add_blacklist_out->signal_clicked().connect([=]() {
    auto name = blacklist_out_name->get_text();
    if (!name.empty()) {
      std::vector<std::string> bl = settings->get_string_array("blacklist-out");
      bl.push_back(name);
      settings->set_string_array("blacklist-out", bl);
      blacklist_out_name->set_text("");
      populate_blacklist_out_listbox();
    }
  });

  populate_blacklist_in_listbox();
  populate_blacklist_out_listbox();

  // calibration

  calibration_button->signal_clicked().connect(
      sigc::mem_fun(*this, &ApplicationUi::on_calibration_button_clicked));

  // pulseaudio signals

  app->pm->sink_added.connect(
      sigc::mem_fun(*this, &ApplicationUi::on_sink_added));
  app->pm->sink_removed.connect(
      sigc::mem_fun(*this, &ApplicationUi::on_sink_removed));
  app->pm->source_added.connect(
      sigc::mem_fun(*this, &ApplicationUi::on_source_added));
  app->pm->source_removed.connect(
      sigc::mem_fun(*this, &ApplicationUi::on_source_removed));

  connections.push_back(app->pm->new_default_sink.connect([&](auto name) {
    if (stack->get_visible_child_name() == "sink_inputs") {
      update_headerbar_subtitle(0);
    }
  }));

  connections.push_back(app->pm->new_default_source.connect([&](auto name) {
    if (stack->get_visible_child_name() == "source_outputs") {
      update_headerbar_subtitle(1);
    }
  }));

  // help button

  help_button->signal_clicked().connect(
      [=]() { app->activate_action("help"); });

  // about button

  about_button->signal_clicked().connect(
      [=]() { app->activate_action("about"); });

  // spectrum interface

  auto b_spectrum = Gtk::Builder::create_from_resource(
      "/com/github/wwmm/pulseeffects/ui/spectrum.glade");

  b_spectrum->get_widget_derived("widgets_grid", spectrum_ui, settings, app);

  placeholder_spectrum->add(*spectrum_ui);

  // spectrum settings interface

  auto b_spectrum_settings = Gtk::Builder::create_from_resource(
      "/com/github/wwmm/pulseeffects/ui/spectrum_settings.glade");

  b_spectrum_settings->get_widget_derived("widgets_grid", spectrum_settings_ui,
                                          settings, app);

  stack_menu_settings->add(*spectrum_settings_ui, "settings_spectrum",
                           _("Spectrum"));

  // sink inputs interface

  auto b_sie_ui = Gtk::Builder::create_from_resource(
      "/com/github/wwmm/pulseeffects/ui/effects_base.glade");

  auto settings_sie_ui =
      Gio::Settings::create("com.github.wwmm.pulseeffects.sinkinputs");

  b_sie_ui->get_widget_derived("widgets_box", sie_ui, settings_sie_ui,
                               app->sie.get());

  app->pm->sink_input_added.connect(
      sigc::mem_fun(*sie_ui, &SinkInputEffectsUi::on_app_added));
  app->pm->sink_input_changed.connect(
      sigc::mem_fun(*sie_ui, &SinkInputEffectsUi::on_app_changed));
  app->pm->sink_input_removed.connect(
      sigc::mem_fun(*sie_ui, &SinkInputEffectsUi::on_app_removed));

  stack->add(*sie_ui, "sink_inputs");
  stack->child_property_icon_name(*sie_ui).set_value("audio-speakers-symbolic");

  connections.push_back(app->sie->new_latency.connect([=](int latency) {
    sie_latency = latency;

    if (stack->get_visible_child_name() == "sink_inputs") {
      update_headerbar_subtitle(0);
    }
  }));

  if (app->sie->playing) {
    app->sie->get_latency();
  }

  // source outputs interface

  auto b_soe_ui = Gtk::Builder::create_from_resource(
      "/com/github/wwmm/pulseeffects/ui/effects_base.glade");

  auto settings_soe_ui =
      Gio::Settings::create("com.github.wwmm.pulseeffects.sourceoutputs");

  b_soe_ui->get_widget_derived("widgets_box", soe_ui, settings_soe_ui,
                               app->soe.get());

  app->pm->source_output_added.connect(
      sigc::mem_fun(*soe_ui, &SourceOutputEffectsUi::on_app_added));
  app->pm->source_output_changed.connect(
      sigc::mem_fun(*soe_ui, &SourceOutputEffectsUi::on_app_changed));
  app->pm->source_output_removed.connect(
      sigc::mem_fun(*soe_ui, &SourceOutputEffectsUi::on_app_removed));

  stack->add(*soe_ui, "source_outputs");
  stack->child_property_icon_name(*soe_ui).set_value(
      "audio-input-microphone-symbolic");

  connections.push_back(app->soe->new_latency.connect([=](int latency) {
    soe_latency = latency;

    if (stack->get_visible_child_name() == "source_outputs") {
      update_headerbar_subtitle(1);
    }
  }));

  if (app->soe->playing) {
    app->soe->get_latency();
  }

  // temporary spectrum connection. it changes with the selected stack child

  spectrum_connection = app->sie->new_spectrum.connect(
      sigc::mem_fun(*spectrum_ui, &SpectrumUi::on_new_spectrum));

  // updating headerbar info

  update_headerbar_subtitle(0);

  // binding glade widgets to gsettings keys

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;
  auto flag_invert_boolean =
      Gio::SettingsBindFlags::SETTINGS_BIND_INVERT_BOOLEAN;

  settings->bind("use-dark-theme", theme_switch, "active", flag);

  settings->bind("use-dark-theme", Gtk::Settings::get_default().get(),
                 "gtk_application_prefer_dark_theme", flag);

  settings->bind("enable-all-apps", enable_all_apps, "active", flag);

  settings->bind("use-default-sink", use_default_sink, "active", flag);

  settings->bind("use-default-sink", output_device, "sensitive",
                 flag | flag_invert_boolean);

  settings->bind("use-default-source", use_default_source, "active", flag);

  settings->bind("use-default-source", input_device, "sensitive",
                 flag | flag_invert_boolean);

  settings->bind("buffer-out", buffer_out.get(), "value", flag);
  settings->bind("latency-out", latency_out.get(), "value", flag);

  settings->bind("buffer-in", buffer_in.get(), "value", flag);
  settings->bind("latency-in", latency_in.get(), "value", flag);

  settings->bind("last-used-preset", presets_menu_label, "label", flag);

  g_settings_bind_with_mapping(settings->gobj(), "blocksize-in",
                               blocksize_in->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, blocksize_enum_to_int,
                               int_to_blocksize_enum, nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "blocksize-out",
                               blocksize_out->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, blocksize_enum_to_int,
                               int_to_blocksize_enum, nullptr, nullptr);

  init_autostart_switch();
}

ApplicationUi::~ApplicationUi() {
  for (auto c : connections) {
    c.disconnect();
  }

  spectrum_connection.disconnect();

  util::debug(log_tag + "destroyed");
}

ApplicationUi* ApplicationUi::create(Application* app_this) {
  auto builder = Gtk::Builder::create_from_resource(
      "/com/github/wwmm/pulseeffects/ui/application.glade");

  ApplicationUi* window = nullptr;

  builder->get_widget_derived("ApplicationUi", window, app_this);

  return window;
}

void ApplicationUi::apply_css_style(std::string css_file_name) {
  auto provider = Gtk::CssProvider::create();

  provider->load_from_resource("/com/github/wwmm/pulseeffects/ui/" +
                               css_file_name);

  auto screen = Gdk::Screen::get_default();
  auto priority = GTK_STYLE_PROVIDER_PRIORITY_APPLICATION;

  Gtk::StyleContext::add_provider_for_screen(screen, provider, priority);
}

void ApplicationUi::init_autostart_switch() {
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

bool ApplicationUi::on_enable_autostart(bool state) {
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

void ApplicationUi::on_reset_settings() {
  settings->reset("");
}

void ApplicationUi::update_headerbar_subtitle(const int& index) {
  std::ostringstream null_sink_rate, current_dev_rate;

  null_sink_rate.precision(1);
  current_dev_rate.precision(1);

  if (index == 0) {  // sie
    headerbar_icon1->set_from_icon_name("emblem-music-symbolic",
                                        Gtk::ICON_SIZE_MENU);

    headerbar_icon2->set_from_icon_name("audio-speakers-symbolic",
                                        Gtk::ICON_SIZE_MENU);

    null_sink_rate << std::fixed << app->pm->apps_sink_info->rate / 1000.0f
                   << "kHz";

    auto sink = app->pm->get_sink_info(app->pm->server_info.default_sink_name);

    current_dev_rate << std::fixed << sink->rate / 1000.0f << "kHz";

    headerbar_info->set_text(
        " ⟶ " + app->pm->apps_sink_info->format + "," + null_sink_rate.str() +
        " ⟶ F32LE," + null_sink_rate.str() + " ⟶ " + sink->format + "," +
        current_dev_rate.str() + " ⟶ " + std::to_string(sie_latency) + "ms ⟶ ");

  } else {  // soe
    headerbar_icon1->set_from_icon_name("audio-input-microphone-symbolic",
                                        Gtk::ICON_SIZE_MENU);

    headerbar_icon2->set_from_icon_name("emblem-music-symbolic",
                                        Gtk::ICON_SIZE_MENU);

    null_sink_rate << std::fixed << app->pm->mic_sink_info->rate / 1000.0f
                   << "kHz";

    auto source =
        app->pm->get_source_info(app->pm->server_info.default_source_name);

    current_dev_rate << std::fixed << source->rate / 1000.0f << "kHz";

    headerbar_info->set_text(
        " ⟶ " + source->format + "," + current_dev_rate.str() + " ⟶ F32LE," +
        null_sink_rate.str() + " ⟶ " + app->pm->mic_sink_info->format + "," +
        null_sink_rate.str() + " ⟶ " + std::to_string(soe_latency) + "ms ⟶ ");
  }
}

void ApplicationUi::on_stack_visible_child_changed() {
  auto name = stack->get_visible_child_name();

  if (name == std::string("sink_inputs")) {
    spectrum_connection.disconnect();

    spectrum_connection = app->sie->new_spectrum.connect(
        sigc::mem_fun(*spectrum_ui, &SpectrumUi::on_new_spectrum));

    update_headerbar_subtitle(0);

  } else if (name == std::string("source_outputs")) {
    spectrum_connection.disconnect();

    spectrum_connection = app->soe->new_spectrum.connect(
        sigc::mem_fun(*spectrum_ui, &SpectrumUi::on_new_spectrum));

    update_headerbar_subtitle(1);
  }

  spectrum_ui->clear_spectrum();
}

void ApplicationUi::on_sink_added(std::shared_ptr<mySinkInfo> info) {
  bool add_to_list = true;

  auto children = sink_list->children();

  for (auto c : children) {
    uint i;
    std::string name;

    c.get_value(0, i);
    c.get_value(1, name);

    if (info->index == i) {
      add_to_list = false;

      break;
    }
  }

  if (add_to_list) {
    Gtk::TreeModel::Row row = *(sink_list->append());

    row->set_value(0, info->index);
    row->set_value(1, info->name);

    if (use_default_sink->get_active()) {
      if (info->name == app->pm->server_info.default_sink_name) {
        output_device->set_active(row);
      }
    } else {
      auto custom_sink = settings->get_string("custom-sink");

      if (info->name == custom_sink) {
        output_device->set_active(row);
      }
    }

    util::debug(log_tag + "added sink: " + info->name);
  }
}

void ApplicationUi::on_sink_removed(uint idx) {
  Gtk::TreeIter remove_iter;
  std::string remove_name;

  auto children = sink_list->children();

  for (auto c : children) {
    uint i;
    std::string name;

    c.get_value(0, i);
    c.get_value(1, name);

    if (idx == i) {
      remove_iter = c;
      remove_name = name;
    }
  }

  sink_list->erase(remove_iter);

  util::debug(log_tag + "removed sink: " + remove_name);
}

void ApplicationUi::on_source_added(std::shared_ptr<mySourceInfo> info) {
  bool add_to_list = true;

  auto children = source_list->children();

  for (auto c : children) {
    uint i;
    std::string name;

    c.get_value(0, i);
    c.get_value(1, name);

    if (info->index == i) {
      add_to_list = false;

      break;
    }
  }

  if (add_to_list) {
    Gtk::TreeModel::Row row = *(source_list->append());

    row->set_value(0, info->index);
    row->set_value(1, info->name);

    if (use_default_source->get_active()) {
      if (info->name == app->pm->server_info.default_source_name) {
        input_device->set_active(row);
      }
    } else {
      auto custom_source = settings->get_string("custom-source");

      if (info->name == custom_source) {
        input_device->set_active(row);
      }
    }

    util::debug(log_tag + "added source: " + info->name);
  }
}

void ApplicationUi::on_source_removed(uint idx) {
  Gtk::TreeIter remove_iter;
  std::string remove_name;

  auto children = source_list->children();

  for (auto c : children) {
    uint i;
    std::string name;

    c.get_value(0, i);
    c.get_value(1, name);

    if (idx == i) {
      remove_iter = c;
      remove_name = name;
    }
  }

  source_list->erase(remove_iter);

  util::debug(log_tag + "removed source: " + remove_name);
}

void ApplicationUi::on_use_default_sink_toggled() {
  if (use_default_sink->get_active()) {
    auto children = sink_list->children();

    for (auto c : children) {
      std::string name;

      c.get_value(1, name);

      if (name == app->pm->server_info.default_sink_name) {
        output_device->set_active(c);
      }
    }
  }
}

void ApplicationUi::on_use_default_source_toggled() {
  if (use_default_source->get_active()) {
    auto children = source_list->children();

    for (auto c : children) {
      std::string name;

      c.get_value(1, name);

      if (name == app->pm->server_info.default_source_name) {
        input_device->set_active(c);
      }
    }
  }
}

void ApplicationUi::on_input_device_changed() {
  Gtk::TreeModel::Row row = *(input_device->get_active());

  if (row) {
    uint index;
    std::string name;

    row.get_value(0, index);
    row.get_value(1, name);

    app->soe->set_source_monitor_name(name);

    if (!use_default_source->get_active()) {
      settings->set_string("custom-source", name);
    }

    util::debug(log_tag + "input device changed: " + name);
  }
}

void ApplicationUi::on_output_device_changed() {
  Gtk::TreeModel::Row row = *(output_device->get_active());

  if (row) {
    uint index;
    std::string name;

    row.get_value(0, index);
    row.get_value(1, name);

    app->sie->set_output_sink_name(name);
    app->soe->webrtc->set_probe_src_device(name + ".monitor");

    if (!use_default_sink->get_active()) {
      settings->set_string("custom-sink", name);
    }

    util::debug(log_tag + "output device changed: " + name);
  }
}

int ApplicationUi::on_listbox_sort(Gtk::ListBoxRow* row1,
                                   Gtk::ListBoxRow* row2) {
  auto name1 = row1->get_name();
  auto name2 = row2->get_name();

  std::vector<std::string> names = {name1, name2};

  std::sort(names.begin(), names.end());

  if (name1 == names[0]) {
    return -1;
  } else if (name2 == names[0]) {
    return 1;
  } else {
    return 0;
  }
}

void ApplicationUi::on_presets_menu_button_clicked() {
  int height = 0.7 * get_allocated_height();

  presets_scrolled_window->set_max_content_height(height);

  populate_presets_listbox();
}

void ApplicationUi::on_import_preset_clicked() {
  // gtkmm 3.22 does not have FileChooseNative so we have to use C api :-(

  gint res;

  auto dialog = gtk_file_chooser_native_new(
      _("Import Presets"), (GtkWindow*)this->gobj(),
      GTK_FILE_CHOOSER_ACTION_OPEN, _("Open"), _("Cancel"));

  auto filter = gtk_file_filter_new();

  gtk_file_filter_set_name(filter, _("Presets"));
  gtk_file_filter_add_pattern(filter, "*.json");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

  res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(dialog));

  if (res == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser* chooser = GTK_FILE_CHOOSER(dialog);

    auto file_list = gtk_file_chooser_get_filenames(chooser);

    g_slist_foreach(file_list,
                    [](auto data, auto user_data) {
                      auto aui = static_cast<ApplicationUi*>(user_data);

                      auto file_path = static_cast<char*>(data);

                      aui->app->presets_manager->import(file_path);
                    },
                    this);

    g_slist_free(file_list);
  }

  g_object_unref(dialog);

  populate_presets_listbox();
}

void ApplicationUi::populate_presets_listbox() {
  auto children = presets_listbox->get_children();

  for (auto c : children) {
    presets_listbox->remove(*c);
  }

  bool reset_menu_button_label = true;

  auto names = app->presets_manager->get_names();

  for (auto name : names) {
    auto b = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/ui/preset_row.glade");

    Gtk::ListBoxRow* row;
    Gtk::Button *save_btn, *remove_btn;
    Gtk::Label* label;

    b->get_widget("preset_row", row);
    b->get_widget("save", save_btn);
    b->get_widget("remove", remove_btn);
    b->get_widget("name", label);

    row->set_name(name);
    label->set_text(name);

    connections.push_back(save_btn->signal_clicked().connect(
        [=]() { app->presets_manager->save(name); }));
    connections.push_back(remove_btn->signal_clicked().connect([=]() {
      app->presets_manager->remove(name);
      populate_presets_listbox();
    }));

    presets_listbox->add(*row);
    presets_listbox->show_all();

    /*if the preset with the name in the button label still exists we do
    not reset the label to "Presets"
    */
    if (name == presets_menu_label->get_text()) {
      reset_menu_button_label = false;
    }
  }

  if (reset_menu_button_label) {
    presets_menu_label->set_text(_("Presets"));
  }
}

void ApplicationUi::populate_blacklist_in_listbox() {
  auto children = blacklist_in_listbox->get_children();

  for (auto c : children) {
    blacklist_in_listbox->remove(*c);
  }

  std::vector<std::string> names = settings->get_string_array("blacklist-in");

  for (auto name : names) {
    auto b = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/ui/blacklist_row.glade");

    Gtk::ListBoxRow* row;
    Gtk::Button* remove_btn;
    Gtk::Label* label;

    b->get_widget("blacklist_row", row);
    b->get_widget("remove", remove_btn);
    b->get_widget("name", label);

    row->set_name(name);
    label->set_text(name);

    connections.push_back(remove_btn->signal_clicked().connect([=]() {
      std::vector<std::string> bl = settings->get_string_array("blacklist-in");

      bl.erase(std::remove_if(bl.begin(), bl.end(),
                              [=](auto& a) { return a == name; }),
               bl.end());

      settings->set_string_array("blacklist-in", bl);

      populate_blacklist_in_listbox();
    }));

    blacklist_in_listbox->add(*row);
    blacklist_in_listbox->show_all();
  }
}

void ApplicationUi::populate_blacklist_out_listbox() {
  auto children = blacklist_out_listbox->get_children();

  for (auto c : children) {
    blacklist_out_listbox->remove(*c);
  }

  std::vector<std::string> names = settings->get_string_array("blacklist-out");

  for (auto name : names) {
    auto b = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/ui/blacklist_row.glade");

    Gtk::ListBoxRow* row;
    Gtk::Button* remove_btn;
    Gtk::Label* label;

    b->get_widget("blacklist_row", row);
    b->get_widget("remove", remove_btn);
    b->get_widget("name", label);

    row->set_name(name);
    label->set_text(name);

    connections.push_back(remove_btn->signal_clicked().connect([=]() {
      std::vector<std::string> bl = settings->get_string_array("blacklist-out");

      bl.erase(std::remove_if(bl.begin(), bl.end(),
                              [=](auto& a) { return a == name; }),
               bl.end());

      settings->set_string_array("blacklist-out", bl);

      populate_blacklist_out_listbox();
    }));

    blacklist_out_listbox->add(*row);
    blacklist_out_listbox->show_all();
  }
}

void ApplicationUi::on_calibration_button_clicked() {
  auto calibration_ui = CalibrationUi::create();

  auto c = app->pm->new_default_source.connect(
      [=](auto name) { calibration_ui->set_source_monitor_name(name); });

  calibration_ui->signal_hide().connect([calibration_ui, c]() {
    c->disconnect();
    delete calibration_ui;
  });

  calibration_ui->show_all();
}
