#ifndef UTIL_HPP
#define UTIL_HPP

#include <glib.h>
#include <iostream>
#include <vector>

namespace util {

void debug(const std::string& s);
void error(const std::string& s);
void critical(const std::string& s);
void warning(const std::string& s);

std::vector<float> logspace(float start, float stop, uint npoints);

}  // namespace util

#endif
