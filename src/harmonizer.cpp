#include "harmonizer.hpp"
#include <mutex>
#include "db_manager.hpp"
#include "easyeffects_db_harmonizer.h"
#include "lv2_macros.hpp"
#include "pipeline_type.hpp"
#include "tags_plugin_name.hpp"

Harmonizer::Harmonizer(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::harmonizer,
                 tags::plugin_package::Package::rakarrack,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(db::Manager::self().get_plugin_db<DbHarmonizer>(
          pipe_type,
          tags::plugin_name::BaseName::harmonizer + "#" + instance_id)) {
  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>("https://github.com/Stazed/rakarrack-plus#StereoHarm_no_mid");

  packageInstalled = lv2_wrapper->found_plugin;

  init_common_controls<DbHarmonizer>(settings);

  BIND_LV2_PORT("DRYWET", dryWet, setDryWet, DbHarmonizer::dryWetChanged);
  BIND_LV2_PORT("INTL", intervalLeft, setIntervalLeft, DbHarmonizer::intervalLeftChanged);
  BIND_LV2_PORT("INTR", intervalRight, setIntervalRight, DbHarmonizer::intervalRightChanged);
  BIND_LV2_PORT("NOTE", note, setNote, DbHarmonizer::noteChanged);
  BIND_LV2_PORT("CHORD", chord, setChord, DbHarmonizer::chordChanged);
  BIND_LV2_PORT("SELECT", chordMode, setChordMode, DbHarmonizer::chordModeChanged);
  BIND_LV2_PORT("GAINL", gainLeft, setGainLeft, DbHarmonizer::gainLeftChanged);
  BIND_LV2_PORT("GAINR", gainRight, setGainRight, DbHarmonizer::gainRightChanged);
}

Harmonizer::~Harmonizer() {
  stop_worker();

  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();
}

void Harmonizer::reset() {
  settings->setDefaults();
}

void Harmonizer::clear_data() {
  setup();
}

void Harmonizer::setup() {
  if (rate == 0 || n_samples == 0 || !lv2_wrapper->found_plugin) {
    return;
  }

  std::scoped_lock lock(data_mutex);

  lv2_wrapper->set_n_samples(n_samples);

  if (lv2_wrapper->has_instance() && rate == lv2_wrapper->get_rate()) {
    return;
  }

  ready = false;

  QMetaObject::invokeMethod(
      baseWorker,
      [this] {
        lv2_wrapper->create_instance(rate);

        std::scoped_lock lock(data_mutex);

        ready = true;
      },
      Qt::QueuedConnection);
}

void Harmonizer::process(std::span<float>& left_in,
                         std::span<float>& right_in,
                         std::span<float>& left_out,
                         std::span<float>& right_out) {
  std::scoped_lock lock(data_mutex);

  if (bypass || !ready) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());
    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  lv2_wrapper->connect_data_ports(left_in, right_in, left_out, right_out);
  lv2_wrapper->run();

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }
  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void Harmonizer::process([[maybe_unused]] std::span<float>& left_in,
                         [[maybe_unused]] std::span<float>& right_in,
                         [[maybe_unused]] std::span<float>& left_out,
                         [[maybe_unused]] std::span<float>& right_out,
                         [[maybe_unused]] std::span<float>& probe_left,
                         [[maybe_unused]] std::span<float>& probe_right) {}
