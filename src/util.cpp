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

}  // namespace util
