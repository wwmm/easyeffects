# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk


class SetupTestSignal():

    def __init__(self, app_builder, test_signal, sink_input_effects,
                 list_sink_inputs):
        self.app_builder = app_builder
        self.ts = test_signal
        self.sie = sink_input_effects
        self.list_sink_inputs = list_sink_inputs
        self.module_path = os.path.dirname(__file__)

        self.handlers = {
            'on_wave1_switch_state_set':
                self.on_wave1_switch_state_set,
            'on_wave1_volume_value_changed':
                self.on_wave1_volume_value_changed,
            'on_wave1_type_toggled':
                self.on_wave1_type_toggled,
            'on_wave1_freq_value_changed':
                self.on_wave1_freq_value_changed,
            'on_wave2_freq_toggled':
                self.on_wave2_freq_toggled,
            'on_wave2_volume_value_changed':
                self.on_wave2_volume_value_changed,
            'on_wave2_switch_state_set':
                self.on_wave2_switch_state_set
        }

        self.app_builder.connect_signals(self.handlers)

        self.wave1_switch = self.app_builder.get_object('wave1_switch')
        self.wave1_volume = self.app_builder.get_object('wave1_volume')
        self.wave1_freq = self.app_builder.get_object('wave1_freq')
        self.wave2_switch = self.app_builder.get_object('wave2_switch')
        self.wave2_volume = self.app_builder.get_object('wave2_volume')
        self.wave2_band8 = self.app_builder.get_object('wave2_band8')

        self.wave1_volume.set_value(0)
        self.wave1_freq.set_value(1000)
        self.ts.set_wave1_freq(1000)
        self.ts.set_wave1_volume(0)
        self.wave1_volume.set_sensitive(False)

        self.wave2_volume.set_value(0)
        self.ts.set_wave2_volume(0)
        # equal loudness signal default frequency is 1 kHz (band 8)
        self.wave2_band8.set_active(True)
        self.wave2_volume.set_sensitive(False)

    def on_wave1_switch_state_set(self, obj, state):
        n_sink_inputs = self.list_sink_inputs.get_n_inputs()

        if state:
            if n_sink_inputs == 0 and not self.sie.is_playing:
                self.sie.set_state('playing')

            self.wave1_volume.set_sensitive(True)
            self.wave1_volume.set_value(0.5)
            self.ts.set_state('playing')
        else:
            self.wave1_volume.set_sensitive(False)
            self.wave1_volume.set_value(0)

            if not self.wave2_switch.get_state():
                self.ts.set_state('paused')

                if n_sink_inputs == 0 and self.sie.is_playing:
                    self.sie.set_state('paused')

    def on_wave1_volume_value_changed(self, obj):
        self.ts.set_wave1_volume(obj.get_value())

    def on_wave1_type_toggled(self, obj):
        if obj.get_active():
            obj_id = Gtk.Buildable.get_name(obj)

            if obj_id == 'wave_sine':
                self.ts.set_wave1_type('sine')
                self.wave1_freq.set_sensitive(True)
            elif obj_id == 'wave_square':
                self.ts.set_wave1_type('square')
                self.wave1_freq.set_sensitive(True)
            elif obj_id == 'wave_saw':
                self.ts.set_wave1_type('saw')
                self.wave1_freq.set_sensitive(True)
            elif obj_id == 'wave_triangle':
                self.ts.set_wave1_type('triangle')
                self.wave1_freq.set_sensitive(True)
            elif obj_id == 'wave_silence':
                self.ts.set_wave1_type('silence')
                self.wave1_freq.set_sensitive(False)
            elif obj_id == 'wave_white_noise':
                self.ts.set_wave1_type('white-noise')
                self.wave1_freq.set_sensitive(False)
            elif obj_id == 'wave_pink_noise':
                self.ts.set_wave1_type('pink-noise')
                self.wave1_freq.set_sensitive(False)
            elif obj_id == 'wave_sine_table':
                self.ts.set_wave1_type('sine-table')
                self.wave1_freq.set_sensitive(True)
            elif obj_id == 'wave_ticks':
                self.ts.set_wave1_type('ticks')
                self.wave1_freq.set_sensitive(True)
            elif obj_id == 'wave_gaussian_noise':
                self.ts.set_wave1_type('gaussian-noise')
                self.wave1_freq.set_sensitive(False)
            elif obj_id == 'wave_red_noise':
                self.ts.set_wave1_type('red-noise')
                self.wave1_freq.set_sensitive(False)
            elif obj_id == 'wave_blue_noise':
                self.ts.set_wave1_type('blue-noise')
                self.wave1_freq.set_sensitive(False)
            elif obj_id == 'wave_violet_noise':
                self.ts.set_wave1_type('violet-noise')
                self.wave1_freq.set_sensitive(False)

    def on_wave1_freq_value_changed(self, obj):
        self.ts.set_wave1_freq(obj.get_value())

    def on_wave2_switch_state_set(self, obj, state):
        n_sink_inputs = self.list_sink_inputs.get_n_inputs()

        if state:
            if n_sink_inputs == 0 and not self.sie.is_playing:
                self.sie.set_state('playing')

            self.wave2_volume.set_sensitive(True)
            self.wave2_volume.set_value(0.5)
            self.ts.set_state('playing')
        else:
            self.wave2_volume.set_sensitive(False)
            self.wave2_volume.set_value(0)

            if not self.wave1_switch.get_state():
                self.ts.set_state('paused')

                if n_sink_inputs == 0 and self.sie.is_playing:
                    self.sie.set_state('paused')

    def on_wave2_volume_value_changed(self, obj):
        self.ts.set_wave2_volume(obj.get_value())

    def on_wave2_freq_toggled(self, obj):
        if obj.get_active():
            obj_id = Gtk.Buildable.get_name(obj)

            lower_factor = 0.5
            upper_factor = 2.0

            if obj_id == 'wave2_band0':
                center = 26
                lower = lower_factor * center
                upper = upper_factor * center

                self.ts.set_wave2_freq(lower, upper, 4)
            elif obj_id == 'wave2_band1':
                center = 41
                lower = lower_factor * center
                upper = upper_factor * center

                self.ts.set_wave2_freq(lower, upper, 4)
            elif obj_id == 'wave2_band2':
                center = 65
                lower = lower_factor * center
                upper = upper_factor * center

                self.ts.set_wave2_freq(lower, upper, 4)
            elif obj_id == 'wave2_band3':
                center = 103
                lower = lower_factor * center
                upper = upper_factor * center

                self.ts.set_wave2_freq(lower, upper, 4)
            elif obj_id == 'wave2_band4':
                center = 163
                lower = lower_factor * center
                upper = upper_factor * center

                self.ts.set_wave2_freq(lower, upper, 8)
            elif obj_id == 'wave2_band5':
                center = 259
                lower = lower_factor * center
                upper = upper_factor * center

                self.ts.set_wave2_freq(lower, upper, 8)
            elif obj_id == 'wave2_band6':
                center = 410
                lower = lower_factor * center
                upper = upper_factor * center

                self.ts.set_wave2_freq(lower, upper, 8)
            elif obj_id == 'wave2_band7':
                center = 649
                lower = lower_factor * center
                upper = upper_factor * center

                self.ts.set_wave2_freq(lower, upper, 8)
            elif obj_id == 'wave2_band8':
                center = 1029
                lower = lower_factor * center
                upper = upper_factor * center

                self.ts.set_wave2_freq(lower, upper, 12)
            elif obj_id == 'wave2_band9':
                center = 1631
                lower = lower_factor * center
                upper = upper_factor * center

                self.ts.set_wave2_freq(lower, upper, 16)
            elif obj_id == 'wave2_band10':
                center = 2585
                lower = lower_factor * center
                upper = upper_factor * center

                self.ts.set_wave2_freq(lower, upper, 20)
            elif obj_id == 'wave2_band11':
                center = 4097
                lower = lower_factor * center
                upper = upper_factor * center

                self.ts.set_wave2_freq(lower, upper, 28)
            elif obj_id == 'wave2_band12':
                center = 6493
                lower = lower_factor * center
                upper = upper_factor * center

                self.ts.set_wave2_freq(lower, upper, 32)
            elif obj_id == 'wave2_band13':
                center = 10291
                lower = lower_factor * center
                upper = 20000

                self.ts.set_wave2_freq(lower, upper, 32)
            elif obj_id == 'wave2_band14':
                center = 16310
                lower = lower_factor * center
                upper = 20000

                self.ts.set_wave2_freq(lower, upper, 32)
