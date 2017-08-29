# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gst', '1.0')
from gi.repository import GLib, Gtk


class EffectsUiBase():

    def __init__(self, builder_path, settings):
        self.module_path = os.path.dirname(__file__)

        self.settings = settings

        self.builder = Gtk.Builder()

        self.builder.add_from_file(self.module_path + builder_path)

        self.ui_window = self.builder.get_object('window')

        self.old_compressor_gain_reduction = 0

        # limiter widgets

        self.ui_limiter_input_gain = self.builder.get_object(
            'limiter_input_gain')
        self.ui_limiter_limit = self.builder.get_object(
            'limiter_limit')
        self.ui_limiter_release_time = self.builder.get_object(
            'limiter_release_time')
        self.ui_limiter_attenuation_levelbar = self.builder.get_object(
            'limiter_attenuation_levelbar')

        self.ui_limiter_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_LOW', 20)
        self.ui_limiter_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_HIGH', 50)
        self.ui_limiter_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_FULL', 70)

        self.ui_limiter_input_level_left = self.builder.get_object(
            'limiter_input_level_left')
        self.ui_limiter_input_level_right = self.builder.get_object(
            'limiter_input_level_right')
        self.ui_limiter_output_level_left = self.builder.get_object(
            'limiter_output_level_left')
        self.ui_limiter_output_level_right = self.builder.get_object(
            'limiter_output_level_right')

        self.ui_limiter_input_level_left_label = self.builder.get_object(
            'limiter_input_level_left_label')
        self.ui_limiter_input_level_right_label = self.builder.get_object(
            'limiter_input_level_right_label')
        self.ui_limiter_output_level_left_label = self.builder.get_object(
            'limiter_output_level_left_label')
        self.ui_limiter_output_level_right_label = self.builder.get_object(
            'limiter_output_level_right_label')
        self.ui_limiter_attenuation_level_label = self.builder.get_object(
            'limiter_attenuation_level_label')

        # compressor widgets

        self.ui_compressor_rms = self.builder.get_object('compressor_rms')
        self.ui_compressor_peak = self.builder.get_object('compressor_peak')
        self.ui_compressor_attack = self.builder.get_object(
            'compressor_attack')
        self.ui_compressor_release = self.builder.get_object(
            'compressor_release')
        self.ui_compressor_threshold = self.builder.get_object(
            'compressor_threshold')
        self.ui_compressor_ratio = self.builder.get_object(
            'compressor_ratio')
        self.ui_compressor_knee = self.builder.get_object('compressor_knee')
        self.ui_compressor_makeup = self.builder.get_object(
            'compressor_makeup')

        self.ui_compressor_input_level_left = self.builder.get_object(
            'compressor_input_level_left')
        self.ui_compressor_input_level_right = self.builder.get_object(
            'compressor_input_level_right')
        self.ui_compressor_output_level_left = self.builder.get_object(
            'compressor_output_level_left')
        self.ui_compressor_output_level_right = self.builder.get_object(
            'compressor_output_level_right')
        self.ui_compressor_gain_reduction_levelbar = self.builder.get_object(
            'compressor_gain_reduction_levelbar')

        self.ui_compressor_input_level_left_label = self.builder.get_object(
            'compressor_input_level_left_label')
        self.ui_compressor_input_level_right_label = self.builder.get_object(
            'compressor_input_level_right_label')
        self.ui_compressor_output_level_left_label = self.builder.get_object(
            'compressor_output_level_left_label')
        self.ui_compressor_output_level_right_label = self.builder.get_object(
            'compressor_output_level_right_label')
        self.ui_compressor_gain_reduction_level_label = \
            self.builder.get_object('compressor_gain_reduction_level_label')

        self.ui_compressor_gain_reduction_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_LOW', 6)
        self.ui_compressor_gain_reduction_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_HIGH', 18)
        self.ui_compressor_gain_reduction_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_FULL', 24)

        # reverb

        self.ui_reverb_room_size = self.builder.get_object(
            'reverb_room_size')
        self.ui_reverb_damping = self.builder.get_object('reverb_damping')
        self.ui_reverb_width = self.builder.get_object('reverb_width')
        self.ui_reverb_level = self.builder.get_object('reverb_level')

        self.ui_reverb_input_level_left = self.builder.get_object(
            'reverb_input_level_left')
        self.ui_reverb_input_level_right = self.builder.get_object(
            'reverb_input_level_right')
        self.ui_reverb_output_level_left = self.builder.get_object(
            'reverb_output_level_left')
        self.ui_reverb_output_level_right = self.builder.get_object(
            'reverb_output_level_right')

        self.ui_reverb_input_level_left_label = self.builder.get_object(
            'reverb_input_level_left_label')
        self.ui_reverb_input_level_right_label = self.builder.get_object(
            'reverb_input_level_right_label')
        self.ui_reverb_output_level_left_label = self.builder.get_object(
            'reverb_output_level_left_label')
        self.ui_reverb_output_level_right_label = self.builder.get_object(
            'reverb_output_level_right_label')

        # highpass

        self.ui_highpass_cutoff = self.builder.get_object('highpass_cutoff')
        self.ui_highpass_poles = self.builder.get_object('highpass_poles')

        self.ui_highpass_input_level_left = self.builder.get_object(
            'highpass_input_level_left')
        self.ui_highpass_input_level_right = self.builder.get_object(
            'highpass_input_level_right')
        self.ui_highpass_output_level_left = self.builder.get_object(
            'highpass_output_level_left')
        self.ui_highpass_output_level_right = self.builder.get_object(
            'highpass_output_level_right')

        self.ui_highpass_input_level_left_label = self.builder.get_object(
            'highpass_input_level_left_label')
        self.ui_highpass_input_level_right_label = self.builder.get_object(
            'highpass_input_level_right_label')
        self.ui_highpass_output_level_left_label = self.builder.get_object(
            'highpass_output_level_left_label')
        self.ui_highpass_output_level_right_label = self.builder.get_object(
            'highpass_output_level_right_label')

        # lowpass

        self.ui_lowpass_cutoff = self.builder.get_object('lowpass_cutoff')
        self.ui_lowpass_poles = self.builder.get_object('lowpass_poles')

        self.ui_lowpass_input_level_left = self.builder.get_object(
            'lowpass_input_level_left')
        self.ui_lowpass_input_level_right = self.builder.get_object(
            'lowpass_input_level_right')
        self.ui_lowpass_output_level_left = self.builder.get_object(
            'lowpass_output_level_left')
        self.ui_lowpass_output_level_right = self.builder.get_object(
            'lowpass_output_level_right')

        self.ui_lowpass_input_level_left_label = self.builder.get_object(
            'lowpass_input_level_left_label')
        self.ui_lowpass_input_level_right_label = self.builder.get_object(
            'lowpass_input_level_right_label')
        self.ui_lowpass_output_level_left_label = self.builder.get_object(
            'lowpass_output_level_left_label')
        self.ui_lowpass_output_level_right_label = self.builder.get_object(
            'lowpass_output_level_right_label')

        # equalizer

        self.ui_equalizer_input_gain = self.builder.get_object(
            'equalizer_input_gain')
        self.ui_equalizer_output_gain = self.builder.get_object(
            'equalizer_output_gain')

        self.ui_eq_band0 = self.builder.get_object('eq_band0')
        self.ui_eq_band1 = self.builder.get_object('eq_band1')
        self.ui_eq_band2 = self.builder.get_object('eq_band2')
        self.ui_eq_band3 = self.builder.get_object('eq_band3')
        self.ui_eq_band4 = self.builder.get_object('eq_band4')
        self.ui_eq_band5 = self.builder.get_object('eq_band5')
        self.ui_eq_band6 = self.builder.get_object('eq_band6')
        self.ui_eq_band7 = self.builder.get_object('eq_band7')
        self.ui_eq_band8 = self.builder.get_object('eq_band8')
        self.ui_eq_band9 = self.builder.get_object('eq_band9')
        self.ui_eq_band10 = self.builder.get_object('eq_band10')
        self.ui_eq_band11 = self.builder.get_object('eq_band11')
        self.ui_eq_band12 = self.builder.get_object('eq_band12')
        self.ui_eq_band13 = self.builder.get_object('eq_band13')
        self.ui_eq_band14 = self.builder.get_object('eq_band14')

        self.ui_eq_band0_freq = self.builder.get_object('eq_band0_freq')
        self.ui_eq_band1_freq = self.builder.get_object('eq_band1_freq')
        self.ui_eq_band2_freq = self.builder.get_object('eq_band2_freq')
        self.ui_eq_band3_freq = self.builder.get_object('eq_band3_freq')
        self.ui_eq_band4_freq = self.builder.get_object('eq_band4_freq')
        self.ui_eq_band5_freq = self.builder.get_object('eq_band5_freq')
        self.ui_eq_band6_freq = self.builder.get_object('eq_band6_freq')
        self.ui_eq_band7_freq = self.builder.get_object('eq_band7_freq')
        self.ui_eq_band8_freq = self.builder.get_object('eq_band8_freq')
        self.ui_eq_band9_freq = self.builder.get_object('eq_band9_freq')
        self.ui_eq_band10_freq = self.builder.get_object('eq_band10_freq')
        self.ui_eq_band11_freq = self.builder.get_object('eq_band11_freq')
        self.ui_eq_band12_freq = self.builder.get_object('eq_band12_freq')
        self.ui_eq_band13_freq = self.builder.get_object('eq_band13_freq')
        self.ui_eq_band14_freq = self.builder.get_object('eq_band14_freq')

        self.ui_eq_band0_qfactor = self.builder.get_object(
            'eq_band0_qfactor')
        self.ui_eq_band1_qfactor = self.builder.get_object(
            'eq_band1_qfactor')
        self.ui_eq_band2_qfactor = self.builder.get_object(
            'eq_band2_qfactor')
        self.ui_eq_band3_qfactor = self.builder.get_object(
            'eq_band3_qfactor')
        self.ui_eq_band4_qfactor = self.builder.get_object(
            'eq_band4_qfactor')
        self.ui_eq_band5_qfactor = self.builder.get_object(
            'eq_band5_qfactor')
        self.ui_eq_band6_qfactor = self.builder.get_object(
            'eq_band6_qfactor')
        self.ui_eq_band7_qfactor = self.builder.get_object(
            'eq_band7_qfactor')
        self.ui_eq_band8_qfactor = self.builder.get_object(
            'eq_band8_qfactor')
        self.ui_eq_band9_qfactor = self.builder.get_object(
            'eq_band9_qfactor')
        self.ui_eq_band10_qfactor = self.builder.get_object(
            'eq_band10_qfactor')
        self.ui_eq_band11_qfactor = self.builder.get_object(
            'eq_band11_qfactor')
        self.ui_eq_band12_qfactor = self.builder.get_object(
            'eq_band12_qfactor')
        self.ui_eq_band13_qfactor = self.builder.get_object(
            'eq_band13_qfactor')
        self.ui_eq_band14_qfactor = self.builder.get_object(
            'eq_band14_qfactor')

        self.ui_equalizer_input_level_left = self.builder.get_object(
            'equalizer_input_level_left')
        self.ui_equalizer_input_level_right = self.builder.get_object(
            'equalizer_input_level_right')
        self.ui_equalizer_output_level_left = self.builder.get_object(
            'equalizer_output_level_left')
        self.ui_equalizer_output_level_right = self.builder.get_object(
            'equalizer_output_level_right')

        self.ui_equalizer_input_level_left_label = self.builder.get_object(
            'equalizer_input_level_left_label')
        self.ui_equalizer_input_level_right_label = self.builder.get_object(
            'equalizer_input_level_right_label')
        self.ui_equalizer_output_level_left_label = self.builder.get_object(
            'equalizer_output_level_left_label')
        self.ui_equalizer_output_level_right_label = self.builder.get_object(
            'equalizer_output_level_right_label')

    def init_limiter_ui(self):
        self.limiter_user = self.settings.get_value('limiter-user').unpack()
        self.apply_limiter_preset(self.limiter_user)

    def init_compressor_ui(self):
        self.compressor_user = self.settings.get_value(
            'compressor-user').unpack()
        self.apply_compressor_preset(self.compressor_user)

    def init_reverb_ui(self):
        self.reverb_user = self.settings.get_value('reverb-user').unpack()
        self.apply_reverb_preset(self.reverb_user)

    def init_highpass_ui(self):
        highpass_cutoff_user = self.settings.get_value(
            'highpass-cutoff').unpack()
        highpass_poles_user = self.settings.get_value(
            'highpass-poles').unpack()

        self.ui_highpass_cutoff.set_value(highpass_cutoff_user)
        self.ui_highpass_poles.set_value(highpass_poles_user)

        self.highpass.set_property('cutoff', highpass_cutoff_user)
        self.highpass.set_property('poles', highpass_poles_user)

    def init_lowpass_ui(self):
        lowpass_cutoff_user = self.settings.get_value(
            'lowpass-cutoff').unpack()
        lowpass_poles_user = self.settings.get_value(
            'lowpass-poles').unpack()

        self.ui_lowpass_cutoff.set_value(lowpass_cutoff_user)
        self.ui_lowpass_poles.set_value(lowpass_poles_user)

        self.lowpass.set_property('cutoff', lowpass_cutoff_user)
        self.lowpass.set_property('poles', lowpass_poles_user)

    def init_equalizer_ui(self):
        equalizer_input_gain_user = self.settings.get_value(
            'equalizer-input-gain').unpack()
        equalizer_output_gain_user = self.settings.get_value(
            'equalizer-output-gain').unpack()

        self.eq_band_user = self.settings.get_value('equalizer-user').unpack()

        self.ui_equalizer_input_gain.set_value(equalizer_input_gain_user)
        self.ui_equalizer_output_gain.set_value(equalizer_output_gain_user)
        self.apply_eq_preset(self.eq_band_user)

        self.init_eq_freq_and_qfactors()

        # we need this when on value changed is not called

        value_linear = 10**(equalizer_input_gain_user / 20)
        self.equalizer_input_gain.set_property('volume', value_linear)

        value_linear = 10**(equalizer_output_gain_user / 20)
        self.equalizer_output_gain.set_property('volume', value_linear)

    def init_eq_freq_and_qfactors(self):
        self.eq_freqs = self.settings.get_value('equalizer-freqs').unpack()
        self.eq_qfactors = self.settings.get_value(
            'equalizer-qfactors').unpack()

        self.ui_eq_band0_freq.set_text('{0:g}'.format(self.eq_freqs[0]))
        self.ui_eq_band1_freq.set_text('{0:g}'.format(self.eq_freqs[1]))
        self.ui_eq_band2_freq.set_text('{0:g}'.format(self.eq_freqs[2]))
        self.ui_eq_band3_freq.set_text('{0:g}'.format(self.eq_freqs[3]))
        self.ui_eq_band4_freq.set_text('{0:g}'.format(self.eq_freqs[4]))
        self.ui_eq_band5_freq.set_text('{0:g}'.format(self.eq_freqs[5]))
        self.ui_eq_band6_freq.set_text('{0:g}'.format(self.eq_freqs[6]))
        self.ui_eq_band7_freq.set_text('{0:g}'.format(self.eq_freqs[7]))
        self.ui_eq_band8_freq.set_text('{0:g}'.format(self.eq_freqs[8]))
        self.ui_eq_band9_freq.set_text('{0:g}'.format(self.eq_freqs[9]))
        self.ui_eq_band10_freq.set_text('{0:g}'.format(self.eq_freqs[10]))
        self.ui_eq_band11_freq.set_text('{0:g}'.format(self.eq_freqs[11]))
        self.ui_eq_band12_freq.set_text('{0:g}'.format(self.eq_freqs[12]))
        self.ui_eq_band13_freq.set_text('{0:g}'.format(self.eq_freqs[13]))
        self.ui_eq_band14_freq.set_text('{0:g}'.format(self.eq_freqs[14]))

        self.ui_eq_band0_qfactor.set_text(str(self.eq_qfactors[0]))
        self.ui_eq_band1_qfactor.set_text(str(self.eq_qfactors[1]))
        self.ui_eq_band2_qfactor.set_text(str(self.eq_qfactors[2]))
        self.ui_eq_band3_qfactor.set_text(str(self.eq_qfactors[3]))
        self.ui_eq_band4_qfactor.set_text(str(self.eq_qfactors[4]))
        self.ui_eq_band5_qfactor.set_text(str(self.eq_qfactors[5]))
        self.ui_eq_band6_qfactor.set_text(str(self.eq_qfactors[6]))
        self.ui_eq_band7_qfactor.set_text(str(self.eq_qfactors[7]))
        self.ui_eq_band8_qfactor.set_text(str(self.eq_qfactors[8]))
        self.ui_eq_band9_qfactor.set_text(str(self.eq_qfactors[9]))
        self.ui_eq_band10_qfactor.set_text(str(self.eq_qfactors[10]))
        self.ui_eq_band11_qfactor.set_text(str(self.eq_qfactors[11]))
        self.ui_eq_band12_qfactor.set_text(str(self.eq_qfactors[12]))
        self.ui_eq_band13_qfactor.set_text(str(self.eq_qfactors[13]))
        self.ui_eq_band14_qfactor.set_text(str(self.eq_qfactors[14]))

        # pipeline

        self.eq_band0.set_property('freq', self.eq_freqs[0])
        self.eq_band1.set_property('freq', self.eq_freqs[1])
        self.eq_band2.set_property('freq', self.eq_freqs[2])
        self.eq_band3.set_property('freq', self.eq_freqs[3])
        self.eq_band4.set_property('freq', self.eq_freqs[4])
        self.eq_band5.set_property('freq', self.eq_freqs[5])
        self.eq_band6.set_property('freq', self.eq_freqs[6])
        self.eq_band7.set_property('freq', self.eq_freqs[7])
        self.eq_band8.set_property('freq', self.eq_freqs[8])
        self.eq_band9.set_property('freq', self.eq_freqs[9])
        self.eq_band10.set_property('freq', self.eq_freqs[10])
        self.eq_band11.set_property('freq', self.eq_freqs[11])
        self.eq_band12.set_property('freq', self.eq_freqs[12])
        self.eq_band13.set_property('freq', self.eq_freqs[13])
        self.eq_band14.set_property('freq', self.eq_freqs[14])

        self.eq_band0.set_property('bandwidth',
                                   self.eq_freqs[0] / self.eq_qfactors[0])
        self.eq_band1.set_property('bandwidth',
                                   self.eq_freqs[1] / self.eq_qfactors[1])
        self.eq_band2.set_property('bandwidth',
                                   self.eq_freqs[2] / self.eq_qfactors[2])
        self.eq_band3.set_property('bandwidth',
                                   self.eq_freqs[3] / self.eq_qfactors[3])
        self.eq_band4.set_property('bandwidth',
                                   self.eq_freqs[4] / self.eq_qfactors[4])
        self.eq_band5.set_property('bandwidth',
                                   self.eq_freqs[5] / self.eq_qfactors[5])
        self.eq_band6.set_property('bandwidth',
                                   self.eq_freqs[6] / self.eq_qfactors[6])
        self.eq_band7.set_property('bandwidth',
                                   self.eq_freqs[7] / self.eq_qfactors[7])
        self.eq_band8.set_property('bandwidth',
                                   self.eq_freqs[8] / self.eq_qfactors[8])
        self.eq_band9.set_property('bandwidth',
                                   self.eq_freqs[9] / self.eq_qfactors[9])
        self.eq_band10.set_property('bandwidth',
                                    self.eq_freqs[10] / self.eq_qfactors[10])
        self.eq_band11.set_property('bandwidth',
                                    self.eq_freqs[11] / self.eq_qfactors[11])
        self.eq_band12.set_property('bandwidth',
                                    self.eq_freqs[12] / self.eq_qfactors[12])
        self.eq_band13.set_property('bandwidth',
                                    self.eq_freqs[13] / self.eq_qfactors[13])
        self.eq_band14.set_property('bandwidth',
                                    self.eq_freqs[14] / self.eq_qfactors[14])

    def apply_limiter_preset(self, values):
        self.ui_limiter_input_gain.set_value(values[0])
        self.ui_limiter_limit.set_value(values[1])
        self.ui_limiter_release_time.set_value(values[2])

        # we need this when on value changed is not called
        self.limiter.set_property('input-gain', values[0])
        self.limiter.set_property('limit', values[1])
        self.limiter.set_property('release-time', values[2])

    def apply_compressor_preset(self, values):
        if values[0] == 0:
            self.ui_compressor_rms.set_active(True)
        elif values[0] == 1:
            self.ui_compressor_peak.set_active(True)

        self.ui_compressor_attack.set_value(values[1])
        self.ui_compressor_release.set_value(values[2])
        self.ui_compressor_threshold.set_value(values[3])
        self.ui_compressor_ratio.set_value(values[4])
        self.ui_compressor_knee.set_value(values[5])
        self.ui_compressor_makeup.set_value(values[6])

        # we need this when on value changed is not called
        self.compressor.set_property('rms-peak', values[0])
        self.compressor.set_property('attack-time', values[1])
        self.compressor.set_property('release-time', values[2])
        self.compressor.set_property('threshold-level', values[3])
        self.compressor.set_property('ratio', values[4])
        self.compressor.set_property('knee-radius', values[5])
        self.compressor.set_property('makeup-gain', values[6])

    def apply_reverb_preset(self, values):
        self.ui_reverb_room_size.set_value(values[0])
        self.ui_reverb_damping.set_value(values[1])
        self.ui_reverb_width.set_value(values[2])
        self.ui_reverb_level.set_value(values[3])

        # we need this when on value changed is not called
        self.freeverb.set_property('room-size', values[0])
        self.freeverb.set_property('damping', values[1])
        self.freeverb.set_property('width', values[2])
        self.freeverb.set_property('level', values[3])

    def apply_eq_preset(self, values):
        self.ui_eq_band0.set_value(values[0])
        self.ui_eq_band1.set_value(values[1])
        self.ui_eq_band2.set_value(values[2])
        self.ui_eq_band3.set_value(values[3])
        self.ui_eq_band4.set_value(values[4])
        self.ui_eq_band5.set_value(values[5])
        self.ui_eq_band6.set_value(values[6])
        self.ui_eq_band7.set_value(values[7])
        self.ui_eq_band8.set_value(values[8])
        self.ui_eq_band9.set_value(values[9])
        self.ui_eq_band10.set_value(values[10])
        self.ui_eq_band11.set_value(values[11])
        self.ui_eq_band12.set_value(values[12])
        self.ui_eq_band13.set_value(values[13])
        self.ui_eq_band14.set_value(values[14])

        # we need this when on value changed is not called
        self.eq_band0.set_property('gain', values[0])
        self.eq_band1.set_property('gain', values[1])
        self.eq_band2.set_property('gain', values[2])
        self.eq_band3.set_property('gain', values[3])
        self.eq_band4.set_property('gain', values[4])
        self.eq_band5.set_property('gain', values[5])
        self.eq_band6.set_property('gain', values[6])
        self.eq_band7.set_property('gain', values[7])
        self.eq_band8.set_property('gain', values[8])
        self.eq_band9.set_property('gain', values[9])
        self.eq_band10.set_property('gain', values[10])
        self.eq_band11.set_property('gain', values[11])
        self.eq_band12.set_property('gain', values[12])
        self.eq_band13.set_property('gain', values[13])
        self.eq_band14.set_property('gain', values[14])

    def save_limiter_user(self, idx, value):
        self.limiter_user[idx] = value

        out = GLib.Variant('ad', self.limiter_user)

        self.settings.set_value('limiter-user', out)

    def save_compressor_user(self, idx, value):
        self.compressor_user[idx] = value

        out = GLib.Variant('ad', self.compressor_user)

        self.settings.set_value('compressor-user', out)

    def save_reverb_user(self, idx, value):
        self.reverb_user[idx] = value

        out = GLib.Variant('ad', self.reverb_user)

        self.settings.set_value('reverb-user', out)

    def save_eq_user(self, idx, value):
        self.eq_band_user[idx] = value

        out = GLib.Variant('ad', self.eq_band_user)

        self.settings.set_value('equalizer-user', out)

    def on_limiter_input_gain_value_changed(self, obj):
        value = obj.get_value()
        self.limiter.set_property('input-gain', value)
        self.save_limiter_user(0, value)

    def on_limiter_limit_value_changed(self, obj):
        value = obj.get_value()
        self.limiter.set_property('limit', value)
        self.save_limiter_user(1, value)

    def on_limiter_release_time_value_changed(self, obj):
        value = obj.get_value()
        self.limiter.set_property('release-time', value)
        self.save_limiter_user(2, value)

    def on_compressor_measurement_type(self, obj):
        if obj.get_active():
            label = obj.get_label()

            if label == 'rms':
                self.compressor.set_property('rms-peak', 0)
                self.save_compressor_user(0, 0)
            elif label == 'peak':
                self.compressor.set_property('rms-peak', 1)
                self.save_compressor_user(0, 1)

    def on_compressor_attack_time_value_changed(self, obj):
        value = obj.get_value()
        self.compressor.set_property('attack-time', value)
        self.save_compressor_user(1, value)

    def on_compressor_release_time_value_changed(self, obj):
        value = obj.get_value()
        self.compressor.set_property('release-time', value)
        self.save_compressor_user(2, value)

    def on_compressor_threshold_value_changed(self, obj):
        value = obj.get_value()
        self.compressor.set_property('threshold-level', value)
        self.save_compressor_user(3, value)

    def on_compressor_ratio_value_changed(self, obj):
        value = obj.get_value()
        self.compressor.set_property('ratio', value)
        self.save_compressor_user(4, value)

    def on_compressor_knee_value_changed(self, obj):
        value = obj.get_value()
        self.compressor.set_property('knee-radius', value)
        self.save_compressor_user(5, value)

    def on_compressor_makeup_value_changed(self, obj):
        value = obj.get_value()
        self.compressor.set_property('makeup-gain', value)
        self.save_compressor_user(6, value)

    def on_compressor_preset_clicked(self, obj):
        obj_id = Gtk.Buildable.get_name(obj)

        if obj_id == 'no_compression':
            value = self.settings.get_value('compressor-no-compression')
            self.apply_compressor_preset(value)
        elif obj_id == 'vlc':
            value = self.settings.get_value('compressor-vlc')
            self.apply_compressor_preset(value)

    def on_reverb_room_size_value_changed(self, obj):
        value = obj.get_value()
        self.freeverb.set_property('room-size', value)
        self.save_reverb_user(0, value)

    def on_reverb_damping_value_changed(self, obj):
        value = obj.get_value()
        self.freeverb.set_property('damping', value)
        self.save_reverb_user(1, value)

    def on_reverb_width_value_changed(self, obj):
        value = obj.get_value()
        self.freeverb.set_property('width', value)
        self.save_reverb_user(2, value)

    def on_reverb_level_value_changed(self, obj):
        value = obj.get_value()
        self.freeverb.set_property('level', value)
        self.save_reverb_user(3, value)

    def on_reverb_preset_clicked(self, obj):
        obj_id = Gtk.Buildable.get_name(obj)

        if obj_id == 'cathedral':
            value = self.settings.get_value('reverb-cathedral')
            self.apply_reverb_preset(value)
        elif obj_id == 'no_reverberation':
            value = self.settings.get_value('reverb-no-reverberation')
            self.apply_reverb_preset(value)
        elif obj_id == 'engine_room':
            value = self.settings.get_value('reverb-engine-room')
            self.apply_reverb_preset(value)
        elif obj_id == 'small_room':
            value = self.settings.get_value('reverb-small-room')
            self.apply_reverb_preset(value)

    def on_highpass_cutoff_value_changed(self, obj):
        value = obj.get_value()
        self.highpass.set_property('cutoff', value)

        out = GLib.Variant('i', value)

        self.settings.set_value('highpass-cutoff', out)

    def on_highpass_poles_value_changed(self, obj):
        value = obj.get_value()
        self.highpass.set_property('poles', value)

        out = GLib.Variant('i', value)

        self.settings.set_value('highpass-poles', out)

    def on_lowpass_cutoff_value_changed(self, obj):
        value = obj.get_value()
        self.lowpass.set_property('cutoff', value)

        out = GLib.Variant('i', value)

        self.settings.set_value('lowpass-cutoff', out)

    def on_lowpass_poles_value_changed(self, obj):
        value = obj.get_value()
        self.lowpass.set_property('poles', value)

        out = GLib.Variant('i', value)

        self.settings.set_value('lowpass-poles', out)

    def on_equalizer_input_gain_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20)

        self.equalizer_input_gain.set_property('volume', value_linear)

        out = GLib.Variant('d', value_db)

        self.settings.set_value('equalizer-input-gain', out)

    def on_equalizer_output_gain_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20)

        self.equalizer_output_gain.set_property('volume', value_linear)

        out = GLib.Variant('d', value_db)

        self.settings.set_value('equalizer-output-gain', out)

    def on_eq_band0_value_changed(self, obj):
        value = obj.get_value()
        self.eq_band0.set_property('gain', value)
        self.save_eq_user(0, value)

    def on_eq_band1_value_changed(self, obj):
        value = obj.get_value()
        self.eq_band1.set_property('gain', value)
        self.save_eq_user(1, value)

    def on_eq_band2_value_changed(self, obj):
        value = obj.get_value()
        self.eq_band2.set_property('gain', value)
        self.save_eq_user(2, value)

    def on_eq_band3_value_changed(self, obj):
        value = obj.get_value()
        self.eq_band3.set_property('gain', value)
        self.save_eq_user(3, value)

    def on_eq_band4_value_changed(self, obj):
        value = obj.get_value()
        self.eq_band4.set_property('gain', value)
        self.save_eq_user(4, value)

    def on_eq_band5_value_changed(self, obj):
        value = obj.get_value()
        self.eq_band5.set_property('gain', value)
        self.save_eq_user(5, value)

    def on_eq_band6_value_changed(self, obj):
        value = obj.get_value()
        self.eq_band6.set_property('gain', value)
        self.save_eq_user(6, value)

    def on_eq_band7_value_changed(self, obj):
        value = obj.get_value()
        self.eq_band7.set_property('gain', value)
        self.save_eq_user(7, value)

    def on_eq_band8_value_changed(self, obj):
        value = obj.get_value()
        self.eq_band8.set_property('gain', value)
        self.save_eq_user(8, value)

    def on_eq_band9_value_changed(self, obj):
        value = obj.get_value()
        self.eq_band9.set_property('gain', value)
        self.save_eq_user(9, value)

    def on_eq_band10_value_changed(self, obj):
        value = obj.get_value()
        self.eq_band10.set_property('gain', value)
        self.save_eq_user(10, value)

    def on_eq_band11_value_changed(self, obj):
        value = obj.get_value()
        self.eq_band11.set_property('gain', value)
        self.save_eq_user(11, value)

    def on_eq_band12_value_changed(self, obj):
        value = obj.get_value()
        self.eq_band12.set_property('gain', value)
        self.save_eq_user(12, value)

    def on_eq_band13_value_changed(self, obj):
        value = obj.get_value()
        self.eq_band13.set_property('gain', value)
        self.save_eq_user(13, value)

    def on_eq_band14_value_changed(self, obj):
        value = obj.get_value()
        self.eq_band14.set_property('gain', value)
        self.save_eq_user(14, value)

    def on_eq_freq_changed(self, obj):
        try:
            value = float(obj.get_text())

            obj_id = Gtk.Buildable.get_name(obj)

            if obj_id == 'eq_band0_freq':
                self.eq_freqs[0] = value
                qfactor = self.eq_qfactors[0]
                self.eq_band0.set_property('freq', value)
                self.eq_band0.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band1_freq':
                self.eq_freqs[1] = value
                qfactor = self.eq_qfactors[1]
                self.eq_band1.set_property('freq', value)
                self.eq_band1.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band2_freq':
                self.eq_freqs[2] = value
                qfactor = self.eq_qfactors[2]
                self.eq_band2.set_property('freq', value)
                self.eq_band2.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band3_freq':
                self.eq_freqs[3] = value
                qfactor = self.eq_qfactors[3]
                self.eq_band3.set_property('freq', value)
                self.eq_band3.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band4_freq':
                self.eq_freqs[4] = value
                qfactor = self.eq_qfactors[4]
                self.eq_band4.set_property('freq', value)
                self.eq_band4.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band5_freq':
                self.eq_freqs[5] = value
                qfactor = self.eq_qfactors[5]
                self.eq_band5.set_property('freq', value)
                self.eq_band5.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band6_freq':
                self.eq_freqs[6] = value
                qfactor = self.eq_qfactors[6]
                self.eq_band6.set_property('freq', value)
                self.eq_band6.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band7_freq':
                self.eq_freqs[7] = value
                qfactor = self.eq_qfactors[7]
                self.eq_band7.set_property('freq', value)
                self.eq_band7.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band8_freq':
                self.eq_freqs[8] = value
                qfactor = self.eq_qfactors[8]
                self.eq_band8.set_property('freq', value)
                self.eq_band8.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band9_freq':
                self.eq_freqs[9] = value
                qfactor = self.eq_qfactors[9]
                self.eq_band9.set_property('freq', value)
                self.eq_band9.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band10_freq':
                self.eq_freqs[10] = value
                qfactor = self.eq_qfactors[10]
                self.eq_band10.set_property('freq', value)
                self.eq_band10.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band11_freq':
                self.eq_freqs[11] = value
                qfactor = self.eq_qfactors[11]
                self.eq_band11.set_property('freq', value)
                self.eq_band11.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band12_freq':
                self.eq_freqs[12] = value
                qfactor = self.eq_qfactors[12]
                self.eq_band12.set_property('freq', value)
                self.eq_band12.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band13_freq':
                self.eq_freqs[13] = value
                qfactor = self.eq_qfactors[13]
                self.eq_band13.set_property('freq', value)
                self.eq_band13.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band14_freq':
                self.eq_freqs[14] = value
                qfactor = self.eq_qfactors[14]
                self.eq_band14.set_property('freq', value)
                self.eq_band14.set_property('bandwidth', value / qfactor)

            out = GLib.Variant('ad', self.eq_freqs)
            self.settings.set_value('equalizer-freqs', out)
        except ValueError:
            pass

    def on_eq_qfactor_changed(self, obj):
        try:
            value = float(obj.get_text())

            obj_id = Gtk.Buildable.get_name(obj)

            if obj_id == 'eq_band0_qfactor':
                self.eq_qfactors[0] = value
                self.eq_band0.set_property(
                    'bandwidth', self.eq_freqs[0] / value)
            elif obj_id == 'eq_band1_qfactor':
                self.eq_qfactors[1] = value
                self.eq_band1.set_property(
                    'bandwidth', self.eq_freqs[1] / value)
            elif obj_id == 'eq_band2_qfactor':
                self.eq_qfactors[2] = value
                self.eq_band2.set_property(
                    'bandwidth', self.eq_freqs[2] / value)
            elif obj_id == 'eq_band3_qfactor':
                self.eq_qfactors[3] = value
                self.eq_band3.set_property(
                    'bandwidth', self.eq_freqs[3] / value)
            elif obj_id == 'eq_band4_qfactor':
                self.eq_qfactors[4] = value
                self.eq_band4.set_property(
                    'bandwidth', self.eq_freqs[4] / value)
            elif obj_id == 'eq_band5_qfactor':
                self.eq_qfactors[5] = value
                self.eq_band5.set_property(
                    'bandwidth', self.eq_freqs[5] / value)
            elif obj_id == 'eq_band6_qfactor':
                self.eq_qfactors[6] = value
                self.eq_band6.set_property(
                    'bandwidth', self.eq_freqs[6] / value)
            elif obj_id == 'eq_band7_qfactor':
                self.eq_qfactors[7] = value
                self.eq_band7.set_property(
                    'bandwidth', self.eq_freqs[7] / value)
            elif obj_id == 'eq_band8_qfactor':
                self.eq_qfactors[8] = value
                self.eq_band8.set_property(
                    'bandwidth', self.eq_freqs[8] / value)
            elif obj_id == 'eq_band9_qfactor':
                self.eq_qfactors[9] = value
                self.eq_band9.set_property(
                    'bandwidth', self.eq_freqs[9] / value)
            elif obj_id == 'eq_band10_qfactor':
                self.eq_qfactors[10] = value
                self.eq_band10.set_property(
                    'bandwidth', self.eq_freqs[10] / value)
            elif obj_id == 'eq_band11_qfactor':
                self.eq_qfactors[11] = value
                self.eq_band11.set_property(
                    'bandwidth', self.eq_freqs[11] / value)
            elif obj_id == 'eq_band12_qfactor':
                self.eq_qfactors[12] = value
                self.eq_band12.set_property(
                    'bandwidth', self.eq_freqs[12] / value)
            elif obj_id == 'eq_band13_qfactor':
                self.eq_qfactors[13] = value
                self.eq_band13.set_property(
                    'bandwidth', self.eq_freqs[13] / value)
            elif obj_id == 'eq_band14_qfactor':
                self.eq_qfactors[14] = value
                self.eq_band14.set_property(
                    'bandwidth', self.eq_freqs[14] / value)

            out = GLib.Variant('ad', self.eq_qfactors)
            self.settings.set_value('equalizer-qfactors', out)
        except ValueError:
            pass

    def ui_update_level(self, widgets, peak):
        left, right = peak[0], peak[1]

        widget_level_left = widgets[0]
        widget_level_right = widgets[1]
        widget_level_left_label = widgets[2]
        widget_level_right_label = widgets[3]

        if left >= -99:
            l_value = 10**(left / 20)
            widget_level_left.set_value(l_value)
            widget_level_left_label.set_text(str(round(left)))
        else:
            widget_level_left.set_value(0)
            widget_level_left_label.set_text('-99')

        if right >= -99:
            r_value = 10**(right / 20)
            widget_level_right.set_value(r_value)
            widget_level_right_label.set_text(str(round(right)))
        else:
            widget_level_right.set_value(0)
            widget_level_right_label.set_text('-99')

    def ui_update_compressor_input_level(self, peak):
        widgets = [self.ui_compressor_input_level_left,
                   self.ui_compressor_input_level_right,
                   self.ui_compressor_input_level_left_label,
                   self.ui_compressor_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_compressor_output_level(self, peak):
        widgets = [self.ui_compressor_output_level_left,
                   self.ui_compressor_output_level_right,
                   self.ui_compressor_output_level_left_label,
                   self.ui_compressor_output_level_right_label]

        self.ui_update_level(widgets, peak)

        gain_reduction = abs(round(
            self.compressor.get_property('gain-reduction')))

        if gain_reduction != self.old_compressor_gain_reduction:
            self.old_compressor_gain_reduction = gain_reduction

            self.ui_compressor_gain_reduction_levelbar.set_value(
                gain_reduction)
            self.ui_compressor_gain_reduction_level_label.set_text(
                str(round(gain_reduction)))

    def ui_update_reverb_input_level(self, peak):
        widgets = [self.ui_reverb_input_level_left,
                   self.ui_reverb_input_level_right,
                   self.ui_reverb_input_level_left_label,
                   self.ui_reverb_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_reverb_output_level(self, peak):
        widgets = [self.ui_reverb_output_level_left,
                   self.ui_reverb_output_level_right,
                   self.ui_reverb_output_level_left_label,
                   self.ui_reverb_output_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_highpass_input_level(self, peak):
        widgets = [self.ui_highpass_input_level_left,
                   self.ui_highpass_input_level_right,
                   self.ui_highpass_input_level_left_label,
                   self.ui_highpass_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_highpass_output_level(self, peak):
        widgets = [self.ui_highpass_output_level_left,
                   self.ui_highpass_output_level_right,
                   self.ui_highpass_output_level_left_label,
                   self.ui_highpass_output_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_lowpass_input_level(self, peak):
        widgets = [self.ui_lowpass_input_level_left,
                   self.ui_lowpass_input_level_right,
                   self.ui_lowpass_input_level_left_label,
                   self.ui_lowpass_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_lowpass_output_level(self, peak):
        widgets = [self.ui_lowpass_output_level_left,
                   self.ui_lowpass_output_level_right,
                   self.ui_lowpass_output_level_left_label,
                   self.ui_lowpass_output_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_equalizer_input_level(self, peak):
        widgets = [self.ui_equalizer_input_level_left,
                   self.ui_equalizer_input_level_right,
                   self.ui_equalizer_input_level_left_label,
                   self.ui_equalizer_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_equalizer_output_level(self, peak):
        widgets = [self.ui_equalizer_output_level_left,
                   self.ui_equalizer_output_level_right,
                   self.ui_equalizer_output_level_left_label,
                   self.ui_equalizer_output_level_right_label]

        self.ui_update_level(widgets, peak)
