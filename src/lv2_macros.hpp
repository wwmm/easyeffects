#pragma once

// NOLINTBEGIN(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)
#define BIND_LV2_PORT(key, getter, setter, onChangedSignal)                                                      \
  {                                                                                                              \
    lv2_wrapper->set_control_port_value(key, static_cast<float>(settings->getter()));                            \
    lv2_wrapper->sync_funcs.emplace_back([&]() { settings->setter(lv2_wrapper->get_control_port_value(key)); }); \
    connect(settings, &onChangedSignal, [this]() {                                                               \
      if (this == nullptr || settings == nullptr || lv2_wrapper == nullptr) {                                    \
        return;                                                                                                  \
      }                                                                                                          \
      lv2_wrapper->set_control_port_value(key, static_cast<float>(settings->getter()));                          \
    });                                                                                                          \
  }

#define BIND_LV2_PORT_DB(key, getter, setter, onChangedSignal, enforceLowerBound)                               \
  {                                                                                                             \
    auto db_v = settings->getter();                                                                             \
    auto linear_v = ((enforceLowerBound) && db_v <= util::minimum_db_d_level)                                   \
                        ? 0.0F                                                                                  \
                        : static_cast<float>(util::db_to_linear(db_v));                                         \
    lv2_wrapper->set_control_port_value(key, linear_v);                                                         \
    lv2_wrapper->sync_funcs.emplace_back([this]() {                                                             \
      const auto linear_v = lv2_wrapper->get_control_port_value(key);                                           \
      const auto db_v =                                                                                         \
          ((enforceLowerBound) & (linear_v == 0.0F)) ? util::minimum_db_d_level : util::linear_to_db(linear_v); \
      settings->setter(db_v);                                                                                   \
    });                                                                                                         \
    connect(settings, &onChangedSignal, [this]() {                                                              \
      if (this == nullptr || settings == nullptr || lv2_wrapper == nullptr) {                                   \
        return;                                                                                                 \
      }                                                                                                         \
      auto db_v = settings->getter();                                                                           \
      auto linear_v = ((enforceLowerBound) && db_v <= util::minimum_db_d_level)                                 \
                          ? 0.0F                                                                                \
                          : static_cast<float>(util::db_to_linear(db_v));                                       \
      lv2_wrapper->set_control_port_value(key, linear_v);                                                       \
    });                                                                                                         \
  }
// NOLINTEND(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)