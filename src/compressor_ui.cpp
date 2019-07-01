#include "compressor_ui.hpp"

namespace {

gboolean mode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("Downward")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("Upward")) {
    g_value_set_int(value, 1);
  }

  return true;
}

GVariant* int_to_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("Downward");
  } else {
    return g_variant_new_string("Upward");
  }
}

gboolean sidechain_type_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("Feed-forward")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("Feed-back")) {
    g_value_set_int(value, 1);
  }

  return true;
}

GVariant* int_to_sidechain_type_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("Feed-forward");
  } else {
    return g_variant_new_string("Feed-back");
  }
}

gboolean sidechain_mode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("Peak")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("RMS")) {
    g_value_set_int(value, 1);
  } else if (v == std::string("Low-Pass")) {
    g_value_set_int(value, 2);
  } else if (v == std::string("Uniform")) {
    g_value_set_int(value, 3);
  }

  return true;
}

GVariant* int_to_sidechain_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("Peak");
  } else if (v == 1) {
    return g_variant_new_string("RMS");
  } else if (v == 2) {
    return g_variant_new_string("Low-Pass");
  } else {
    return g_variant_new_string("Uniform");
  }
}

gboolean sidechain_source_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("Middle")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("Side")) {
    g_value_set_int(value, 1);
  } else if (v == std::string("Left")) {
    g_value_set_int(value, 2);
  } else if (v == std::string("Right")) {
    g_value_set_int(value, 3);
  }

  return true;
}

GVariant* int_to_sidechain_source_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("Middle");
  } else if (v == 1) {
    return g_variant_new_string("Side");
  } else if (v == 2) {
    return g_variant_new_string("Left");
  } else {
    return g_variant_new_string("Right");
  }
}

}  // namespace

CompressorUi::CompressorUi(BaseObjectType* cobject,
                           const Glib::RefPtr<Gtk::Builder>& builder,
                           const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "compressor";

  // loading glade widgets

  builder->get_widget("listen", listen);
  builder->get_widget("compression_mode", compression_mode);
  builder->get_widget("sidechain_type", sidechain_type);
  builder->get_widget("sidechain_mode", sidechain_mode);
  builder->get_widget("sidechain_source", sidechain_source);
  builder->get_widget("reduction", reduction);
  builder->get_widget("reduction_label", reduction_label);
  builder->get_widget("sidechain", sidechain);
  builder->get_widget("sidechain_label", sidechain_label);
  builder->get_widget("curve", curve);
  builder->get_widget("curve_label", curve_label);

  get_object(builder, "attack", attack);
  get_object(builder, "knee", knee);
  get_object(builder, "makeup", makeup);
  get_object(builder, "ratio", ratio);
  get_object(builder, "release", release);
  get_object(builder, "threshold", threshold);
  get_object(builder, "preamp", preamp);
  get_object(builder, "reactivity", reactivity);
  get_object(builder, "lookahead", lookahead);
  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("attack", attack.get(), "value", flag);
  settings->bind("knee", knee.get(), "value", flag);
  settings->bind("makeup", makeup.get(), "value", flag);
  settings->bind("ratio", ratio.get(), "value", flag);
  settings->bind("release", release.get(), "value", flag);
  settings->bind("threshold", threshold.get(), "value", flag);
  settings->bind("sidechain-listen", listen, "active", flag);
  settings->bind("sidechain-preamp", preamp.get(), "value", flag);
  settings->bind("sidechain-reactivity", reactivity.get(), "value", flag);
  settings->bind("sidechain-lookahead", lookahead.get(), "value", flag);
  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);

  g_settings_bind_with_mapping(settings->gobj(), "mode", compression_mode->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               mode_enum_to_int, int_to_mode_enum, nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "sidechain-type", sidechain_type->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, sidechain_type_enum_to_int, int_to_sidechain_type_enum, nullptr,
                               nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "sidechain-mode", sidechain_mode->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, sidechain_mode_enum_to_int, int_to_sidechain_mode_enum, nullptr,
                               nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "sidechain-source", sidechain_source->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, sidechain_source_enum_to_int, int_to_sidechain_source_enum,
                               nullptr, nullptr);
}

CompressorUi::~CompressorUi() {
  util::debug(name + " ui destroyed");
}

void CompressorUi::on_new_reduction(double value) {
  reduction->set_value(value);

  reduction_label->set_text(level_to_str(util::linear_to_db(value), 0));
}

void CompressorUi::on_new_sidechain(double value) {
  sidechain->set_value(value);

  sidechain_label->set_text(level_to_str(util::linear_to_db(value), 0));
}

void CompressorUi::on_new_curve(double value) {
  curve->set_value(value);

  curve_label->set_text(level_to_str(util::linear_to_db(value), 0));
}
