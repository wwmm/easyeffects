#include "crossfeed_ui.hpp"

CrossfeedUi::CrossfeedUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "crossfeed";

  // loading glade widgets

  builder->get_widget("preset_cmoy", preset_cmoy);
  builder->get_widget("preset_default", preset_default);
  builder->get_widget("preset_jmeier", preset_jmeier);

  get_object(builder, "fcut", fcut);
  get_object(builder, "feed", feed);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("fcut", fcut.get(), "value", flag);
  settings->bind("feed", feed.get(), "value", flag);

  init_presets_buttons();
}

CrossfeedUi::~CrossfeedUi() {
  util::debug(name + " ui destroyed");
}

void CrossfeedUi::init_presets_buttons() {
  preset_cmoy->signal_clicked().connect([=]() {
    fcut->set_value(700);
    feed->set_value(6);
  });

  preset_default->signal_clicked().connect([=]() {
    fcut->set_value(700);
    feed->set_value(4.5);
  });

  preset_jmeier->signal_clicked().connect([=]() {
    fcut->set_value(650);
    feed->set_value(9);
  });
}
