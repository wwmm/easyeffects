# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk


class SetupTestSignal():

    def __init__(self, app_builder, test_signal):
        self.app_builder = app_builder
        self.ts = test_signal
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
        if state:
            self.wave1_volume.set_sensitive(True)
            self.wave1_volume.set_value(0.5)
            self.ts.set_state('playing')
        else:
            self.wave1_volume.set_sensitive(False)
            self.wave1_volume.set_value(0)

            if not self.wave2_switch.get_state():
                self.ts.set_state('paused')

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
        if state:
            self.wave2_volume.set_sensitive(True)
            self.wave2_volume.set_value(0.5)
            self.ts.set_state('playing')
        else:
            self.wave2_volume.set_sensitive(False)
            self.wave2_volume.set_value(0)

            if not self.wave1_switch.get_state():
                self.ts.set_state('paused')

    def on_wave2_volume_value_changed(self, obj):
        self.ts.set_wave2_volume(obj.get_value())

    def on_wave2_freq_toggled(self, obj):
        if obj.get_active():
            obj_id = Gtk.Buildable.get_name(obj)

            if obj_id == 'wave2_band0':  # 26 Hz
                self.ts.set_wave2_freq(20, 41, 4)
            elif obj_id == 'wave2_band1':  # 41 Hz
                self.ts.set_wave2_freq(26, 65, 4)
            elif obj_id == 'wave2_band2':  # 65 Hz
                self.ts.set_wave2_freq(41, 103, 4)
            elif obj_id == 'wave2_band3':  # 103 Hz
                self.ts.set_wave2_freq(65, 163, 4)
            elif obj_id == 'wave2_band4':  # 163 Hz
                self.ts.set_wave2_freq(103, 259, 4)
            elif obj_id == 'wave2_band5':  # 259 Hz
                self.ts.set_wave2_freq(163, 410, 8)
            elif obj_id == 'wave2_band6':  # 410 Hz
                self.ts.set_wave2_freq(259, 649, 8)
            elif obj_id == 'wave2_band7':  # 649 Hz
                self.ts.set_wave2_freq(410, 1029, 8)
            elif obj_id == 'wave2_band8':  # 1029 Hz
                self.ts.set_wave2_freq(649, 1631, 12)
            elif obj_id == 'wave2_band9':  # 1631 Hz
                self.ts.set_wave2_freq(1029, 2585, 12)
            elif obj_id == 'wave2_band10':  # 2585 Hz
                self.ts.set_wave2_freq(1631, 4097, 16)
            elif obj_id == 'wave2_band11':  # 4097 Hz
                self.ts.set_wave2_freq(2585, 6493, 24)
            elif obj_id == 'wave2_band12':  # 6493 Hz
                self.ts.set_wave2_freq(4097, 10291, 32)
            elif obj_id == 'wave2_band13':  # 10291 Hz
                self.ts.set_wave2_freq(6493, 16310, 32)
            elif obj_id == 'wave2_band14':  # 16310 Hz
                self.ts.set_wave2_freq(10291, 20000, 32)
