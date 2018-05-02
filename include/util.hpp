#ifndef UTIL_HPP
#define UTIL_HPP

#include <glib.h>
#include <iostream>

namespace util {

void debug(const std::string& s);
void error(const std::string& s);
void critical(const std::string& s);
void warning(const std::string& s);

}  // namespace util

#endif
