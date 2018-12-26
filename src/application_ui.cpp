#include "application_ui.hpp"
#include <glibmm/i18n.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/settings.h>
#include "util.hpp"

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

  builder->get_widget("placeholder_spectrum", placeholder_spectrum);
  builder->get_widget("stack", stack);
  builder->get_widget("stack_menu_settings", stack_menu_settings);
  builder->get_widget("presets_menu_button", presets_menu_button);
  builder->get_widget("presets_menu", presets_menu);
  builder->get_widget("presets_menu_label", presets_menu_label);

  builder->get_widget("calibration_button", calibration_button);

  builder->get_widget("headerbar", headerbar);
  builder->get_widget("help_button", help_button);
  builder->get_widget("headerbar_icon1", headerbar_icon1);
  builder->get_widget("headerbar_icon2", headerbar_icon2);
  builder->get_widget("headerbar_info", headerbar_info);

  stack->connect_property_changed(
      "visible-child",
      sigc::mem_fun(*this, &ApplicationUi::on_stack_visible_child_changed));

  // calibration

  calibration_button->signal_clicked().connect(
      sigc::mem_fun(*this, &ApplicationUi::on_calibration_button_clicked));

  // pulseaudio signals

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

  // presets menu widgets

  presets_menu_ui = PresetsMenuUi::add_to_popover(presets_menu, app);

  presets_menu_button->signal_clicked().connect(sigc::mem_fun(
      *presets_menu_ui, &PresetsMenuUi::on_presets_menu_button_clicked));

  presets_menu_label->set_text(settings->get_string("last-used-preset"));

  settings->signal_changed("last-used-preset").connect([=](auto key) {
    presets_menu_label->set_text(settings->get_string("last-used-preset"));
  });

  // spectrum widgets

  spectrum_ui = SpectrumUi::add_to_box(placeholder_spectrum, app);

  // general settings widgets

  general_settings_ui =
      GeneralSettingsUi::add_to_stack(stack_menu_settings, app);

  // spectrum settings widgets

  auto b_spectrum_settings = Gtk::Builder::create_from_resource(
      "/com/github/wwmm/pulseeffects/ui/spectrum_settings.glade");

  b_spectrum_settings->get_widget_derived("widgets_grid", spectrum_settings_ui,
                                          settings, app);

  stack_menu_settings->add(*spectrum_settings_ui, "settings_spectrum",
                           _("Spectrum"));

  // Pulseaudio settings widgets

  auto b_pulse_settings = Gtk::Builder::create_from_resource(
      "/com/github/wwmm/pulseeffects/ui/pulse_settings.glade");

  b_pulse_settings->get_widget_derived("widgets_grid", pulse_settings_ui,
                                       settings, app);

  stack_menu_settings->add(*pulse_settings_ui, "settings_pulse",
                           _("Pulseaudio"));

  // Blacklist settings widgets

  auto b_blacklist_settings = Gtk::Builder::create_from_resource(
      "/com/github/wwmm/pulseeffects/ui/blacklist_settings.glade");

  b_blacklist_settings->get_widget_derived("widgets_grid",
                                           blacklist_settings_ui, settings);

  stack_menu_settings->add(*blacklist_settings_ui, "settings_blacklist",
                           _("Blacklist"));

  // sink inputs widgets

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

  // source outputs widgets

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

  settings->bind("use-dark-theme", Gtk::Settings::get_default().get(),
                 "gtk_application_prefer_dark_theme", flag);
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
