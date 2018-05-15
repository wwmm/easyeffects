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

std::vector<float> logspace(float start, float stop, uint npoints) {
    std::vector<float> output;

    float delta = (stop - start) / npoints;

    float v = 0.0f;

    while (v < stop) {
        output.push_back(pow(10, v));

        v += delta;
    }

    return output;
}

std::vector<float> linspace(float start, float stop, uint npoints) {
    std::vector<float> output;

    float delta = (stop - start) / npoints;

    float v = 0.0f;

    while (v < stop) {
        output.push_back(v);

        v += delta;
    }

    return output;
}

gboolean db20_gain_to_linear(GValue* value,
                             GVariant* variant,
                             gpointer user_data) {
    double v_db = g_variant_get_double(variant);

    auto v_linear = pow(10, v_db / 20.0);

    g_value_set_float(value, v_linear);

    return true;
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

gboolean ms_to_ns(GValue* value, GVariant* variant, gpointer user_data) {
    gint64 v_ns = g_variant_get_double(variant) * 1000000;

    warning(std::to_string(v_ns));

    g_value_set_int64(value, v_ns);

    return true;
}

}  // namespace util
