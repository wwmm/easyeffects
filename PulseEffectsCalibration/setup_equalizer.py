# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk


class SetupEqualizer():

    def __init__(self, app_builder, effects):
        self.app_builder = app_builder
        self.effects = effects
        self.module_path = os.path.dirname(__file__)

        self.handlers = {
            'on_equalizer_input_gain_value_changed':
                self.on_equalizer_input_gain_value_changed,
            'on_equalizer_output_gain_value_changed':
                self.on_equalizer_output_gain_value_changed,
            'on_eq_band0_value_changed': self.on_eq_band0_value_changed,
            'on_eq_band1_value_changed': self.on_eq_band1_value_changed,
            'on_eq_band2_value_changed': self.on_eq_band2_value_changed,
            'on_eq_band3_value_changed': self.on_eq_band3_value_changed,
            'on_eq_band4_value_changed': self.on_eq_band4_value_changed,
            'on_eq_band5_value_changed': self.on_eq_band5_value_changed,
            'on_eq_band6_value_changed': self.on_eq_band6_value_changed,
            'on_eq_band7_value_changed': self.on_eq_band7_value_changed,
            'on_eq_band8_value_changed': self.on_eq_band8_value_changed,
            'on_eq_band9_value_changed': self.on_eq_band9_value_changed,
            'on_eq_band10_value_changed': self.on_eq_band10_value_changed,
            'on_eq_band11_value_changed': self.on_eq_band11_value_changed,
            'on_eq_band12_value_changed': self.on_eq_band12_value_changed,
            'on_eq_band13_value_changed': self.on_eq_band13_value_changed,
            'on_eq_band14_value_changed': self.on_eq_band14_value_changed
        }

        self.equalizer_input_gain = self.app_builder.get_object(
            'equalizer_input_gain')
        self.equalizer_output_gain = self.app_builder.get_object(
            'equalizer_output_gain')

        self.eq_band0 = self.app_builder.get_object('eq_band0')
        self.eq_band1 = self.app_builder.get_object('eq_band1')
        self.eq_band2 = self.app_builder.get_object('eq_band2')
        self.eq_band3 = self.app_builder.get_object('eq_band3')
        self.eq_band4 = self.app_builder.get_object('eq_band4')
        self.eq_band5 = self.app_builder.get_object('eq_band5')
        self.eq_band6 = self.app_builder.get_object('eq_band6')
        self.eq_band7 = self.app_builder.get_object('eq_band7')
        self.eq_band8 = self.app_builder.get_object('eq_band8')
        self.eq_band9 = self.app_builder.get_object('eq_band9')
        self.eq_band10 = self.app_builder.get_object('eq_band10')
        self.eq_band11 = self.app_builder.get_object('eq_band11')
        self.eq_band12 = self.app_builder.get_object('eq_band12')
        self.eq_band13 = self.app_builder.get_object('eq_band13')
        self.eq_band14 = self.app_builder.get_object('eq_band14')

        self.equalizer_input_level = self.app_builder.get_object(
            'equalizer_input_level')
        self.equalizer_output_level = self.app_builder.get_object(
            'equalizer_output_level')

        self.equalizer_input_level_label = self.app_builder.get_object(
            'equalizer_input_level_label')
        self.equalizer_output_level_label = self.app_builder.get_object(
            'equalizer_output_level_label')

    def init(self):
        value_linear = 10**(0.0 / 20)
        self.effects.set_eq_input_gain(value_linear)

        value_linear = 10**(0.0 / 20)
        self.effects.set_eq_output_gain(value_linear)

        self.effects.set_eq_band0(0)
        self.effects.set_eq_band1(0)
        self.effects.set_eq_band2(0)
        self.effects.set_eq_band3(0)
        self.effects.set_eq_band4(0)
        self.effects.set_eq_band5(0)
        self.effects.set_eq_band6(0)
        self.effects.set_eq_band7(0)
        self.effects.set_eq_band8(0)
        self.effects.set_eq_band9(0)
        self.effects.set_eq_band10(0)
        self.effects.set_eq_band11(0)
        self.effects.set_eq_band12(0)
        self.effects.set_eq_band13(0)
        self.effects.set_eq_band14(0)

    def connect_signals(self):
        self.effects.connect('new_equalizer_input_level',
                             self.on_new_equalizer_input_level)

        self.effects.connect('new_equalizer_output_level',
                             self.on_new_equalizer_output_level)

    def on_new_equalizer_input_level(self, obj, value):
        if value >= -99:
            self.equalizer_input_level_label.set_text(str(round(value)))

            value = 10**(value / 20)
            self.equalizer_input_level.set_value(value)
        else:
            self.equalizer_input_level.set_value(0)
            self.equalizer_input_level_label.set_text('-99')

    def on_new_equalizer_output_level(self, obj, value):
        if value >= -99:
            self.equalizer_output_level_label.set_text(str(round(value)))

            value = 10**(value / 20)
            self.equalizer_output_level.set_value(value)
        else:
            self.equalizer_output_level.set_value(0)
            self.equalizer_output_level_label.set_text('-99')

    def apply_eq_preset(self, values):
        self.eq_band0.set_value(values[0])
        self.eq_band1.set_value(values[1])
        self.eq_band2.set_value(values[2])
        self.eq_band3.set_value(values[3])
        self.eq_band4.set_value(values[4])
        self.eq_band5.set_value(values[5])
        self.eq_band6.set_value(values[6])
        self.eq_band7.set_value(values[7])
        self.eq_band8.set_value(values[8])
        self.eq_band9.set_value(values[9])
        self.eq_band10.set_value(values[10])
        self.eq_band11.set_value(values[11])
        self.eq_band12.set_value(values[12])
        self.eq_band13.set_value(values[13])
        self.eq_band14.set_value(values[14])

    def on_eq_preset_clicked(self, obj):
        obj_id = Gtk.Buildable.get_name(obj)

        if obj_id == 'C1-U':
            # value = self.settings.get_value('equalizer-equal-loudness-20')
            # self.apply_eq_preset(value)
            pass

    def save_eq_user(self, idx, value):
        self.eq_band_user[idx] = value

    def on_equalizer_input_gain_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20)

        self.effects.set_eq_input_gain(value_linear)

    def on_equalizer_output_gain_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20)

        self.effects.set_eq_output_gain(value_linear)

    def on_eq_band0_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band0(value)

    def on_eq_band1_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band1(value)

    def on_eq_band2_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band2(value)

    def on_eq_band3_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band3(value)

    def on_eq_band4_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band4(value)

    def on_eq_band5_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band5(value)

    def on_eq_band6_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band6(value)

    def on_eq_band7_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band7(value)

    def on_eq_band8_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band8(value)

    def on_eq_band9_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band9(value)

    def on_eq_band10_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band10(value)

    def on_eq_band11_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band11(value)

    def on_eq_band12_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band12(value)

    def on_eq_band13_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band13(value)

    def on_eq_band14_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band14(value)
