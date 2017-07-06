# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gtk


class SetupTestSignal():

    def __init__(self, app_builder, effects):
        self.app_builder = app_builder
        self.effects = effects
        self.module_path = os.path.dirname(__file__)

        self.handlers = {
            'on_test_signal_switch_state_set':
                self.on_test_signal_switch_state_set,
            'on_wave2_freq_toggled': self.on_wave2_freq_toggled
        }

    def on_test_signal_switch_state_set(self, obj, state):
        if state:
            if not self.sie_effects.is_playing:
                self.sie_effects.set_state('playing')

            self.effects.set_state('playing')
        else:
            self.effects.set_state('paused')

    def on_wave2_freq_toggled(self, obj):
        if obj.get_active():
            obj_id = Gtk.Buildable.get_name(obj)

            if obj_id == 'wave2_band0':
                # amplitude scaling factor, lower, center, upper
                self.effects.set_freq(1, 23, 26, 29)
            elif obj_id == 'wave2_band1':
                self.effects.set_freq(1.58, 38, 41, 44)
            elif obj_id == 'wave2_band2':
                self.effects.set_freq(2.5, 62, 65, 68)
            elif obj_id == 'wave2_band3':
                self.effects.set_freq(3.96, 100, 103, 106)
            elif obj_id == 'wave2_band4':
                self.effects.set_freq(6.27, 159, 163, 166)
            elif obj_id == 'wave2_band5':
                self.effects.set_freq(9.96, 256, 259, 262)
            elif obj_id == 'wave2_band6':
                self.effects.set_freq(15.77, 407, 410, 413)
            elif obj_id == 'wave2_band7':
                self.effects.set_freq(24.96, 646, 649, 652)
            elif obj_id == 'wave2_band8':
                self.effects.set_freq(39.58, 1026, 1029, 1032)
            elif obj_id == 'wave2_band9':
                self.effects.set_freq(62.73, 1628, 1631, 1634)
            elif obj_id == 'wave2_band10':
                self.effects.set_freq(99.42, 2582, 2585, 2588)
            elif obj_id == 'wave2_band11':
                self.effects.set_freq(157.58, 4094, 4097, 4100)
            elif obj_id == 'wave2_band12':
                self.effects.set_freq(249.73, 6490, 6493, 6496)
            elif obj_id == 'wave2_band13':
                self.effects.set_freq(395.81, 10288, 10291, 10294)
            elif obj_id == 'wave2_band14':
                self.effects.set_freq(627.31, 16307, 16310, 16313)
