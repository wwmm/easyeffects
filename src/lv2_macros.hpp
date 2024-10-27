#pragma once

// NOLINTNEXTLINE
#define BIND_LV2_PORT(key, getter, setter, onChangedSignal)                                                    \
  lv2_wrapper->set_control_port_value(key, static_cast<float>(settings->getter()));                            \
  lv2_wrapper->sync_funcs.emplace_back([&]() { settings->setter(lv2_wrapper->get_control_port_value(key)); }); \
  connect(settings, &onChangedSignal,                                                                          \
          [&]() { lv2_wrapper->set_control_port_value(key, static_cast<float>(settings->getter())); });
