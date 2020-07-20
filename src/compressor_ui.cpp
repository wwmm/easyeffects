#include "compressor_ui.hpp"

namespace {

auto mode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("Downward")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("Upward")) {
    g_value_set_int(value, 1);
  }

  return 1;
}

auto int_to_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("Downward");
  }

  return g_variant_new_string("Upward");
}

auto sidechain_type_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("Feed-forward")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("Feed-back")) {
    g_value_set_int(value, 1);
  }

  return 1;
}

auto int_to_sidechain_type_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("Feed-forward");
  }

  return g_variant_new_string("Feed-back");
}

auto sidechain_mode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
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

  return 1;
}

auto int_to_sidechain_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("Peak");
  }

  if (v == 1) {
    return g_variant_new_string("RMS");
  }

  if (v == 2) {
    return g_variant_new_string("Low-Pass");
  }

  return g_variant_new_string("Uniform");
}

auto sidechain_source_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
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

  return 1;
}

auto int_to_sidechain_source_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("Middle");
  }

  if (v == 1) {
    return g_variant_new_string("Side");
  }

  if (v == 2) {
    return g_variant_new_string("Left");
  }

  return g_variant_new_string("Right");
}

auto filter_mode_enum_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("off")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("12 dB/oct")) {
    g_value_set_int(value, 1);
  } else if (v == std::string("24 dB/oct")) {
    g_value_set_int(value, 2);
  } else if (v == std::string("36 dB/oct")) {
    g_value_set_int(value, 3);
  }

  return 1;
}

auto int_to_filter_mode_enum(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("off");
  }

  if (v == 1) {
    return g_variant_new_string("12 dB/oct");
  }

  if (v == 2) {
    return g_variant_new_string("24 dB/oct");
  }

  return g_variant_new_string("36 dB/oct");
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
  builder->get_widget("hpf_mode", hpf_mode);
  builder->get_widget("lpf_mode", lpf_mode);
  builder->get_widget("reduction", reduction);
  builder->get_widget("reduction_label", reduction_label);
  builder->get_widget("sidechain", sidechain);
  builder->get_widget("sidechain_label", sidechain_label);
  builder->get_widget("curve", curve);
  builder->get_widget("curve_label", curve_label);
  builder->get_widget("plugin_reset", reset_button);

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
  get_object(builder, "release_threshold", release_threshold);
  get_object(builder, "boost_threshold", boost_threshold);
  get_object(builder, "hpf_freq", hpf_freq);
  get_object(builder, "lpf_freq", lpf_freq);

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
  settings->bind("release-threshold", release_threshold.get(), "value", flag);
  settings->bind("boost-threshold", boost_threshold.get(), "value", flag);
  settings->bind("hpf-frequency", hpf_freq.get(), "value", flag);
  settings->bind("lpf-frequency", lpf_freq.get(), "value", flag);

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

  g_settings_bind_with_mapping(settings->gobj(), "hpf-mode", hpf_mode->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               filter_mode_enum_to_int, int_to_filter_mode_enum, nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "lpf-mode", lpf_mode->gobj(), "active", G_SETTINGS_BIND_DEFAULT,
                               filter_mode_enum_to_int, int_to_filter_mode_enum, nullptr, nullptr);

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });
}

CompressorUi::~CompressorUi() {
  util::debug(name + " ui destroyed");
}

void CompressorUi::reset() {
  try {
    std::string section = (preset_type == PresetType::output) ? "output" : "input";

    update_default_key<double>(settings, "input-gain", section + ".compressor.input-gain");

    update_default_key<double>(settings, "output-gain", section + ".compressor.output-gain");

    update_default_string_key(settings, "mode", section + ".compressor.mode");

    update_default_key<double>(settings, "attack", section + ".compressor.attack");

    update_default_key<double>(settings, "release", section + ".compressor.release");

    update_default_key<double>(settings, "release-threshold", section + ".compressor.release-threshold");

    update_default_key<double>(settings, "threshold", section + ".compressor.threshold");

    update_default_key<double>(settings, "ratio", section + ".compressor.ratio");

    update_default_key<double>(settings, "knee", section + ".compressor.knee");

    update_default_key<double>(settings, "makeup", section + ".compressor.makeup");

    update_default_key<double>(settings, "boost-threshold", section + ".compressor.boost-threshold");

    update_default_key<bool>(settings, "sidechain-listen", section + ".compressor.sidechain.listen");

    update_default_string_key(settings, "sidechain-type", section + ".compressor.sidechain.type");

    update_default_string_key(settings, "sidechain-mode", section + ".compressor.sidechain.mode");

    update_default_string_key(settings, "sidechain-source", section + ".compressor.sidechain.source");

    update_default_key<double>(settings, "sidechain-preamp", section + ".compressor.sidechain.preamp");

    update_default_key<double>(settings, "sidechain-reactivity", section + ".compressor.sidechain.reactivity");

    update_default_key<double>(settings, "sidechain-lookahead", section + ".compressor.sidechain.lookahead");

    update_default_string_key(settings, "hpf-mode", section + ".compressor.hpf-mode");

    update_default_key<double>(settings, "hpf-frequency", section + ".compressor.hpf-frequency");

    update_default_string_key(settings, "lpf-mode", section + ".compressor.lpf-mode");

    update_default_key<double>(settings, "lpf-frequency", section + ".compressor.lpf-frequency");

    util::debug(name + " plugin: successfully reset");
  } catch (std::exception& e) {
    util::debug(name + " plugin: an error occurred during reset process");
  }
}

void CompressorUi::on_new_reduction(double value) {
  reduction->set_value(value);

  reduction_label->set_text(level_to_str(util::linear_to_db(static_cast<float>(value)), 0));
}

void CompressorUi::on_new_sidechain(double value) {
  sidechain->set_value(value);

  sidechain_label->set_text(level_to_str(util::linear_to_db(static_cast<float>(value)), 0));
}

void CompressorUi::on_new_curve(double value) {
  curve->set_value(value);

  curve_label->set_text(level_to_str(util::linear_to_db(static_cast<float>(value)), 0));
}
