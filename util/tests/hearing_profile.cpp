#include "hearing_profile.hpp"
#include <array>
#include <iostream>
#include <limits>
#include <span>

int main() {
  const std::array<double, 6> flat{-60, -60, -60, -60, -60, -60};
  const auto flat_profile = hearing::make_profile(flat, flat);
  if (!flat_profile || flat_profile->headroom != 0.0 || flat_profile->left != std::array<double, 6>{}) {
    std::cerr << "Equal thresholds must produce a flat profile\n";
    return 1;
  }

  const std::array<double, 6> left{-60, -58, -56, -54, -52, -50};
  const std::array<double, 6> right{-58, -56, -54, -52, -50, -20};
  const auto profile = hearing::make_profile(left, right);
  if (!profile || profile->left != std::array<double, 6>{0, 1, 2, 3, 4, 5} ||
      profile->right != std::array<double, 6>{1, 2, 3, 4, 5, 6} || profile->headroom != 21.0) {
    std::cerr << "Channel differences, gain cap, or headroom are incorrect\n";
    return 1;
  }

  if (hearing::make_profile(std::span(left).first(5), right) ||
      hearing::make_profile(left, std::span<const double>{})) {
    std::cerr << "Incomplete measurements must be rejected\n";
    return 1;
  }

  for (const auto invalid :
       {-81.0, -19.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity()}) {
    auto thresholds = flat;
    thresholds[2] = invalid;
    if (hearing::make_profile(thresholds, flat) || hearing::make_profile(flat, thresholds)) {
      std::cerr << "Invalid measurements must be rejected in either channel\n";
      return 1;
    }
  }

  const std::array<double, 6> limits{-80, -20, -20, -20, -20, -20};
  const auto maximum = hearing::make_profile(limits, limits);
  if (!maximum || maximum->headroom != 30.0 || maximum->left[5] != 6.0) {
    std::cerr << "Boundary measurements must retain bounded gain and sufficient headroom\n";
    return 1;
  }
}
