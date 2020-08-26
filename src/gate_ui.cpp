#include "gate_ui.hpp"
#include <cstring>

namespace {

auto detection_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  auto v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "RMS") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "Peak") == 0) {
    g_value_set_int(value, 1);
  }

  return 1;
}

auto int_to_detection_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("RMS");
  }

  return g_variant_new_string("Peak");
}

auto stereo_link_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  auto v = g_variant_get_string(variant, nullptr);

  if (std::strcmp(v, "Average") == 0) {
    g_value_set_int(value, 0);
  } else if (std::strcmp(v, "Maximum") == 0) {
    g_value_set_int(value, 1);
  }

  return 1;
}

auto int_to_stereo_link_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("Average");
  }

  return g_variant_new_string("Maximum");
}

}  // namespace

GateUi::GateUi(BaseObjectType* cobject,
               const Glib::RefPtr<Gtk::Builder>& builder,
               const std::string& schema,
               const std::string& schema_path)
    : Gtk::Grid(cobject), PluginUiBase(builder, schema, schema_path) {
  name = "gate";

  // loading glade widgets

  builder->get_widget("detection", detection);
  builder->get_widget("stereo_link", stereo_link);
  builder->get_widget("gating", gating);
  builder->get_widget("gating_label", gating_label);
  builder->get_widget("plugin_reset", reset_button);

  get_object(builder, "attack", attack);
  get_object(builder, "knee", knee);
  get_object(builder, "input", input);
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
  settings->bind("input", input.get(), "value", flag);
  settings->bind("makeup", makeup.get(), "value", flag);
  settings->bind("range", range.get(), "value", flag);
  settings->bind("ratio", ratio.get(), "value", flag);
  settings->bind("release", release.get(), "value", flag);
  settings->bind("threshold", threshold.get(), "value", flag);

  g_settings_bind_with_mapping(settings->gobj(), "detection", detection->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               detection_enum_to_int, int_to_detection_enum, nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "stereo-link", stereo_link->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               stereo_link_enum_to_int, int_to_stereo_link_enum, nullptr, nullptr);

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });
}

GateUi::~GateUi() {
  util::debug(name + " ui destroyed");
}

void GateUi::reset() {
  try {
    std::string section = (preset_type == PresetType::output) ? "output" : "input";

    update_default_string_key(settings, "detection", section + ".gate.detection");

    update_default_string_key(settings, "stereo-link", section + ".gate.stereo-link");

    update_default_key<double>(settings, "range", section + ".gate.range");

    update_default_key<double>(settings, "attack", section + ".gate.attack");

    update_default_key<double>(settings, "release", section + ".gate.release");

    update_default_key<double>(settings, "threshold", section + ".gate.threshold");

    update_default_key<double>(settings, "ratio", section + ".gate.ratio");

    update_default_key<double>(settings, "knee", section + ".gate.knee");

    update_default_key<double>(settings, "input", section + ".gate.input");

    update_default_key<double>(settings, "makeup", section + ".gate.makeup");

    util::debug(name + " plugin: successfully reset");
  } catch (std::exception& e) {
    util::debug(name + " plugin: an error occurred during reset process");
  }
}

void GateUi::on_new_gating(double value) {
  gating->set_value(1.0 - value);

  gating_label->set_text(level_to_str(util::linear_to_db(static_cast<float>(value)), 0));
}
