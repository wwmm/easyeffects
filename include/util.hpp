#ifndef UTIL_HPP
#define UTIL_HPP

#include <glib-object.h>
#include <glib.h>
#include <iostream>
#include <thread>
#include <vector>

namespace util {

void debug(const std::string& s);
void error(const std::string& s);
void critical(const std::string& s);
void warning(const std::string& s);
void info(const std::string& s);

std::vector<float> logspace(const float& start, const float& stop, const uint& npoints);
std::vector<float> linspace(const float& start, const float& stop, const uint& npoints);

float linear_to_db(const float& amp);

float db_to_linear(const float& db);

gboolean db20_gain_to_linear(GValue* value, GVariant* variant, gpointer user_data);

GVariant* linear_gain_to_db20(const GValue* value, const GVariantType* expected_type, gpointer user_data);

gboolean db10_gain_to_linear(GValue* value, GVariant* variant, gpointer user_data);

gboolean double_to_float(GValue* value, GVariant* variant, gpointer user_data);

gboolean db20_gain_to_linear_double(GValue* value, GVariant* variant, gpointer user_data);

GVariant* linear_double_gain_to_db20(const GValue* value, const GVariantType* expected_type, gpointer user_data);

gboolean double_x10_to_int(GValue* value, GVariant* variant, gpointer user_data);

gboolean ms_to_ns(GValue* value, GVariant* variant, gpointer user_data);

void print_thread_id();

}  // namespace util

#endif
