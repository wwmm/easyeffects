#pragma once

#include <qqmlintegration.h>
#include <QString>
#include <memory>
#include <span>
#include <string>
#include "easyeffects_db_harmonizer.h"
#include "lv2_wrapper.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"

class Harmonizer : public PluginBase {
  Q_OBJECT
  QML_NAMED_ELEMENT(BackendHarmonizer)
  QML_UNCREATABLE("Use the c++ instance")

 public:
  Harmonizer(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  ~Harmonizer() override;
  void reset() override;
  void clear_data() override;
  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out,
               std::span<float>& probe_left,
               std::span<float>& probe_right) override;

 private:
  bool ready = false;
  std::unique_ptr<lv2::Lv2Wrapper> lv2_wrapper;
  DbHarmonizer* settings = nullptr;
};
