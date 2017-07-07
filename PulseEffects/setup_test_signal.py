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
            'on_wave2_freq_toggled':
                self.on_wave2_freq_toggled,
            'on_wave2_volume_value_changed':
                self.on_wave2_volume_value_changed,
            'on_wave2_switch_state_set':
                self.on_wave2_switch_state_set
        }

        self.app_builder.connect_signals(self.handlers)

        self.wave1_volume = self.app_builder.get_object('wave1_volume_scale')
        self.wave1_freq = self.app_builder.get_object('wave1_freq')
        self.wave2_volume = self.app_builder.get_object('wave2_volume_scale')
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

            self.ts.set_state('playing')

            self.wave1_volume.set_sensitive(True)
            self.wave1_volume.set_value(0.5)
        else:
            if n_sink_inputs == 0 and self.sie.is_playing:
                self.sie.set_state('paused')

            self.ts.set_state('paused')

            self.wave1_volume.set_sensitive(False)
            self.wave1_volume.set_value(0)

    def on_wave1_volume_value_changed(self, obj):
        self.ts.set_wave1_volume(obj.get_value())

    def on_wave2_switch_state_set(self, obj, state):
        n_sink_inputs = self.list_sink_inputs.get_n_inputs()

        if state:
            if n_sink_inputs == 0 and not self.sie.is_playing:
                self.sie.set_state('playing')

            self.ts.set_state('playing')

            self.wave2_volume.set_sensitive(True)
            self.wave2_volume.set_value(0.5)
        else:
            if n_sink_inputs == 0 and self.sie.is_playing:
                self.sie.set_state('paused')

            self.ts.set_state('paused')

            self.wave2_volume.set_sensitive(False)
            self.wave2_volume.set_value(0)

    def on_wave2_volume_value_changed(self, obj):
        self.ts.set_wave2_volume(obj.get_value())

    def on_wave2_freq_toggled(self, obj):
        if obj.get_active():
            obj_id = Gtk.Buildable.get_name(obj)

            if obj_id == 'wave2_band0':
                # amplitude scaling factor, lower, center, upper
                self.ts.set_wave2_freq(23, 26, 29)
            elif obj_id == 'wave2_band1':
                self.ts.set_wave2_freq(38, 41, 44)
            elif obj_id == 'wave2_band2':
                self.ts.set_wave2_freq(62, 65, 68)
            elif obj_id == 'wave2_band3':
                self.ts.set_wave2_freq(100, 103, 106)
            elif obj_id == 'wave2_band4':
                self.ts.set_wave2_freq(159, 163, 166)
            elif obj_id == 'wave2_band5':
                self.ts.set_wave2_freq(256, 259, 262)
            elif obj_id == 'wave2_band6':
                self.ts.set_wave2_freq(407, 410, 413)
            elif obj_id == 'wave2_band7':
                self.ts.set_wave2_freq(646, 649, 652)
            elif obj_id == 'wave2_band8':
                self.ts.set_wave2_freq(1026, 1029, 1032)
            elif obj_id == 'wave2_band9':
                self.ts.set_wave2_freq(1628, 1631, 1634)
            elif obj_id == 'wave2_band10':
                self.ts.set_wave2_freq(2582, 2585, 2588)
            elif obj_id == 'wave2_band11':
                self.ts.set_wave2_freq(4094, 4097, 4100)
            elif obj_id == 'wave2_band12':
                self.ts.set_wave2_freq(6490, 6493, 6496)
            elif obj_id == 'wave2_band13':
                self.ts.set_wave2_freq(10288, 10291, 10294)
            elif obj_id == 'wave2_band14':
                self.ts.set_wave2_freq(16307, 16310, 16313)
