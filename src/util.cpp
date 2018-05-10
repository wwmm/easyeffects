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

    while (v <= stop) {
        output.push_back(pow(10, v));

        v += delta;
    }

    return output;
}

}  // namespace util
