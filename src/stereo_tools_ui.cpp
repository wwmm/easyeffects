#include "stereo_tools_ui.hpp"
#include <cstring>

namespace {

auto stereo_tools_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  auto v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "LR > LR (Stereo Default)") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "LR > MS (Stereo to Mid-Side)") == 0) {
    g_value_set_int(value, 1);
  } else if (std::strcmp(v, "MS > LR (Mid-Side to Stereo)") == 0) {
    g_value_set_int(value, 2);
  } else if (std::strcmp(v, "LR > LL (Mono Left Channel)") == 0) {
    g_value_set_int(value, 3);
  } else if (std::strcmp(v, "LR > RR (Mono Right Channel)") == 0) {
    g_value_set_int(value, 4);
  } else if (std::strcmp(v, "LR > L+R (Mono Sum L+R)") == 0) {
    g_value_set_int(value, 5);
  } else if (std::strcmp(v, "LR > RL (Stereo Flip Channels)") == 0) {
    g_value_set_int(value, 6);
  }

  return 1;
}

auto int_to_stereo_tools_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("LR > LR (Stereo Default)");
  }

  if (v == 1) {
    return g_variant_new_string("LR > MS (Stereo to Mid-Side)");
  }

  if (v == 2) {
    return g_variant_new_string("MS > LR (Mid-Side to Stereo)");
  }

  if (v == 3) {
    return g_variant_new_string("LR > LL (Mono Left Channel)");
  }

  if (v == 4) {
    return g_variant_new_string("LR > RR (Mono Right Channel)");
  }

  if (v == 5) {
    return g_variant_new_string("LR > L+R (Mono Sum L+R)");
  }

  return g_variant_new_string("LR > RL (Stereo Flip Channels)");
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
  builder->get_widget("plugin_reset", reset_button);

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

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });
}

StereoToolsUi::~StereoToolsUi() {
  util::debug(name + " ui destroyed");
}

void StereoToolsUi::reset() {
  try {
    std::string section = (preset_type == PresetType::output) ? "output" : "input";

    update_default_key<double>(settings, "input-gain", section + ".stereo_tools.input-gain");

    update_default_key<double>(settings, "output-gain", section + ".stereo_tools.output-gain");

    update_default_key<double>(settings, "balance-in", section + ".stereo_tools.balance-in");

    update_default_key<double>(settings, "balance-out", section + ".stereo_tools.balance-out");

    update_default_key<bool>(settings, "softclip", section + ".stereo_tools.softclip");

    update_default_key<bool>(settings, "mutel", section + ".stereo_tools.mutel");

    update_default_key<bool>(settings, "muter", section + ".stereo_tools.muter");

    update_default_key<bool>(settings, "phasel", section + ".stereo_tools.phasel");

    update_default_key<bool>(settings, "phaser", section + ".stereo_tools.phaser");

    update_default_string_key(settings, "mode", section + ".stereo_tools.mode");

    update_default_key<double>(settings, "slev", section + ".stereo_tools.side-level");

    update_default_key<double>(settings, "sbal", section + ".stereo_tools.side-balance");

    update_default_key<double>(settings, "mlev", section + ".stereo_tools.middle-level");

    update_default_key<double>(settings, "mpan", section + ".stereo_tools.middle-panorama");

    update_default_key<double>(settings, "stereo-base", section + ".stereo_tools.stereo-base");

    update_default_key<double>(settings, "delay", section + ".stereo_tools.delay");

    update_default_key<double>(settings, "sc-level", section + ".stereo_tools.sc-level");

    update_default_key<double>(settings, "stereo-phase", section + ".stereo_tools.stereo-phase");

    util::debug(name + " plugin: successfully reset");
  } catch (std::exception& e) {
    util::debug(name + " plugin: an error occurred during reset process");
  }
}
