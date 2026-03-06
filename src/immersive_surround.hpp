#pragma once

#include <qtmetamacros.h>
#include <QString>
#include <span>
#include <string>
#include <vector>
#include "easyeffects_db_immersive_surround.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"

class ImmersiveSurround : public PluginBase {
  Q_OBJECT

 public:
  ImmersiveSurround(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  ImmersiveSurround(const ImmersiveSurround&) = delete;
  auto operator=(const ImmersiveSurround&) -> ImmersiveSurround& = delete;
  ImmersiveSurround(const ImmersiveSurround&&) = delete;
  auto operator=(const ImmersiveSurround&&) -> ImmersiveSurround& = delete;
  ~ImmersiveSurround() override;

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

  auto get_latency_seconds() -> float override;

 private:
  bool ready = false;

  uint max_delay_samples = 1U;
  uint current_delay_samples = 1U;
  uint delay_index = 0U;

  float surround_amount = 0.55F;
  float stereo_width = 1.2F;
  float rear_mix = 0.35F;
  float center_gain = 1.0F;
  float damping = 0.7F;
  bool rear_phase_invert = true;

  float rear_lp_l = 0.0F;
  float rear_lp_r = 0.0F;

  std::vector<float> delay_l;
  std::vector<float> delay_r;

  db::ImmersiveSurround* settings = nullptr;

  void update_delay_samples();
};
