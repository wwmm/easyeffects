#include "compressor_ui.hpp"

namespace {

gboolean detection_enum_to_int(GValue* value,
                               GVariant* variant,
                               gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("RMS")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("Peak")) {
    g_value_set_int(value, 1);
  }

  return true;
}

GVariant* int_to_detection_enum(const GValue* value,
                                const GVariantType* expected_type,
                                gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("RMS");
  } else {
    return g_variant_new_string("Peak");
  }
}

gboolean stereo_link_enum_to_int(GValue* value,
                                 GVariant* variant,
                                 gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("Average")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("Maximum")) {
    g_value_set_int(value, 1);
  }

  return true;
}

GVariant* int_to_stereo_link_enum(const GValue* value,
                                  const GVariantType* expected_type,
                                  gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("Average");
  } else {
    return g_variant_new_string("Maximum");
  }
}

}  // namespace

CompressorUi::CompressorUi(BaseObjectType* cobject,
                           const Glib::RefPtr<Gtk::Builder>& builder,
                           const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "compressor";

  // loading glade widgets

  builder->get_widget("compression_mode", compression_mode);
  builder->get_widget("sidechain_type", sidechain_type);
  builder->get_widget("sidechain_mode", sidechain_mode);
  builder->get_widget("sidechain_source", sidechain_source);
  builder->get_widget("reduction", reduction);
  builder->get_widget("reduction_label", reduction_label);

  get_object(builder, "attack", attack);
  get_object(builder, "knee", knee);
  get_object(builder, "makeup", makeup);
  get_object(builder, "ratio", ratio);
  get_object(builder, "release", release);
  get_object(builder, "threshold", threshold);
  get_object(builder, "preamp", preamp);
  get_object(builder, "reactivity", reactivity);
  get_object(builder, "lookahead", lookahead);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("attack", attack.get(), "value", flag);
  settings->bind("knee", knee.get(), "value", flag);
  settings->bind("makeup", makeup.get(), "value", flag);
  settings->bind("ratio", ratio.get(), "value", flag);
  settings->bind("release", release.get(), "value", flag);
  settings->bind("threshold", threshold.get(), "value", flag);
  settings->bind("preamp", preamp.get(), "value", flag);
  settings->bind("reactivity", reactivity.get(), "value", flag);
  settings->bind("lookahead", lookahead.get(), "value", flag);

  // g_settings_bind_with_mapping(settings->gobj(), "detection",
  // detection->gobj(),
  //                              "active", G_SETTINGS_BIND_DEFAULT,
  //                              detection_enum_to_int, int_to_detection_enum,
  //                              nullptr, nullptr);
  //
  // g_settings_bind_with_mapping(settings->gobj(), "stereo-link",
  //                              stereo_link->gobj(), "active",
  //                              G_SETTINGS_BIND_DEFAULT,
  //                              stereo_link_enum_to_int,
  //                              int_to_stereo_link_enum, nullptr, nullptr);

  settings->set_boolean("post-messages", true);
}

CompressorUi::~CompressorUi() {
  settings->set_boolean("post-messages", false);

  util::debug(name + " ui destroyed");
}

void CompressorUi::on_new_reduction(double value) {
  reduction->set_value(1 - value);

  reduction_label->set_text(level_to_str(util::linear_to_db(value)));
}
