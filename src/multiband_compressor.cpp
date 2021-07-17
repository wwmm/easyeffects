/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "multiband_compressor.hpp"

MultibandCompressor::MultibandCompressor(const std::string& tag,
                                         const std::string& schema,
                                         const std::string& schema_path,
                                         PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::multiband_compressor, schema, schema_path, pipe_manager),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://lsp-plug.in/plugins/lv2/mb_compressor_stereo")) {
  if (!lv2_wrapper->found_plugin) {
    util::warning(log_tag + "http://lsp-plug.in/plugins/lv2/mb_compressor_stereo is not installed");
  }

  input_gain = static_cast<float>(util::db_to_linear(settings->get_double("input-gain")));
  output_gain = static_cast<float>(util::db_to_linear(settings->get_double("output-gain")));

  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });

  lv2_wrapper->bind_key_enum(settings, "operating-mode", "mode");

  lv2_wrapper->bind_key_enum(settings, "sidechain-boost", "envb");

  lv2_wrapper->bind_key_bool(settings, "enable-band1", "cbe-1");
  lv2_wrapper->bind_key_bool(settings, "enable-band2", "cbe-2");
  lv2_wrapper->bind_key_bool(settings, "enable-band3", "cbe-3");
  lv2_wrapper->bind_key_bool(settings, "enable-band4", "cbe-4");
  lv2_wrapper->bind_key_bool(settings, "enable-band5", "cbe-5");
  lv2_wrapper->bind_key_bool(settings, "enable-band6", "cbe-6");
  lv2_wrapper->bind_key_bool(settings, "enable-band7", "cbe-7");

  lv2_wrapper->bind_key_double(settings, "split-frequency1", "sf-1");
  lv2_wrapper->bind_key_double(settings, "split-frequency2", "sf-2");
  lv2_wrapper->bind_key_double(settings, "split-frequency3", "sf-3");
  lv2_wrapper->bind_key_double(settings, "split-frequency4", "sf-4");
  lv2_wrapper->bind_key_double(settings, "split-frequency5", "sf-5");
  lv2_wrapper->bind_key_double(settings, "split-frequency6", "sf-6");
  lv2_wrapper->bind_key_double(settings, "split-frequency7", "sf-7");

  lv2_wrapper->bind_key_enum(settings, "compressor-mode0", "cm-0");
  lv2_wrapper->bind_key_enum(settings, "compressor-mode1", "cm-1");
  lv2_wrapper->bind_key_enum(settings, "compressor-mode2", "cm-2");
  lv2_wrapper->bind_key_enum(settings, "compressor-mode3", "cm-3");
  lv2_wrapper->bind_key_enum(settings, "compressor-mode4", "cm-4");
  lv2_wrapper->bind_key_enum(settings, "compressor-mode5", "cm-5");
  lv2_wrapper->bind_key_enum(settings, "compressor-mode6", "cm-6");
  lv2_wrapper->bind_key_enum(settings, "compressor-mode7", "cm-7");

  lv2_wrapper->bind_key_enum(settings, "sidechain-mode0", "scm-0");
  lv2_wrapper->bind_key_enum(settings, "sidechain-mode1", "scm-1");
  lv2_wrapper->bind_key_enum(settings, "sidechain-mode2", "scm-2");
  lv2_wrapper->bind_key_enum(settings, "sidechain-mode3", "scm-3");
  lv2_wrapper->bind_key_enum(settings, "sidechain-mode4", "scm-4");
  lv2_wrapper->bind_key_enum(settings, "sidechain-mode5", "scm-5");
  lv2_wrapper->bind_key_enum(settings, "sidechain-mode6", "scm-6");
  lv2_wrapper->bind_key_enum(settings, "sidechain-mode7", "scm-7");

  lv2_wrapper->bind_key_enum(settings, "sidechain-source0", "scs-0");
  lv2_wrapper->bind_key_enum(settings, "sidechain-source1", "scs-1");
  lv2_wrapper->bind_key_enum(settings, "sidechain-source2", "scs-2");
  lv2_wrapper->bind_key_enum(settings, "sidechain-source3", "scs-3");
  lv2_wrapper->bind_key_enum(settings, "sidechain-source4", "scs-4");
  lv2_wrapper->bind_key_enum(settings, "sidechain-source5", "scs-5");
  lv2_wrapper->bind_key_enum(settings, "sidechain-source6", "scs-6");
  lv2_wrapper->bind_key_enum(settings, "sidechain-source7", "scs-7");

  lv2_wrapper->bind_key_double(settings, "attack-time0", "at-0");
  lv2_wrapper->bind_key_double(settings, "attack-time1", "at-1");
  lv2_wrapper->bind_key_double(settings, "attack-time2", "at-2");
  lv2_wrapper->bind_key_double(settings, "attack-time3", "at-3");
  lv2_wrapper->bind_key_double(settings, "attack-time4", "at-4");
  lv2_wrapper->bind_key_double(settings, "attack-time5", "at-5");
  lv2_wrapper->bind_key_double(settings, "attack-time6", "at-6");
  lv2_wrapper->bind_key_double(settings, "attack-time7", "at-7");

  lv2_wrapper->bind_key_double_db(settings, "attack-threshold0", "al-0");
  lv2_wrapper->bind_key_double_db(settings, "attack-threshold1", "al-1");
  lv2_wrapper->bind_key_double_db(settings, "attack-threshold2", "al-2");
  lv2_wrapper->bind_key_double_db(settings, "attack-threshold3", "al-3");
  lv2_wrapper->bind_key_double_db(settings, "attack-threshold4", "al-4");
  lv2_wrapper->bind_key_double_db(settings, "attack-threshold5", "al-5");
  lv2_wrapper->bind_key_double_db(settings, "attack-threshold6", "al-6");
  lv2_wrapper->bind_key_double_db(settings, "attack-threshold7", "al-7");

  lv2_wrapper->bind_key_double(settings, "release-time0", "rt-0");
  lv2_wrapper->bind_key_double(settings, "release-time1", "rt-1");
  lv2_wrapper->bind_key_double(settings, "release-time2", "rt-2");
  lv2_wrapper->bind_key_double(settings, "release-time3", "rt-3");
  lv2_wrapper->bind_key_double(settings, "release-time4", "rt-4");
  lv2_wrapper->bind_key_double(settings, "release-time5", "rt-5");
  lv2_wrapper->bind_key_double(settings, "release-time6", "rt-6");
  lv2_wrapper->bind_key_double(settings, "release-time7", "rt-7");

  lv2_wrapper->bind_key_double_db(settings, "release-threshold0", "rrl-0");
  lv2_wrapper->bind_key_double_db(settings, "release-threshold1", "rrl-1");
  lv2_wrapper->bind_key_double_db(settings, "release-threshold2", "rrl-2");
  lv2_wrapper->bind_key_double_db(settings, "release-threshold3", "rrl-3");
  lv2_wrapper->bind_key_double_db(settings, "release-threshold4", "rrl-4");
  lv2_wrapper->bind_key_double_db(settings, "release-threshold5", "rrl-5");
  lv2_wrapper->bind_key_double_db(settings, "release-threshold6", "rrl-6");
  lv2_wrapper->bind_key_double_db(settings, "release-threshold7", "rrl-7");

  lv2_wrapper->bind_key_double(settings, "ratio0", "cr-0");
  lv2_wrapper->bind_key_double(settings, "ratio1", "cr-1");
  lv2_wrapper->bind_key_double(settings, "ratio2", "cr-2");
  lv2_wrapper->bind_key_double(settings, "ratio3", "cr-3");
  lv2_wrapper->bind_key_double(settings, "ratio4", "cr-4");
  lv2_wrapper->bind_key_double(settings, "ratio5", "cr-5");
  lv2_wrapper->bind_key_double(settings, "ratio6", "cr-6");
  lv2_wrapper->bind_key_double(settings, "ratio7", "cr-7");

  lv2_wrapper->bind_key_double_db(settings, "knee0", "kn-0");
  lv2_wrapper->bind_key_double_db(settings, "knee1", "kn-1");
  lv2_wrapper->bind_key_double_db(settings, "knee2", "kn-2");
  lv2_wrapper->bind_key_double_db(settings, "knee3", "kn-3");
  lv2_wrapper->bind_key_double_db(settings, "knee4", "kn-4");
  lv2_wrapper->bind_key_double_db(settings, "knee5", "kn-5");
  lv2_wrapper->bind_key_double_db(settings, "knee6", "kn-6");
  lv2_wrapper->bind_key_double_db(settings, "knee7", "kn-7");

  lv2_wrapper->bind_key_double_db(settings, "boost-amount0", "bsa-0");
  lv2_wrapper->bind_key_double_db(settings, "boost-amount1", "bsa-1");
  lv2_wrapper->bind_key_double_db(settings, "boost-amount2", "bsa-2");
  lv2_wrapper->bind_key_double_db(settings, "boost-amount3", "bsa-3");
  lv2_wrapper->bind_key_double_db(settings, "boost-amount4", "bsa-4");
  lv2_wrapper->bind_key_double_db(settings, "boost-amount5", "bsa-5");
  lv2_wrapper->bind_key_double_db(settings, "boost-amount6", "bsa-6");
  lv2_wrapper->bind_key_double_db(settings, "boost-amount7", "bsa-7");

  lv2_wrapper->bind_key_double_db(settings, "boost-threshold0", "bth-0");
  lv2_wrapper->bind_key_double_db(settings, "boost-threshold1", "bth-1");
  lv2_wrapper->bind_key_double_db(settings, "boost-threshold2", "bth-2");
  lv2_wrapper->bind_key_double_db(settings, "boost-threshold3", "bth-3");
  lv2_wrapper->bind_key_double_db(settings, "boost-threshold4", "bth-4");
  lv2_wrapper->bind_key_double_db(settings, "boost-threshold5", "bth-5");
  lv2_wrapper->bind_key_double_db(settings, "boost-threshold6", "bth-6");
  lv2_wrapper->bind_key_double_db(settings, "boost-threshold7", "bth-7");

  lv2_wrapper->bind_key_double_db(settings, "makeup0", "mk-0");
  lv2_wrapper->bind_key_double_db(settings, "makeup1", "mk-1");
  lv2_wrapper->bind_key_double_db(settings, "makeup2", "mk-2");
  lv2_wrapper->bind_key_double_db(settings, "makeup3", "mk-3");
  lv2_wrapper->bind_key_double_db(settings, "makeup4", "mk-4");
  lv2_wrapper->bind_key_double_db(settings, "makeup5", "mk-5");
  lv2_wrapper->bind_key_double_db(settings, "makeup6", "mk-6");
  lv2_wrapper->bind_key_double_db(settings, "makeup7", "mk-7");

  lv2_wrapper->bind_key_bool(settings, "highcut-filter-0", "schc-0");
  lv2_wrapper->bind_key_bool(settings, "highcut-filter-1", "schc-1");
  lv2_wrapper->bind_key_bool(settings, "highcut-filter-2", "schc-2");
  lv2_wrapper->bind_key_bool(settings, "highcut-filter-3", "schc-3");
  lv2_wrapper->bind_key_bool(settings, "highcut-filter-4", "schc-4");
  lv2_wrapper->bind_key_bool(settings, "highcut-filter-5", "schc-5");
  lv2_wrapper->bind_key_bool(settings, "highcut-filter-6", "schc-6");
  lv2_wrapper->bind_key_bool(settings, "highcut-filter-7", "schc-7");

  lv2_wrapper->bind_key_double(settings, "highcut-frequency-0", "schf-0");
  lv2_wrapper->bind_key_double(settings, "highcut-frequency-1", "schf-1");
  lv2_wrapper->bind_key_double(settings, "highcut-frequency-2", "schf-2");
  lv2_wrapper->bind_key_double(settings, "highcut-frequency-3", "schf-3");
  lv2_wrapper->bind_key_double(settings, "highcut-frequency-4", "schf-4");
  lv2_wrapper->bind_key_double(settings, "highcut-frequency-5", "schf-5");
  lv2_wrapper->bind_key_double(settings, "highcut-frequency-6", "schf-6");
  lv2_wrapper->bind_key_double(settings, "highcut-frequency-7", "schf-7");

  lv2_wrapper->bind_key_bool(settings, "lowcut-filter-0", "sclc-0");
  lv2_wrapper->bind_key_bool(settings, "lowcut-filter-1", "sclc-1");
  lv2_wrapper->bind_key_bool(settings, "lowcut-filter-2", "sclc-2");
  lv2_wrapper->bind_key_bool(settings, "lowcut-filter-3", "sclc-3");
  lv2_wrapper->bind_key_bool(settings, "lowcut-filter-4", "sclc-4");
  lv2_wrapper->bind_key_bool(settings, "lowcut-filter-5", "sclc-5");
  lv2_wrapper->bind_key_bool(settings, "lowcut-filter-6", "sclc-6");
  lv2_wrapper->bind_key_bool(settings, "lowcut-filter-7", "sclc-7");

  lv2_wrapper->bind_key_double(settings, "lowcut-frequency-0", "sclf-0");
  lv2_wrapper->bind_key_double(settings, "lowcut-frequency-1", "sclf-1");
  lv2_wrapper->bind_key_double(settings, "lowcut-frequency-2", "sclf-2");
  lv2_wrapper->bind_key_double(settings, "lowcut-frequency-3", "sclf-3");
  lv2_wrapper->bind_key_double(settings, "lowcut-frequency-4", "sclf-4");
  lv2_wrapper->bind_key_double(settings, "lowcut-frequency-5", "sclf-5");
  lv2_wrapper->bind_key_double(settings, "lowcut-frequency-6", "sclf-6");
  lv2_wrapper->bind_key_double(settings, "lowcut-frequency-7", "sclf-7");

  lv2_wrapper->bind_key_bool(settings, "mute0", "bm-0");
  lv2_wrapper->bind_key_bool(settings, "mute1", "bm-1");
  lv2_wrapper->bind_key_bool(settings, "mute2", "bm-2");
  lv2_wrapper->bind_key_bool(settings, "mute3", "bm-3");
  lv2_wrapper->bind_key_bool(settings, "mute4", "bm-4");
  lv2_wrapper->bind_key_bool(settings, "mute5", "bm-5");
  lv2_wrapper->bind_key_bool(settings, "mute6", "bm-6");
  lv2_wrapper->bind_key_bool(settings, "mute7", "bm-7");

  lv2_wrapper->bind_key_bool(settings, "solo0", "bs-0");
  lv2_wrapper->bind_key_bool(settings, "solo1", "bs-1");
  lv2_wrapper->bind_key_bool(settings, "solo2", "bs-2");
  lv2_wrapper->bind_key_bool(settings, "solo3", "bs-3");
  lv2_wrapper->bind_key_bool(settings, "solo4", "bs-4");
  lv2_wrapper->bind_key_bool(settings, "solo5", "bs-5");
  lv2_wrapper->bind_key_bool(settings, "solo6", "bs-6");
  lv2_wrapper->bind_key_bool(settings, "solo7", "bs-7");

  lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp-0", "scp-0");
  lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp-1", "scp-1");
  lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp-2", "scp-2");
  lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp-3", "scp-3");
  lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp-4", "scp-4");
  lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp-5", "scp-5");
  lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp-6", "scp-6");
  lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp-7", "scp-7");

  lv2_wrapper->bind_key_double(settings, "sidechain-reactivity-0", "scr-0");
  lv2_wrapper->bind_key_double(settings, "sidechain-reactivity-1", "scr-1");
  lv2_wrapper->bind_key_double(settings, "sidechain-reactivity-2", "scr-2");
  lv2_wrapper->bind_key_double(settings, "sidechain-reactivity-3", "scr-3");
  lv2_wrapper->bind_key_double(settings, "sidechain-reactivity-4", "scr-4");
  lv2_wrapper->bind_key_double(settings, "sidechain-reactivity-5", "scr-5");
  lv2_wrapper->bind_key_double(settings, "sidechain-reactivity-6", "scr-6");
  lv2_wrapper->bind_key_double(settings, "sidechain-reactivity-7", "scr-7");

  lv2_wrapper->bind_key_double(settings, "sidechain-lookahead-0", "sla-0");
  lv2_wrapper->bind_key_double(settings, "sidechain-lookahead-1", "sla-1");
  lv2_wrapper->bind_key_double(settings, "sidechain-lookahead-2", "sla-2");
  lv2_wrapper->bind_key_double(settings, "sidechain-lookahead-3", "sla-3");
  lv2_wrapper->bind_key_double(settings, "sidechain-lookahead-4", "sla-4");
  lv2_wrapper->bind_key_double(settings, "sidechain-lookahead-5", "sla-5");
  lv2_wrapper->bind_key_double(settings, "sidechain-lookahead-6", "sla-6");
  lv2_wrapper->bind_key_double(settings, "sidechain-lookahead-7", "sla-7");

  initialize_listener();
}

MultibandCompressor::~MultibandCompressor() {
  util::debug(log_tag + name + " destroyed");

  pw_thread_loop_lock(pm->thread_loop);

  pw_filter_set_active(filter, false);

  pw_filter_disconnect(filter);

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);
}

void MultibandCompressor::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);
  lv2_wrapper->create_instance(rate);
}

void MultibandCompressor::process(std::span<float>& left_in,
                                  std::span<float>& right_in,
                                  std::span<float>& left_out,
                                  std::span<float>& right_out) {
  if (!lv2_wrapper->found_plugin || !lv2_wrapper->has_instance() || bypass) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  apply_gain(left_in, right_in, input_gain);

  lv2_wrapper->connect_data_ports(left_in, right_in, left_out, right_out);
  lv2_wrapper->run();

  apply_gain(left_out, right_out, output_gain);

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += sample_duration;

    if (notification_dt >= notification_time_window) {
      /*float output0_value = lv2_wrapper->get_control_port_value("output0");
      float output1_value = lv2_wrapper->get_control_port_value("output1");
      float output2_value = lv2_wrapper->get_control_port_value("output2");
      float output3_value = lv2_wrapper->get_control_port_value("output3");*/

      float compression0_value = lv2_wrapper->get_control_port_value("rlm-0");
      float compression1_value = lv2_wrapper->get_control_port_value("rlm-1");
      float compression2_value = lv2_wrapper->get_control_port_value("rlm-2");
      float compression3_value = lv2_wrapper->get_control_port_value("rlm-3");
      float compression4_value = lv2_wrapper->get_control_port_value("rlm-4");
      float compression5_value = lv2_wrapper->get_control_port_value("rlm-5");
      float compression6_value = lv2_wrapper->get_control_port_value("rlm-6");
      float compression7_value = lv2_wrapper->get_control_port_value("rlm-7");

      Glib::signal_idle().connect_once([=, this] {
        /*output0.emit(output0_value);
        output1.emit(output1_value);
        output2.emit(output2_value);
        output3.emit(output3_value);*/

        compression0.emit(compression0_value);
        compression1.emit(compression1_value);
        compression2.emit(compression2_value);
        compression3.emit(compression3_value);
        compression4.emit(compression0_value);
        compression5.emit(compression1_value);
        compression6.emit(compression2_value);
        compression7.emit(compression3_value);
      });

      notify();

      notification_dt = 0.0F;
    }
  }
}
