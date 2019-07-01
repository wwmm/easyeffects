#include "util.hpp"
#include <cmath>

namespace util {

void debug(const std::string& s) {
  g_debug(s.c_str(), "%s");
}

void error(const std::string& s) {
  g_error(s.c_str(), "%s");
}

void critical(const std::string& s) {
  g_critical(s.c_str(), "%s");
}

void warning(const std::string& s) {
  g_warning(s.c_str(), "%s");
}

void info(const std::string& s) {
  g_info(s.c_str(), "%s");
}

void print_thread_id() {
  std::cout << "thread id: " << std::this_thread::get_id() << std::endl;
}

std::vector<float> logspace(const float& start, const float& stop, const uint& npoints) {
  std::vector<float> output;

  float delta = (stop - start) / npoints;

  float v = start;

  while (v < stop) {
    output.push_back(powf(10.0f, v));

    v += delta;
  }

  return output;
}

std::vector<float> linspace(const float& start, const float& stop, const uint& npoints) {
  std::vector<float> output;

  float delta = (stop - start) / npoints;

  float v = start;

  while (v < stop) {
    output.push_back(v);

    v += delta;
  }

  return output;
}

float linear_to_db(const float& amp) {
  if (amp >= 0.00001f) {
    return 20.0f * log10f(amp);
  } else {
    return -99.0f;
  }
}

float db_to_linear(const float& db) {
  return expf((db / 20.0f) * logf(10.0f));
}

gboolean db20_gain_to_linear(GValue* value, GVariant* variant, gpointer user_data) {
  double v_db = g_variant_get_double(variant);

  float v_linear = powf(10.0f, (float)v_db / 20.0f);

  g_value_set_float(value, v_linear);

  return true;
}

GVariant* linear_gain_to_db20(const GValue* value, const GVariantType* expected_type, gpointer user_data) {
  float v_linear = g_value_get_float(value);

  double v_db = 20 * log10f(v_linear);

  return g_variant_new_double(v_db);
}

gboolean db10_gain_to_linear(GValue* value, GVariant* variant, gpointer user_data) {
  double v_db = g_variant_get_double(variant);

  float v_linear = powf(10.0f, (float)v_db / 10.0f);

  g_value_set_float(value, v_linear);

  return true;
}

gboolean double_to_float(GValue* value, GVariant* variant, gpointer user_data) {
  float v_d = g_variant_get_double(variant);

  g_value_set_float(value, v_d);

  return true;
}

gboolean db20_gain_to_linear_double(GValue* value, GVariant* variant, gpointer user_data) {
  double v_db = g_variant_get_double(variant);

  double v_linear = pow(10, v_db / 20.0);

  g_value_set_double(value, v_linear);

  return true;
}

GVariant* linear_double_gain_to_db20(const GValue* value, const GVariantType* expected_type, gpointer user_data) {
  double v_linear = g_value_get_double(value);

  double v_db = 20 * log10(v_linear);

  return g_variant_new_double(v_db);
}

gboolean double_x10_to_int(GValue* value, GVariant* variant, gpointer user_data) {
  double v_d = g_variant_get_double(variant);

  g_value_set_int(value, v_d * 10);

  return true;
}

gboolean ms_to_ns(GValue* value, GVariant* variant, gpointer user_data) {
  guint64 v_ns = g_variant_get_double(variant) * 1000000;

  g_value_set_uint64(value, v_ns);

  return true;
}

}  // namespace util
