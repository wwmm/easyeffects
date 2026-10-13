#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <span>

namespace hearing {

inline constexpr std::array frequencies{250, 500, 1000, 2000, 4000, 8000};

struct Profile {
  std::array<double, frequencies.size()> left{};
  std::array<double, frequencies.size()> right{};
  double headroom = 0.0;
};

inline auto make_profile(std::span<const double> left, std::span<const double> right) -> std::optional<Profile> {
  const auto valid = [](std::span<const double> thresholds) {
    return thresholds.size() == frequencies.size() && std::ranges::all_of(thresholds, [](double value) {
             return std::isfinite(value) && value >= -80.0 && value <= -20.0;
           });
  };

  if (!valid(left) || !valid(right)) {
    return std::nullopt;
  }

  Profile profile;
  const auto minimum = std::min(std::ranges::min(left), std::ranges::min(right));
  const auto fill = [minimum](std::span<const double> thresholds, auto& gains) {
    double total = 0.0;
    for (std::size_t n = 0; n < thresholds.size(); ++n) {
      gains[n] = std::clamp(0.5 * (thresholds[n] - minimum), 0.0, 6.0);
      total += gains[n];
    }
    return total;
  };

  const auto left_headroom = fill(left, profile.left);
  const auto right_headroom = fill(right, profile.right);
  profile.headroom = std::max(left_headroom, right_headroom);
  return profile;
}

}
