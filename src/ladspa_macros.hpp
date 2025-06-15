#pragma once

// NOLINTBEGIN(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)
#define BIND_LADSPA_PORT(key, getter, setter, onChangedSignal)                                                   \
  {                                                                                                              \
    auto actual_value = ladspa_wrapper->set_control_port_value_clamp(key, settings->getter());                   \
    if (actual_value != settings->getter() && !(std::isnan(actual_value) && std::isnan(settings->getter()))) {   \
      settings->setter(actual_value);                                                                            \
    }                                                                                                            \
    connect(settings, &onChangedSignal, [this]() {                                                               \
      if (this == nullptr || settings == nullptr || ladspa_wrapper == nullptr) {                                 \
        return;                                                                                                  \
      }                                                                                                          \
      auto actual_value = ladspa_wrapper->set_control_port_value_clamp(key, settings->getter());                 \
      if (actual_value != settings->getter() && !(std::isnan(actual_value) && std::isnan(settings->getter()))) { \
        settings->setter(actual_value);                                                                          \
      }                                                                                                          \
    });                                                                                                          \
  }

#define BIND_LADSPA_PORT_DB_EXPONENTIAL(key, getter, setter, onChangedSignal, enforceLowerBound)                    \
  {                                                                                                                 \
    auto db_v = settings->getter();                                                                                 \
    auto clamped =                                                                                                  \
        ((enforceLowerBound) && db_v <= util::minimum_db_d_level) ? -std::numeric_limits<float>::infinity() : db_v; \
    const auto new_v = ladspa_wrapper->set_control_port_value_clamp(key, clamped);                                  \
    if (new_v != clamped && !(std::isnan(new_v) && std::isnan(clamped))) {                                          \
      settings->setter(new_v);                                                                                      \
    }                                                                                                               \
    connect(settings, &onChangedSignal, [this]() {                                                                  \
      if (this == nullptr || settings == nullptr || ladspa_wrapper == nullptr) {                                    \
        return;                                                                                                     \
      }                                                                                                             \
      auto db_v = settings->getter();                                                                               \
      auto clamped = ((enforceLowerBound) && db_v <= util::minimum_db_d_level)                                      \
                         ? -std::numeric_limits<float>::infinity()                                                  \
                         : static_cast<float>(util::db_to_linear(db_v));                                            \
      const auto new_v = ladspa_wrapper->set_control_port_value_clamp(key, clamped);                                \
      if (new_v != clamped && !(std::isnan(new_v) && std::isnan(clamped))) {                                        \
        settings->setter(new_v);                                                                                    \
      }                                                                                                             \
    });                                                                                                             \
  }

#define BIND_LADSPA_PORT_DB(key, getter, setter, onChangedSignal, enforceLowerBound) \
  {                                                                                  \
    auto db_v = settings->getter();                                                  \
    auto clamped = ((enforceLowerBound) && db_v <= util::minimum_db_d_level)         \
                       ? -std::numeric_limits<float>::infinity()                     \
                       : static_cast<float>(util::db_to_linear(db_v));               \
    const auto new_v = ladspa_wrapper->set_control_port_value_clamp(key, clamped);   \
    if (new_v != clamped && !(std::isnan(new_v) && std::isnan(clamped))) {           \
      settings->setter(new_v);                                                       \
    }                                                                                \
    connect(settings, &onChangedSignal, [this]() {                                   \
      if (this == nullptr || settings == nullptr || ladspa_wrapper == nullptr) {     \
        return;                                                                      \
      }                                                                              \
      auto db_v = settings->getter();                                                \
      auto clamped = ((enforceLowerBound) && db_v <= util::minimum_db_d_level)       \
                         ? -std::numeric_limits<float>::infinity()                   \
                         : static_cast<float>(util::db_to_linear(db_v));             \
      const auto new_v = ladspa_wrapper->set_control_port_value_clamp(key, clamped); \
      if (new_v != clamped && !(std::isnan(new_v) && std::isnan(clamped))) {         \
        settings->setter(new_v);                                                     \
      }                                                                              \
    });                                                                              \
  }
// NOLINTEND(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)