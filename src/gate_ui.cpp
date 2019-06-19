#include "gate_ui.hpp"

namespace {

gboolean detection_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("RMS")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("Peak")) {
    g_value_set_int(value, 1);
  }

  return true;
}

GVariant* int_to_detection_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("RMS");
  } else {
    return g_variant_new_string("Peak");
  }
}

gboolean stereo_link_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("Average")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("Maximum")) {
    g_value_set_int(value, 1);
  }

  return true;
}

GVariant* int_to_stereo_link_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("Average");
  } else {
    return g_variant_new_string("Maximum");
  }
}

}  // namespace

GateUi::GateUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "gate";

  // loading glade widgets

  builder->get_widget("detection", detection);
  builder->get_widget("stereo_link", stereo_link);
  builder->get_widget("gating", gating);
  builder->get_widget("gating_label", gating_label);

  get_object(builder, "attack", attack);
  get_object(builder, "knee", knee);
  get_object(builder, "makeup", makeup);
  get_object(builder, "range", range);
  get_object(builder, "ratio", ratio);
  get_object(builder, "release", release);
  get_object(builder, "threshold", threshold);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("attack", attack.get(), "value", flag);
  settings->bind("knee", knee.get(), "value", flag);
  settings->bind("makeup", makeup.get(), "value", flag);
  settings->bind("range", range.get(), "value", flag);
  settings->bind("ratio", ratio.get(), "value", flag);
  settings->bind("release", release.get(), "value", flag);
  settings->bind("threshold", threshold.get(), "value", flag);

  g_settings_bind_with_mapping(settings->gobj(), "detection", detection->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               detection_enum_to_int, int_to_detection_enum, nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "stereo-link", stereo_link->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               stereo_link_enum_to_int, int_to_stereo_link_enum, nullptr, nullptr);
}

GateUi::~GateUi() {
  util::debug(name + " ui destroyed");
}

void GateUi::on_new_gating(double value) {
  gating->set_value(1 - value);

  gating_label->set_text(level_to_str(util::linear_to_db(value), 0));
}
