#include "stereo_tools_ui.hpp"

namespace {

gboolean stereo_tools_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("LR > LR (Stereo Default)")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("LR > MS (Stereo to Mid-Side)")) {
    g_value_set_int(value, 1);
  } else if (v == std::string("MS > LR (Mid-Side to Stereo)")) {
    g_value_set_int(value, 2);
  } else if (v == std::string("LR > LL (Mono Left Channel)")) {
    g_value_set_int(value, 3);
  } else if (v == std::string("LR > RR (Mono Right Channel)")) {
    g_value_set_int(value, 4);
  } else if (v == std::string("LR > L+R (Mono Sum L+R)")) {
    g_value_set_int(value, 5);
  } else if (v == std::string("LR > RL (Stereo Flip Channels)")) {
    g_value_set_int(value, 6);
  }

  return true;
}

GVariant* int_to_stereo_tools_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("LR > LR (Stereo Default)");
  } else if (v == 1) {
    return g_variant_new_string("LR > MS (Stereo to Mid-Side)");
  } else if (v == 2) {
    return g_variant_new_string("MS > LR (Mid-Side to Stereo)");
  } else if (v == 3) {
    return g_variant_new_string("LR > LL (Mono Left Channel)");
  } else if (v == 4) {
    return g_variant_new_string("LR > RR (Mono Right Channel)");
  } else if (v == 5) {
    return g_variant_new_string("LR > L+R (Mono Sum L+R)");
  } else {
    return g_variant_new_string("LR > RL (Stereo Flip Channels)");
  }
}

}  // namespace

StereoToolsUi::StereoToolsUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& builder,
                             const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "stereo_tools";

  // loading glade widgets

  builder->get_widget("softclip", softclip);
  builder->get_widget("mutel", mutel);
  builder->get_widget("muter", muter);
  builder->get_widget("phasel", phasel);
  builder->get_widget("phaser", phaser);
  builder->get_widget("mode", mode);

  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);
  get_object(builder, "balance_in", balance_in);
  get_object(builder, "balance_out", balance_out);
  get_object(builder, "slev", slev);
  get_object(builder, "sbal", sbal);
  get_object(builder, "mlev", mlev);
  get_object(builder, "mpan", mpan);
  get_object(builder, "stereo_base", stereo_base);
  get_object(builder, "delay", delay);
  get_object(builder, "sc_level", sc_level);
  get_object(builder, "stereo_phase", stereo_phase);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);

  settings->bind("softclip", softclip, "active", flag);
  settings->bind("mutel", mutel, "active", flag);
  settings->bind("muter", muter, "active", flag);
  settings->bind("phasel", phasel, "active", flag);
  settings->bind("phaser", phaser, "active", flag);
  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);
  settings->bind("balance-in", balance_in.get(), "value", flag);
  settings->bind("balance-out", balance_out.get(), "value", flag);
  settings->bind("slev", slev.get(), "value", flag);
  settings->bind("sbal", sbal.get(), "value", flag);
  settings->bind("mlev", mlev.get(), "value", flag);
  settings->bind("mpan", mpan.get(), "value", flag);
  settings->bind("stereo-base", stereo_base.get(), "value", flag);
  settings->bind("delay", delay.get(), "value", flag);
  settings->bind("sc-level", sc_level.get(), "value", flag);
  settings->bind("stereo-phase", stereo_phase.get(), "value", flag);

  g_settings_bind_with_mapping(settings->gobj(), "mode", mode->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               stereo_tools_enum_to_int, int_to_stereo_tools_enum, nullptr, nullptr);
}

StereoToolsUi::~StereoToolsUi() {
  util::debug(name + " ui destroyed");
}
