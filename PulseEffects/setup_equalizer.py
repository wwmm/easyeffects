# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gtk


class SetupEqualizer():

    def __init__(self, app_builder, effects, settings):
        self.app_builder = app_builder
        self.effects = effects
        self.settings = settings
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

        self.equalizer_input_level_left = self.app_builder.get_object(
            'equalizer_input_level_left')
        self.equalizer_input_level_right = self.app_builder.get_object(
            'equalizer_input_level_right')
        self.equalizer_output_level_left = self.app_builder.get_object(
            'equalizer_output_level_left')
        self.equalizer_output_level_right = self.app_builder.get_object(
            'equalizer_output_level_right')

        self.equalizer_input_level_left_label = self.app_builder.get_object(
            'equalizer_input_level_left_label')
        self.equalizer_input_level_right_label = self.app_builder.get_object(
            'equalizer_input_level_right_label')
        self.equalizer_output_level_left_label = self.app_builder.get_object(
            'equalizer_output_level_left_label')
        self.equalizer_output_level_right_label = self.app_builder.get_object(
            'equalizer_output_level_right_label')

        self.init_menu()

    def init_menu(self):
        builder = Gtk.Builder()

        builder.add_from_file(self.module_path + '/ui/equalizer_menu.glade')

        builder.connect_signals(self)

        menu = builder.get_object('menu')
        self.eq_highpass_cutoff_freq = builder.get_object(
            'eq_highpass_cutoff_freq')
        self.eq_highpass_poles = builder.get_object(
            'eq_highpass_poles')
        self.eq_lowpass_cutoff_freq = builder.get_object(
            'eq_lowpass_cutoff_freq')
        self.eq_lowpass_poles = builder.get_object(
            'eq_lowpass_poles')

        button = self.app_builder.get_object('equalizer_popover')

        popover = Gtk.Popover.new(button)
        popover.props.transitions_enabled = True
        popover.add(menu)

        def button_clicked(arg):
            if popover.get_visible():
                popover.hide()
            else:
                popover.show_all()

        button.connect("clicked", button_clicked)

    def init(self):
        eq_highpass_cutoff_freq_user = self.settings.get_value(
            'equalizer-highpass-cutoff').unpack()
        eq_highpass_poles_user = self.settings.get_value(
            'equalizer-highpass-poles').unpack()

        eq_lowpass_cutoff_freq_user = self.settings.get_value(
            'equalizer-lowpass-cutoff').unpack()
        eq_lowpass_poles_user = self.settings.get_value(
            'equalizer-lowpass-poles').unpack()

        equalizer_input_gain_user = self.settings.get_value(
            'equalizer-input-gain').unpack()
        equalizer_output_gain_user = self.settings.get_value(
            'equalizer-output-gain').unpack()

        self.eq_band_user = self.settings.get_value('equalizer-user').unpack()

        self.equalizer_input_gain.set_value(equalizer_input_gain_user)
        self.equalizer_output_gain.set_value(equalizer_output_gain_user)
        self.apply_eq_preset(self.eq_band_user)
        self.eq_highpass_cutoff_freq.set_value(eq_highpass_cutoff_freq_user)
        self.eq_highpass_poles.set_value(eq_highpass_poles_user)
        self.eq_lowpass_cutoff_freq.set_value(eq_lowpass_cutoff_freq_user)
        self.eq_lowpass_poles.set_value(eq_lowpass_poles_user)

        # we need this when saved value is equal to widget default value

        value_linear = 10**(equalizer_input_gain_user / 20)
        self.effects.set_eq_input_gain(value_linear)

        value_linear = 10**(equalizer_output_gain_user / 20)
        self.effects.set_eq_output_gain(value_linear)

        self.effects.set_eq_band0(self.eq_band_user[0])
        self.effects.set_eq_band1(self.eq_band_user[1])
        self.effects.set_eq_band2(self.eq_band_user[2])
        self.effects.set_eq_band3(self.eq_band_user[3])
        self.effects.set_eq_band4(self.eq_band_user[4])
        self.effects.set_eq_band5(self.eq_band_user[5])
        self.effects.set_eq_band6(self.eq_band_user[6])
        self.effects.set_eq_band7(self.eq_band_user[7])
        self.effects.set_eq_band8(self.eq_band_user[8])
        self.effects.set_eq_band9(self.eq_band_user[9])
        self.effects.set_eq_band10(self.eq_band_user[10])
        self.effects.set_eq_band11(self.eq_band_user[11])
        self.effects.set_eq_band12(self.eq_band_user[12])
        self.effects.set_eq_band13(self.eq_band_user[13])
        self.effects.set_eq_band14(self.eq_band_user[14])

        self.effects.set_eq_highpass_cutoff_freq(eq_highpass_cutoff_freq_user)
        self.effects.set_eq_lowpass_cutoff_freq(eq_lowpass_cutoff_freq_user)

    def connect_signals(self):
        self.effects.connect('new_equalizer_input_level',
                             self.on_new_equalizer_input_level)

        self.effects.connect('new_equalizer_output_level',
                             self.on_new_equalizer_output_level)

    def on_new_equalizer_input_level(self, obj, left, right):
        if left >= -99:
            l_value = 10**(left / 20)
            self.equalizer_input_level_left.set_value(l_value)
            self.equalizer_input_level_left_label.set_text(
                str(round(left)))
        else:
            self.equalizer_input_level_left.set_value(0)
            self.equalizer_input_level_left_label.set_text('-99')

        if right >= -99:
            r_value = 10**(right / 20)
            self.equalizer_input_level_right.set_value(r_value)
            self.equalizer_input_level_right_label.set_text(
                str(round(right)))
        else:
            self.equalizer_input_level_right.set_value(0)
            self.equalizer_input_level_right_label.set_text('-99')

    def on_new_equalizer_output_level(self, obj, left, right):
        if left >= -99:
            l_value = 10**(left / 20)
            self.equalizer_output_level_left.set_value(l_value)
            self.equalizer_output_level_left_label.set_text(
                str(round(left)))
        else:
            self.equalizer_output_level_left.set_value(0)
            self.equalizer_output_level_left_label.set_text('-99')

        if right >= -99:
            r_value = 10**(right / 20)
            self.equalizer_output_level_right.set_value(r_value)
            self.equalizer_output_level_right_label.set_text(
                str(round(right)))
        else:
            self.equalizer_output_level_right.set_value(0)
            self.equalizer_output_level_right_label.set_text('-99')

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

        if obj_id == 'equal_loudness_20':
            value = self.settings.get_value('equalizer-equal-loudness-20')
            self.apply_eq_preset(value)
        elif obj_id == 'equal_loudness_40':
            value = self.settings.get_value('equalizer-equal-loudness-40')
            self.apply_eq_preset(value)
        elif obj_id == 'equal_loudness_60':
            value = self.settings.get_value('equalizer-equal-loudness-60')
            self.apply_eq_preset(value)
        elif obj_id == 'equal_loudness_80':
            value = self.settings.get_value('equalizer-equal-loudness-80')
            self.apply_eq_preset(value)
        elif obj_id == 'equal_loudness_100':
            value = self.settings.get_value('equalizer-equal-loudness-100')
            self.apply_eq_preset(value)
        elif obj_id == 'flat':
            value = self.settings.get_value('equalizer-flat')
            self.apply_eq_preset(value)

    def save_eq_user(self, idx, value):
        self.eq_band_user[idx] = value

        out = GLib.Variant('ad', self.eq_band_user)

        self.settings.set_value('equalizer-user', out)

    def on_equalizer_input_gain_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20)

        self.effects.set_eq_input_gain(value_linear)

        out = GLib.Variant('d', value_db)

        self.settings.set_value('equalizer-input-gain', out)

    def on_equalizer_output_gain_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20)

        self.effects.set_eq_output_gain(value_linear)

        out = GLib.Variant('d', value_db)

        self.settings.set_value('equalizer-output-gain', out)

    def on_eq_band0_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band0(value)
        self.save_eq_user(0, value)

    def on_eq_band1_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band1(value)
        self.save_eq_user(1, value)

    def on_eq_band2_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band2(value)
        self.save_eq_user(2, value)

    def on_eq_band3_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band3(value)
        self.save_eq_user(3, value)

    def on_eq_band4_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band4(value)
        self.save_eq_user(4, value)

    def on_eq_band5_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band5(value)
        self.save_eq_user(5, value)

    def on_eq_band6_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band6(value)
        self.save_eq_user(6, value)

    def on_eq_band7_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band7(value)
        self.save_eq_user(7, value)

    def on_eq_band8_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band8(value)
        self.save_eq_user(8, value)

    def on_eq_band9_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band9(value)
        self.save_eq_user(9, value)

    def on_eq_band10_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band10(value)
        self.save_eq_user(10, value)

    def on_eq_band11_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band11(value)
        self.save_eq_user(11, value)

    def on_eq_band12_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band12(value)
        self.save_eq_user(12, value)

    def on_eq_band13_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band13(value)
        self.save_eq_user(13, value)

    def on_eq_band14_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_band14(value)
        self.save_eq_user(14, value)

    def on_eq_highpass_cutoff_freq_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_highpass_cutoff_freq(value)

        out = GLib.Variant('i', value)

        self.settings.set_value('equalizer-highpass-cutoff', out)

    def on_eq_highpass_poles_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_highpass_poles(value)

        out = GLib.Variant('i', value)

        self.settings.set_value('equalizer-highpass-poles', out)

    def on_eq_lowpass_cutoff_freq_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_lowpass_cutoff_freq(value)

        out = GLib.Variant('i', value)

        self.settings.set_value('equalizer-lowpass-cutoff', out)

    def on_eq_lowpass_poles_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_eq_lowpass_poles(value)

        out = GLib.Variant('i', value)

        self.settings.set_value('equalizer-lowpass-poles', out)

    def reset(self):
        self.settings.reset('equalizer-highpass-cutoff')
        self.settings.reset('equalizer-highpass-poles')
        self.settings.reset('equalizer-lowpass-cutoff')
        self.settings.reset('equalizer-lowpass-poles')
        self.settings.reset('equalizer-input-gain')
        self.settings.reset('equalizer-output-gain')
        self.settings.reset('equalizer-user')

        self.init()
