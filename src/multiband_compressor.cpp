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

  lv2_wrapper->bind_key_bool(settings, "enable-band1", "cbe_1");
  lv2_wrapper->bind_key_bool(settings, "enable-band2", "cbe_2");
  lv2_wrapper->bind_key_bool(settings, "enable-band3", "cbe_3");
  lv2_wrapper->bind_key_bool(settings, "enable-band4", "cbe_4");
  lv2_wrapper->bind_key_bool(settings, "enable-band5", "cbe_5");
  lv2_wrapper->bind_key_bool(settings, "enable-band6", "cbe_6");
  lv2_wrapper->bind_key_bool(settings, "enable-band7", "cbe_7");

  lv2_wrapper->bind_key_double(settings, "split-frequency1", "sf_1");
  lv2_wrapper->bind_key_double(settings, "split-frequency2", "sf_2");
  lv2_wrapper->bind_key_double(settings, "split-frequency3", "sf_3");
  lv2_wrapper->bind_key_double(settings, "split-frequency4", "sf_4");
  lv2_wrapper->bind_key_double(settings, "split-frequency5", "sf_5");
  lv2_wrapper->bind_key_double(settings, "split-frequency6", "sf_6");
  lv2_wrapper->bind_key_double(settings, "split-frequency7", "sf_7");

  lv2_wrapper->bind_key_enum(settings, "compressor-mode0", "cm_0");
  lv2_wrapper->bind_key_enum(settings, "compressor-mode1", "cm_1");
  lv2_wrapper->bind_key_enum(settings, "compressor-mode2", "cm_2");
  lv2_wrapper->bind_key_enum(settings, "compressor-mode3", "cm_3");
  lv2_wrapper->bind_key_enum(settings, "compressor-mode4", "cm_4");
  lv2_wrapper->bind_key_enum(settings, "compressor-mode5", "cm_5");
  lv2_wrapper->bind_key_enum(settings, "compressor-mode6", "cm_6");
  lv2_wrapper->bind_key_enum(settings, "compressor-mode7", "cm_7");

  lv2_wrapper->bind_key_enum(settings, "sidechain-mode0", "scm_0");
  lv2_wrapper->bind_key_enum(settings, "sidechain-mode1", "scm_1");
  lv2_wrapper->bind_key_enum(settings, "sidechain-mode2", "scm_2");
  lv2_wrapper->bind_key_enum(settings, "sidechain-mode3", "scm_3");
  lv2_wrapper->bind_key_enum(settings, "sidechain-mode4", "scm_4");
  lv2_wrapper->bind_key_enum(settings, "sidechain-mode5", "scm_5");
  lv2_wrapper->bind_key_enum(settings, "sidechain-mode6", "scm_6");
  lv2_wrapper->bind_key_enum(settings, "sidechain-mode7", "scm_7");

  lv2_wrapper->bind_key_enum(settings, "sidechain-source0", "scs_0");
  lv2_wrapper->bind_key_enum(settings, "sidechain-source1", "scs_1");
  lv2_wrapper->bind_key_enum(settings, "sidechain-source2", "scs_2");
  lv2_wrapper->bind_key_enum(settings, "sidechain-source3", "scs_3");
  lv2_wrapper->bind_key_enum(settings, "sidechain-source4", "scs_4");
  lv2_wrapper->bind_key_enum(settings, "sidechain-source5", "scs_5");
  lv2_wrapper->bind_key_enum(settings, "sidechain-source6", "scs_6");
  lv2_wrapper->bind_key_enum(settings, "sidechain-source7", "scs_7");

  lv2_wrapper->bind_key_double(settings, "attack-time0", "at_0");
  lv2_wrapper->bind_key_double(settings, "attack-time1", "at_1");
  lv2_wrapper->bind_key_double(settings, "attack-time2", "at_2");
  lv2_wrapper->bind_key_double(settings, "attack-time3", "at_3");
  lv2_wrapper->bind_key_double(settings, "attack-time4", "at_4");
  lv2_wrapper->bind_key_double(settings, "attack-time5", "at_5");
  lv2_wrapper->bind_key_double(settings, "attack-time6", "at_6");
  lv2_wrapper->bind_key_double(settings, "attack-time7", "at_7");

  lv2_wrapper->bind_key_double_db(settings, "attack-threshold0", "al_0");
  lv2_wrapper->bind_key_double_db(settings, "attack-threshold1", "al_1");
  lv2_wrapper->bind_key_double_db(settings, "attack-threshold2", "al_2");
  lv2_wrapper->bind_key_double_db(settings, "attack-threshold3", "al_3");
  lv2_wrapper->bind_key_double_db(settings, "attack-threshold4", "al_4");
  lv2_wrapper->bind_key_double_db(settings, "attack-threshold5", "al_5");
  lv2_wrapper->bind_key_double_db(settings, "attack-threshold6", "al_6");
  lv2_wrapper->bind_key_double_db(settings, "attack-threshold7", "al_7");

  lv2_wrapper->bind_key_double(settings, "release-time0", "rt_0");
  lv2_wrapper->bind_key_double(settings, "release-time1", "rt_1");
  lv2_wrapper->bind_key_double(settings, "release-time2", "rt_2");
  lv2_wrapper->bind_key_double(settings, "release-time3", "rt_3");
  lv2_wrapper->bind_key_double(settings, "release-time4", "rt_4");
  lv2_wrapper->bind_key_double(settings, "release-time5", "rt_5");
  lv2_wrapper->bind_key_double(settings, "release-time6", "rt_6");
  lv2_wrapper->bind_key_double(settings, "release-time7", "rt_7");

  lv2_wrapper->bind_key_double_db(settings, "release-threshold0", "rrl_0");
  lv2_wrapper->bind_key_double_db(settings, "release-threshold1", "rrl_1");
  lv2_wrapper->bind_key_double_db(settings, "release-threshold2", "rrl_2");
  lv2_wrapper->bind_key_double_db(settings, "release-threshold3", "rrl_3");
  lv2_wrapper->bind_key_double_db(settings, "release-threshold4", "rrl_4");
  lv2_wrapper->bind_key_double_db(settings, "release-threshold5", "rrl_5");
  lv2_wrapper->bind_key_double_db(settings, "release-threshold6", "rrl_6");
  lv2_wrapper->bind_key_double_db(settings, "release-threshold7", "rrl_7");

  lv2_wrapper->bind_key_double(settings, "ratio0", "cr_0");
  lv2_wrapper->bind_key_double(settings, "ratio1", "cr_1");
  lv2_wrapper->bind_key_double(settings, "ratio2", "cr_2");
  lv2_wrapper->bind_key_double(settings, "ratio3", "cr_3");
  lv2_wrapper->bind_key_double(settings, "ratio4", "cr_4");
  lv2_wrapper->bind_key_double(settings, "ratio5", "cr_5");
  lv2_wrapper->bind_key_double(settings, "ratio6", "cr_6");
  lv2_wrapper->bind_key_double(settings, "ratio7", "cr_7");

  lv2_wrapper->bind_key_double_db(settings, "knee0", "kn_0");
  lv2_wrapper->bind_key_double_db(settings, "knee1", "kn_1");
  lv2_wrapper->bind_key_double_db(settings, "knee2", "kn_2");
  lv2_wrapper->bind_key_double_db(settings, "knee3", "kn_3");
  lv2_wrapper->bind_key_double_db(settings, "knee4", "kn_4");
  lv2_wrapper->bind_key_double_db(settings, "knee5", "kn_5");
  lv2_wrapper->bind_key_double_db(settings, "knee6", "kn_6");
  lv2_wrapper->bind_key_double_db(settings, "knee7", "kn_7");

  lv2_wrapper->bind_key_double_db(settings, "boost-amount0", "bsa_0");
  lv2_wrapper->bind_key_double_db(settings, "boost-amount1", "bsa_1");
  lv2_wrapper->bind_key_double_db(settings, "boost-amount2", "bsa_2");
  lv2_wrapper->bind_key_double_db(settings, "boost-amount3", "bsa_3");
  lv2_wrapper->bind_key_double_db(settings, "boost-amount4", "bsa_4");
  lv2_wrapper->bind_key_double_db(settings, "boost-amount5", "bsa_5");
  lv2_wrapper->bind_key_double_db(settings, "boost-amount6", "bsa_6");
  lv2_wrapper->bind_key_double_db(settings, "boost-amount7", "bsa_7");

  lv2_wrapper->bind_key_double_db(settings, "boost-threshold0", "bth_0");
  lv2_wrapper->bind_key_double_db(settings, "boost-threshold1", "bth_1");
  lv2_wrapper->bind_key_double_db(settings, "boost-threshold2", "bth_2");
  lv2_wrapper->bind_key_double_db(settings, "boost-threshold3", "bth_3");
  lv2_wrapper->bind_key_double_db(settings, "boost-threshold4", "bth_4");
  lv2_wrapper->bind_key_double_db(settings, "boost-threshold5", "bth_5");
  lv2_wrapper->bind_key_double_db(settings, "boost-threshold6", "bth_6");
  lv2_wrapper->bind_key_double_db(settings, "boost-threshold7", "bth_7");

  lv2_wrapper->bind_key_double_db(settings, "makeup0", "mk_0");
  lv2_wrapper->bind_key_double_db(settings, "makeup1", "mk_1");
  lv2_wrapper->bind_key_double_db(settings, "makeup2", "mk_2");
  lv2_wrapper->bind_key_double_db(settings, "makeup3", "mk_3");
  lv2_wrapper->bind_key_double_db(settings, "makeup4", "mk_4");
  lv2_wrapper->bind_key_double_db(settings, "makeup5", "mk_5");
  lv2_wrapper->bind_key_double_db(settings, "makeup6", "mk_6");
  lv2_wrapper->bind_key_double_db(settings, "makeup7", "mk_7");

  lv2_wrapper->bind_key_bool(settings, "highcut-filter-0", "schc_0");
  lv2_wrapper->bind_key_bool(settings, "highcut-filter-1", "schc_1");
  lv2_wrapper->bind_key_bool(settings, "highcut-filter-2", "schc_2");
  lv2_wrapper->bind_key_bool(settings, "highcut-filter-3", "schc_3");
  lv2_wrapper->bind_key_bool(settings, "highcut-filter-4", "schc_4");
  lv2_wrapper->bind_key_bool(settings, "highcut-filter-5", "schc_5");
  lv2_wrapper->bind_key_bool(settings, "highcut-filter-6", "schc_6");
  lv2_wrapper->bind_key_bool(settings, "highcut-filter-7", "schc_7");

  lv2_wrapper->bind_key_double(settings, "highcut-frequency-0", "schf_0");
  lv2_wrapper->bind_key_double(settings, "highcut-frequency-1", "schf_1");
  lv2_wrapper->bind_key_double(settings, "highcut-frequency-2", "schf_2");
  lv2_wrapper->bind_key_double(settings, "highcut-frequency-3", "schf_3");
  lv2_wrapper->bind_key_double(settings, "highcut-frequency-4", "schf_4");
  lv2_wrapper->bind_key_double(settings, "highcut-frequency-5", "schf_5");
  lv2_wrapper->bind_key_double(settings, "highcut-frequency-6", "schf_6");
  lv2_wrapper->bind_key_double(settings, "highcut-frequency-7", "schf_7");

  lv2_wrapper->bind_key_bool(settings, "lowcut-filter-0", "sclc_0");
  lv2_wrapper->bind_key_bool(settings, "lowcut-filter-1", "sclc_1");
  lv2_wrapper->bind_key_bool(settings, "lowcut-filter-2", "sclc_2");
  lv2_wrapper->bind_key_bool(settings, "lowcut-filter-3", "sclc_3");
  lv2_wrapper->bind_key_bool(settings, "lowcut-filter-4", "sclc_4");
  lv2_wrapper->bind_key_bool(settings, "lowcut-filter-5", "sclc_5");
  lv2_wrapper->bind_key_bool(settings, "lowcut-filter-6", "sclc_6");
  lv2_wrapper->bind_key_bool(settings, "lowcut-filter-7", "sclc_7");

  lv2_wrapper->bind_key_double(settings, "lowcut-frequency-0", "sclf_0");
  lv2_wrapper->bind_key_double(settings, "lowcut-frequency-1", "sclf_1");
  lv2_wrapper->bind_key_double(settings, "lowcut-frequency-2", "sclf_2");
  lv2_wrapper->bind_key_double(settings, "lowcut-frequency-3", "sclf_3");
  lv2_wrapper->bind_key_double(settings, "lowcut-frequency-4", "sclf_4");
  lv2_wrapper->bind_key_double(settings, "lowcut-frequency-5", "sclf_5");
  lv2_wrapper->bind_key_double(settings, "lowcut-frequency-6", "sclf_6");
  lv2_wrapper->bind_key_double(settings, "lowcut-frequency-7", "sclf_7");

  lv2_wrapper->bind_key_bool(settings, "mute0", "bm_0");
  lv2_wrapper->bind_key_bool(settings, "mute1", "bm_1");
  lv2_wrapper->bind_key_bool(settings, "mute2", "bm_2");
  lv2_wrapper->bind_key_bool(settings, "mute3", "bm_3");
  lv2_wrapper->bind_key_bool(settings, "mute4", "bm_4");
  lv2_wrapper->bind_key_bool(settings, "mute5", "bm_5");
  lv2_wrapper->bind_key_bool(settings, "mute6", "bm_6");
  lv2_wrapper->bind_key_bool(settings, "mute7", "bm_7");

  lv2_wrapper->bind_key_bool(settings, "solo0", "bs_0");
  lv2_wrapper->bind_key_bool(settings, "solo1", "bs_1");
  lv2_wrapper->bind_key_bool(settings, "solo2", "bs_2");
  lv2_wrapper->bind_key_bool(settings, "solo3", "bs_3");
  lv2_wrapper->bind_key_bool(settings, "solo4", "bs_4");
  lv2_wrapper->bind_key_bool(settings, "solo5", "bs_5");
  lv2_wrapper->bind_key_bool(settings, "solo6", "bs_6");
  lv2_wrapper->bind_key_bool(settings, "solo7", "bs_7");

  lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp-0", "scp_0");
  lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp-1", "scp_1");
  lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp-2", "scp_2");
  lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp-3", "scp_3");
  lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp-4", "scp_4");
  lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp-5", "scp_5");
  lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp-6", "scp_6");
  lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp-7", "scp_7");

  lv2_wrapper->bind_key_double(settings, "sidechain-reactivity-0", "scr_0");
  lv2_wrapper->bind_key_double(settings, "sidechain-reactivity-1", "scr_1");
  lv2_wrapper->bind_key_double(settings, "sidechain-reactivity-2", "scr_2");
  lv2_wrapper->bind_key_double(settings, "sidechain-reactivity-3", "scr_3");
  lv2_wrapper->bind_key_double(settings, "sidechain-reactivity-4", "scr_4");
  lv2_wrapper->bind_key_double(settings, "sidechain-reactivity-5", "scr_5");
  lv2_wrapper->bind_key_double(settings, "sidechain-reactivity-6", "scr_6");
  lv2_wrapper->bind_key_double(settings, "sidechain-reactivity-7", "scr_7");

  lv2_wrapper->bind_key_double(settings, "sidechain-lookahead-0", "sla_0");
  lv2_wrapper->bind_key_double(settings, "sidechain-lookahead-1", "sla_1");
  lv2_wrapper->bind_key_double(settings, "sidechain-lookahead-2", "sla_2");
  lv2_wrapper->bind_key_double(settings, "sidechain-lookahead-3", "sla_3");
  lv2_wrapper->bind_key_double(settings, "sidechain-lookahead-4", "sla_4");
  lv2_wrapper->bind_key_double(settings, "sidechain-lookahead-5", "sla_5");
  lv2_wrapper->bind_key_double(settings, "sidechain-lookahead-6", "sla_6");
  lv2_wrapper->bind_key_double(settings, "sidechain-lookahead-7", "sla_7");

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

      float compression0_value = lv2_wrapper->get_control_port_value("rlm_0");
      float compression1_value = lv2_wrapper->get_control_port_value("rlm_1");
      float compression2_value = lv2_wrapper->get_control_port_value("rlm_2");
      float compression3_value = lv2_wrapper->get_control_port_value("rlm_3");
      float compression4_value = lv2_wrapper->get_control_port_value("rlm_4");
      float compression5_value = lv2_wrapper->get_control_port_value("rlm_5");
      float compression6_value = lv2_wrapper->get_control_port_value("rlm_6");
      float compression7_value = lv2_wrapper->get_control_port_value("rlm_7");

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
