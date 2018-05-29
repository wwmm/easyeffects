#include <cmath>
#include "util.hpp"

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

std::vector<float> logspace(const float& start,
                            const float& stop,
                            const uint& npoints) {
    std::vector<float> output;

    float delta = (stop - start) / npoints;

    float v = 0.0f;

    while (v < stop) {
        output.push_back(pow(10, v));

        v += delta;
    }

    return output;
}

std::vector<float> linspace(const float& start,
                            const float& stop,
                            const uint& npoints) {
    std::vector<float> output;

    float delta = (stop - start) / npoints;

    float v = 0.0f;

    while (v < stop) {
        output.push_back(v);

        v += delta;
    }

    return output;
}

float linear_to_db(const float& amp) {
    if (amp >= 0.00001) {
        return 20 * log10(amp);
    } else {
        return -99;
    }
}

float db_to_linear(const float& db) {
    return exp((db / 20.0) * log(10.0));
}

gboolean db20_gain_to_linear(GValue* value,
                             GVariant* variant,
                             gpointer user_data) {
    double v_db = g_variant_get_double(variant);

    auto v_linear = pow(10, v_db / 20.0);

    g_value_set_float(value, v_linear);

    return true;
}

GVariant* linear_gain_to_db20(const GValue* value,
                              const GVariantType* expected_type,
                              gpointer user_data) {
    double v_linear = g_value_get_float(value);

    auto v_db = 20 * log10(v_linear);

    return g_variant_new_double(v_db);
}

gboolean db10_gain_to_linear(GValue* value,
                             GVariant* variant,
                             gpointer user_data) {
    double v_db = g_variant_get_double(variant);

    auto v_linear = pow(10, v_db / 10.0);

    g_value_set_float(value, v_linear);

    return true;
}

gboolean double_to_float(GValue* value, GVariant* variant, gpointer user_data) {
    double v_d = g_variant_get_double(variant);

    g_value_set_float(value, v_d);

    return true;
}

gboolean double_x10_to_int(GValue* value,
                           GVariant* variant,
                           gpointer user_data) {
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
