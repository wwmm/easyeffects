#ifndef UTIL_HPP
#define UTIL_HPP

#include <glib-object.h>
#include <glib.h>
#include <iostream>
#include <vector>

namespace util {

void debug(const std::string& s);
void error(const std::string& s);
void critical(const std::string& s);
void warning(const std::string& s);

std::vector<float> logspace(float start, float stop, uint npoints);
std::vector<float> linspace(float start, float stop, uint npoints);

gboolean db20_gain_to_linear(GValue* value,
                             GVariant* variant,
                             gpointer user_data);

GVariant* linear_gain_to_db20(const GValue* value,
                              const GVariantType* expected_type,
                              gpointer user_data);

gboolean db10_gain_to_linear(GValue* value,
                             GVariant* variant,
                             gpointer user_data);

gboolean double_to_float(GValue* value, GVariant* variant, gpointer user_data);

gboolean ms_to_ns(GValue* value, GVariant* variant, gpointer user_data);

}  // namespace util

#endif
