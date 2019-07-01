#include "webrtc_ui.hpp"

namespace {

gboolean echo_suppression_level_to_int(GValue* value, GVariant* variant, gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("low")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("moderate")) {
    g_value_set_int(value, 1);
  } else if (v == std::string("high")) {
    g_value_set_int(value, 2);
  }

  return true;
}

GVariant* int_to_echo_suppression_level(const GValue* value, const GVariantType* expected_type, gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("low");
  } else if (v == 1) {
    return g_variant_new_string("moderate");
  } else {
    return g_variant_new_string("high");
  }
}

gboolean noise_suppression_level_to_int(GValue* value, GVariant* variant, gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("low")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("moderate")) {
    g_value_set_int(value, 1);
  } else if (v == std::string("high")) {
    g_value_set_int(value, 2);
  } else if (v == std::string("very-high")) {
    g_value_set_int(value, 3);
  }

  return true;
}

GVariant* int_to_noise_suppression_level(const GValue* value, const GVariantType* expected_type, gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("low");
  } else if (v == 1) {
    return g_variant_new_string("moderate");
  } else if (v == 2) {
    return g_variant_new_string("high");
  } else {
    return g_variant_new_string("very-high");
  }
}

gboolean gain_control_mode_to_int(GValue* value, GVariant* variant, gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("adaptive-digital")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("fixed-digital")) {
    g_value_set_int(value, 1);
  }

  return true;
}

GVariant* int_to_gain_control_mode(const GValue* value, const GVariantType* expected_type, gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("adaptive-digital");
  } else {
    return g_variant_new_string("fixed-digital");
  }
}

gboolean voice_detection_likelihood_to_int(GValue* value, GVariant* variant, gpointer user_data) {
  auto v = g_variant_get_string(variant, nullptr);

  if (v == std::string("very-low")) {
    g_value_set_int(value, 0);
  } else if (v == std::string("low")) {
    g_value_set_int(value, 1);
  } else if (v == std::string("moderate")) {
    g_value_set_int(value, 2);
  } else if (v == std::string("high")) {
    g_value_set_int(value, 3);
  }

  return true;
}

GVariant* int_to_voice_detection_likelihood(const GValue* value,
                                            const GVariantType* expected_type,
                                            gpointer user_data) {
  int v = g_value_get_int(value);

  if (v == 0) {
    return g_variant_new_string("very-low");
  } else if (v == 1) {
    return g_variant_new_string("low");
  } else if (v == 2) {
    return g_variant_new_string("moderate");
  } else {
    return g_variant_new_string("high");
  }
}

}  // namespace

WebrtcUi::WebrtcUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "webrtc";

  // loading glade widgets

  builder->get_widget("echo_cancel", echo_cancel);
  builder->get_widget("extended_filter", extended_filter);
  builder->get_widget("high_pass_filter", high_pass_filter);
  builder->get_widget("delay_agnostic", delay_agnostic);
  builder->get_widget("noise_suppression", noise_suppression);
  builder->get_widget("gain_control", gain_control);
  builder->get_widget("limiter", limiter);
  builder->get_widget("voice_detection", voice_detection);
  builder->get_widget("echo_suppression_level", echo_suppression_level);
  builder->get_widget("noise_suppression_level", noise_suppression_level);
  builder->get_widget("gain_control_mode", gain_control_mode);
  builder->get_widget("voice_detection_likelihood", voice_detection_likelihood);

  get_object(builder, "compression_gain_db", compression_gain_db);
  get_object(builder, "target_level_dbfs", target_level_dbfs);
  get_object(builder, "voice_detection_frame_size", voice_detection_frame_size);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);

  settings->bind("compression-gain-db", compression_gain_db.get(), "value", flag);
  settings->bind("target-level-dbfs", target_level_dbfs.get(), "value", flag);
  settings->bind("voice-detection-frame-size-ms", voice_detection_frame_size.get(), "value", flag);
  settings->bind("echo-cancel", echo_cancel, "active", flag);
  settings->bind("extended-filter", extended_filter, "active", flag);
  settings->bind("high-pass-filter", high_pass_filter, "active", flag);
  settings->bind("delay-agnostic", delay_agnostic, "active", flag);
  settings->bind("noise-suppression", noise_suppression, "active", flag);
  settings->bind("gain-control", gain_control, "active", flag);
  settings->bind("limiter", limiter, "active", flag);
  settings->bind("voice-detection", voice_detection, "active", flag);

  g_settings_bind_with_mapping(settings->gobj(), "echo-suppression-level", echo_suppression_level->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, echo_suppression_level_to_int, int_to_echo_suppression_level,
                               nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "noise-suppression-level", noise_suppression_level->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, noise_suppression_level_to_int, int_to_noise_suppression_level,
                               nullptr, nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "gain-control-mode", gain_control_mode->gobj(), "active",
                               G_SETTINGS_BIND_DEFAULT, gain_control_mode_to_int, int_to_gain_control_mode, nullptr,
                               nullptr);

  g_settings_bind_with_mapping(settings->gobj(), "voice-detection-likelihood", voice_detection_likelihood->gobj(),
                               "active", G_SETTINGS_BIND_DEFAULT, voice_detection_likelihood_to_int,
                               int_to_voice_detection_likelihood, nullptr, nullptr);
}

WebrtcUi::~WebrtcUi() {
  util::debug(name + " ui destroyed");
}
