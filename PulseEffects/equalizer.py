# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GLib, Gst, GstInsertBin, Gtk
from PulseEffectsCalibration.application import Application as Calibration


Gst.init(None)


class Equalizer():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.build_equalizer_bin()
        self.load_ui()

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_equalizer_bin(self):
        self.equalizer_input_gain = Gst.ElementFactory.make('volume', None)
        self.equalizer_output_gain = Gst.ElementFactory.make('volume', None)
        equalizer = Gst.ElementFactory.make('equalizer-nbands', None)
        equalizer_input_level = Gst.ElementFactory.make(
            'level', 'equalizer_input_level')
        equalizer_output_level = Gst.ElementFactory.make(
            'level', 'equalizer_output_level')

        equalizer.set_property('num-bands', 15)

        self.eq_band0 = equalizer.get_child_by_index(0)
        self.eq_band1 = equalizer.get_child_by_index(1)
        self.eq_band2 = equalizer.get_child_by_index(2)
        self.eq_band3 = equalizer.get_child_by_index(3)
        self.eq_band4 = equalizer.get_child_by_index(4)
        self.eq_band5 = equalizer.get_child_by_index(5)
        self.eq_band6 = equalizer.get_child_by_index(6)
        self.eq_band7 = equalizer.get_child_by_index(7)
        self.eq_band8 = equalizer.get_child_by_index(8)
        self.eq_band9 = equalizer.get_child_by_index(9)
        self.eq_band10 = equalizer.get_child_by_index(10)
        self.eq_band11 = equalizer.get_child_by_index(11)
        self.eq_band12 = equalizer.get_child_by_index(12)
        self.eq_band13 = equalizer.get_child_by_index(13)
        self.eq_band14 = equalizer.get_child_by_index(14)

        # It seems there is a bug in the low shelf filter.
        # When we increase the lower shelf gain higher frequencies
        # are attenuated. Setting the first band to peak type instead of
        # shelf fixes this.

        self.eq_band0.set_property('type', 0)  # 0: peak type
        self.eq_band14.set_property('type', 0)  # 0: peak type

        self.bin = GstInsertBin.InsertBin.new('equalizer_bin')

        self.bin.append(self.equalizer_input_gain, self.on_filter_added, None)
        self.bin.append(equalizer_input_level, self.on_filter_added, None)
        self.bin.append(equalizer, self.on_filter_added, None)
        self.bin.append(self.equalizer_output_gain, self.on_filter_added, None)
        self.bin.append(equalizer_output_level, self.on_filter_added, None)

    def load_ui(self):
        self.builder = Gtk.Builder()
        self.builder.add_from_file(self.module_path + '/ui/equalizer.glade')
        self.builder.connect_signals(self)

        self.ui_window = self.builder.get_object('window')
        self.ui_equalizer_controls = self.builder.get_object(
            'equalizer_controls')

        self.ui_equalizer_enable = self.builder.get_object('equalizer_enable')

        self.ui_equalizer_input_gain = self.builder.get_object(
            'equalizer_input_gain')
        self.ui_equalizer_output_gain = self.builder.get_object(
            'equalizer_output_gain')

        self.ui_band0 = self.builder.get_object('eq_band0')
        self.ui_band1 = self.builder.get_object('eq_band1')
        self.ui_band2 = self.builder.get_object('eq_band2')
        self.ui_band3 = self.builder.get_object('eq_band3')
        self.ui_band4 = self.builder.get_object('eq_band4')
        self.ui_band5 = self.builder.get_object('eq_band5')
        self.ui_band6 = self.builder.get_object('eq_band6')
        self.ui_band7 = self.builder.get_object('eq_band7')
        self.ui_band8 = self.builder.get_object('eq_band8')
        self.ui_band9 = self.builder.get_object('eq_band9')
        self.ui_band10 = self.builder.get_object('eq_band10')
        self.ui_band11 = self.builder.get_object('eq_band11')
        self.ui_band12 = self.builder.get_object('eq_band12')
        self.ui_band13 = self.builder.get_object('eq_band13')
        self.ui_band14 = self.builder.get_object('eq_band14')

        self.ui_band0_f = self.builder.get_object('eq_band0_freq')
        self.ui_band1_f = self.builder.get_object('eq_band1_freq')
        self.ui_band2_f = self.builder.get_object('eq_band2_freq')
        self.ui_band3_f = self.builder.get_object('eq_band3_freq')
        self.ui_band4_f = self.builder.get_object('eq_band4_freq')
        self.ui_band5_f = self.builder.get_object('eq_band5_freq')
        self.ui_band6_f = self.builder.get_object('eq_band6_freq')
        self.ui_band7_f = self.builder.get_object('eq_band7_freq')
        self.ui_band8_f = self.builder.get_object('eq_band8_freq')
        self.ui_band9_f = self.builder.get_object('eq_band9_freq')
        self.ui_band10_f = self.builder.get_object('eq_band10_freq')
        self.ui_band11_f = self.builder.get_object('eq_band11_freq')
        self.ui_band12_f = self.builder.get_object('eq_band12_freq')
        self.ui_band13_f = self.builder.get_object('eq_band13_freq')
        self.ui_band14_f = self.builder.get_object('eq_band14_freq')

        self.ui_band0_q = self.builder.get_object('eq_band0_qfactor')
        self.ui_band1_q = self.builder.get_object('eq_band1_qfactor')
        self.ui_band2_q = self.builder.get_object('eq_band2_qfactor')
        self.ui_band3_q = self.builder.get_object('eq_band3_qfactor')
        self.ui_band4_q = self.builder.get_object('eq_band4_qfactor')
        self.ui_band5_q = self.builder.get_object('eq_band5_qfactor')
        self.ui_band6_q = self.builder.get_object('eq_band6_qfactor')
        self.ui_band7_q = self.builder.get_object('eq_band7_qfactor')
        self.ui_band8_q = self.builder.get_object('eq_band8_qfactor')
        self.ui_band9_q = self.builder.get_object('eq_band9_qfactor')
        self.ui_band10_q = self.builder.get_object('eq_band10_qfactor')
        self.ui_band11_q = self.builder.get_object('eq_band11_qfactor')
        self.ui_band12_q = self.builder.get_object('eq_band12_qfactor')
        self.ui_band13_q = self.builder.get_object('eq_band13_qfactor')
        self.ui_band14_q = self.builder.get_object('eq_band14_qfactor')

        self.ui_eq_calibrate_button = self.builder.get_object(
            'eq_calibrate_button')

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

    def bind(self):
        self.settings.bind('equalizer-state', self.ui_equalizer_enable,
                           'active', Gio.SettingsBindFlags.DEFAULT)
        self.settings.bind('equalizer-state', self.ui_equalizer_controls,
                           'sensitive', Gio.SettingsBindFlags.GET)
        self.settings.bind('equalizer-input-gain',
                           self.ui_equalizer_input_gain, 'value',
                           Gio.SettingsBindFlags.GET)
        self.settings.bind('equalizer-output-gain',
                           self.ui_equalizer_output_gain, 'value',
                           Gio.SettingsBindFlags.GET)

    def init_ui(self):
        self.eq_band_user = self.settings.get_value('equalizer-user').unpack()

        self.apply_eq_preset(self.eq_band_user)

        self.init_eq_freq_and_qfactors()

    def init_eq_freq_and_qfactors(self):
        self.freqs = self.settings.get_value('equalizer-freqs').unpack()
        self.qfactors = self.settings.get_value(
            'equalizer-qfactors').unpack()

        self.ui_band0_f.set_text('{0:g}'.format(self.freqs[0]))
        self.ui_band1_f.set_text('{0:g}'.format(self.freqs[1]))
        self.ui_band2_f.set_text('{0:g}'.format(self.freqs[2]))
        self.ui_band3_f.set_text('{0:g}'.format(self.freqs[3]))
        self.ui_band4_f.set_text('{0:g}'.format(self.freqs[4]))
        self.ui_band5_f.set_text('{0:g}'.format(self.freqs[5]))
        self.ui_band6_f.set_text('{0:g}'.format(self.freqs[6]))
        self.ui_band7_f.set_text('{0:g}'.format(self.freqs[7]))
        self.ui_band8_f.set_text('{0:g}'.format(self.freqs[8]))
        self.ui_band9_f.set_text('{0:g}'.format(self.freqs[9]))
        self.ui_band10_f.set_text('{0:g}'.format(self.freqs[10]))
        self.ui_band11_f.set_text('{0:g}'.format(self.freqs[11]))
        self.ui_band12_f.set_text('{0:g}'.format(self.freqs[12]))
        self.ui_band13_f.set_text('{0:g}'.format(self.freqs[13]))
        self.ui_band14_f.set_text('{0:g}'.format(self.freqs[14]))

        self.ui_band0_q.set_text(str(self.qfactors[0]))
        self.ui_band1_q.set_text(str(self.qfactors[1]))
        self.ui_band2_q.set_text(str(self.qfactors[2]))
        self.ui_band3_q.set_text(str(self.qfactors[3]))
        self.ui_band4_q.set_text(str(self.qfactors[4]))
        self.ui_band5_q.set_text(str(self.qfactors[5]))
        self.ui_band6_q.set_text(str(self.qfactors[6]))
        self.ui_band7_q.set_text(str(self.qfactors[7]))
        self.ui_band8_q.set_text(str(self.qfactors[8]))
        self.ui_band9_q.set_text(str(self.qfactors[9]))
        self.ui_band10_q.set_text(str(self.qfactors[10]))
        self.ui_band11_q.set_text(str(self.qfactors[11]))
        self.ui_band12_q.set_text(str(self.qfactors[12]))
        self.ui_band13_q.set_text(str(self.qfactors[13]))
        self.ui_band14_q.set_text(str(self.qfactors[14]))

        # pipeline

        self.eq_band0.set_property('freq', self.freqs[0])
        self.eq_band1.set_property('freq', self.freqs[1])
        self.eq_band2.set_property('freq', self.freqs[2])
        self.eq_band3.set_property('freq', self.freqs[3])
        self.eq_band4.set_property('freq', self.freqs[4])
        self.eq_band5.set_property('freq', self.freqs[5])
        self.eq_band6.set_property('freq', self.freqs[6])
        self.eq_band7.set_property('freq', self.freqs[7])
        self.eq_band8.set_property('freq', self.freqs[8])
        self.eq_band9.set_property('freq', self.freqs[9])
        self.eq_band10.set_property('freq', self.freqs[10])
        self.eq_band11.set_property('freq', self.freqs[11])
        self.eq_band12.set_property('freq', self.freqs[12])
        self.eq_band13.set_property('freq', self.freqs[13])
        self.eq_band14.set_property('freq', self.freqs[14])

        self.eq_band0.set_property('bandwidth',
                                   self.freqs[0] / self.qfactors[0])
        self.eq_band1.set_property('bandwidth',
                                   self.freqs[1] / self.qfactors[1])
        self.eq_band2.set_property('bandwidth',
                                   self.freqs[2] / self.qfactors[2])
        self.eq_band3.set_property('bandwidth',
                                   self.freqs[3] / self.qfactors[3])
        self.eq_band4.set_property('bandwidth',
                                   self.freqs[4] / self.qfactors[4])
        self.eq_band5.set_property('bandwidth',
                                   self.freqs[5] / self.qfactors[5])
        self.eq_band6.set_property('bandwidth',
                                   self.freqs[6] / self.qfactors[6])
        self.eq_band7.set_property('bandwidth',
                                   self.freqs[7] / self.qfactors[7])
        self.eq_band8.set_property('bandwidth',
                                   self.freqs[8] / self.qfactors[8])
        self.eq_band9.set_property('bandwidth',
                                   self.freqs[9] / self.qfactors[9])
        self.eq_band10.set_property('bandwidth',
                                    self.freqs[10] / self.qfactors[10])
        self.eq_band11.set_property('bandwidth',
                                    self.freqs[11] / self.qfactors[11])
        self.eq_band12.set_property('bandwidth',
                                    self.freqs[12] / self.qfactors[12])
        self.eq_band13.set_property('bandwidth',
                                    self.freqs[13] / self.qfactors[13])
        self.eq_band14.set_property('bandwidth',
                                    self.freqs[14] / self.qfactors[14])

    def print_eq_freqs(self):
        print(self.eq_band0.get_property('freq'))
        print(self.eq_band1.get_property('freq'))
        print(self.eq_band2.get_property('freq'))
        print(self.eq_band3.get_property('freq'))
        print(self.eq_band4.get_property('freq'))
        print(self.eq_band5.get_property('freq'))
        print(self.eq_band6.get_property('freq'))
        print(self.eq_band7.get_property('freq'))
        print(self.eq_band8.get_property('freq'))
        print(self.eq_band9.get_property('freq'))
        print(self.eq_band10.get_property('freq'))
        print(self.eq_band11.get_property('freq'))
        print(self.eq_band12.get_property('freq'))
        print(self.eq_band13.get_property('freq'))
        print(self.eq_band14.get_property('freq'))

    def print_eq_bandwwidths(self):
        print(self.eq_band0.get_property('bandwidth'))
        print(self.eq_band1.get_property('bandwidth'))
        print(self.eq_band2.get_property('bandwidth'))
        print(self.eq_band3.get_property('bandwidth'))
        print(self.eq_band4.get_property('bandwidth'))
        print(self.eq_band5.get_property('bandwidth'))
        print(self.eq_band6.get_property('bandwidth'))
        print(self.eq_band7.get_property('bandwidth'))
        print(self.eq_band8.get_property('bandwidth'))
        print(self.eq_band9.get_property('bandwidth'))
        print(self.eq_band10.get_property('bandwidth'))
        print(self.eq_band11.get_property('bandwidth'))
        print(self.eq_band12.get_property('bandwidth'))
        print(self.eq_band13.get_property('bandwidth'))
        print(self.eq_band14.get_property('bandwidth'))

    def apply_eq_preset(self, values):
        self.ui_band0.set_value(values[0])
        self.ui_band1.set_value(values[1])
        self.ui_band2.set_value(values[2])
        self.ui_band3.set_value(values[3])
        self.ui_band4.set_value(values[4])
        self.ui_band5.set_value(values[5])
        self.ui_band6.set_value(values[6])
        self.ui_band7.set_value(values[7])
        self.ui_band8.set_value(values[8])
        self.ui_band9.set_value(values[9])
        self.ui_band10.set_value(values[10])
        self.ui_band11.set_value(values[11])
        self.ui_band12.set_value(values[12])
        self.ui_band13.set_value(values[13])
        self.ui_band14.set_value(values[14])

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

    def save_eq_user(self, idx, value):
        self.eq_band_user[idx] = value

        out = GLib.Variant('ad', self.eq_band_user)

        self.settings.set_value('equalizer-user', out)

    def on_equalizer_input_gain_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20.0)

        self.equalizer_input_gain.set_property('volume', value_linear)

    def on_equalizer_output_gain_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20.0)

        self.equalizer_output_gain.set_property('volume', value_linear)

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
                self.freqs[0] = value
                qfactor = self.qfactors[0]
                self.eq_band0.set_property('freq', value)
                self.eq_band0.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band1_freq':
                self.freqs[1] = value
                qfactor = self.qfactors[1]
                self.eq_band1.set_property('freq', value)
                self.eq_band1.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band2_freq':
                self.freqs[2] = value
                qfactor = self.qfactors[2]
                self.eq_band2.set_property('freq', value)
                self.eq_band2.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band3_freq':
                self.freqs[3] = value
                qfactor = self.qfactors[3]
                self.eq_band3.set_property('freq', value)
                self.eq_band3.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band4_freq':
                self.freqs[4] = value
                qfactor = self.qfactors[4]
                self.eq_band4.set_property('freq', value)
                self.eq_band4.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band5_freq':
                self.freqs[5] = value
                qfactor = self.qfactors[5]
                self.eq_band5.set_property('freq', value)
                self.eq_band5.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band6_freq':
                self.freqs[6] = value
                qfactor = self.qfactors[6]
                self.eq_band6.set_property('freq', value)
                self.eq_band6.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band7_freq':
                self.freqs[7] = value
                qfactor = self.qfactors[7]
                self.eq_band7.set_property('freq', value)
                self.eq_band7.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band8_freq':
                self.freqs[8] = value
                qfactor = self.qfactors[8]
                self.eq_band8.set_property('freq', value)
                self.eq_band8.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band9_freq':
                self.freqs[9] = value
                qfactor = self.qfactors[9]
                self.eq_band9.set_property('freq', value)
                self.eq_band9.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band10_freq':
                self.freqs[10] = value
                qfactor = self.qfactors[10]
                self.eq_band10.set_property('freq', value)
                self.eq_band10.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band11_freq':
                self.freqs[11] = value
                qfactor = self.qfactors[11]
                self.eq_band11.set_property('freq', value)
                self.eq_band11.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band12_freq':
                self.freqs[12] = value
                qfactor = self.qfactors[12]
                self.eq_band12.set_property('freq', value)
                self.eq_band12.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band13_freq':
                self.freqs[13] = value
                qfactor = self.qfactors[13]
                self.eq_band13.set_property('freq', value)
                self.eq_band13.set_property('bandwidth', value / qfactor)
            elif obj_id == 'eq_band14_freq':
                self.freqs[14] = value
                qfactor = self.qfactors[14]
                self.eq_band14.set_property('freq', value)
                self.eq_band14.set_property('bandwidth', value / qfactor)

            out = GLib.Variant('ad', self.freqs)
            self.settings.set_value('equalizer-freqs', out)
        except ValueError:
            pass

    def on_eq_qfactor_changed(self, obj):
        try:
            value = float(obj.get_text())

            obj_id = Gtk.Buildable.get_name(obj)

            if obj_id == 'eq_band0_qfactor':
                self.qfactors[0] = value
                self.eq_band0.set_property('bandwidth', self.freqs[0] / value)
            elif obj_id == 'eq_band1_qfactor':
                self.qfactors[1] = value
                self.eq_band1.set_property('bandwidth', self.freqs[1] / value)
            elif obj_id == 'eq_band2_qfactor':
                self.qfactors[2] = value
                self.eq_band2.set_property('bandwidth', self.freqs[2] / value)
            elif obj_id == 'eq_band3_qfactor':
                self.qfactors[3] = value
                self.eq_band3.set_property('bandwidth', self.freqs[3] / value)
            elif obj_id == 'eq_band4_qfactor':
                self.qfactors[4] = value
                self.eq_band4.set_property('bandwidth', self.freqs[4] / value)
            elif obj_id == 'eq_band5_qfactor':
                self.qfactors[5] = value
                self.eq_band5.set_property('bandwidth', self.freqs[5] / value)
            elif obj_id == 'eq_band6_qfactor':
                self.qfactors[6] = value
                self.eq_band6.set_property('bandwidth', self.freqs[6] / value)
            elif obj_id == 'eq_band7_qfactor':
                self.qfactors[7] = value
                self.eq_band7.set_property('bandwidth', self.freqs[7] / value)
            elif obj_id == 'eq_band8_qfactor':
                self.qfactors[8] = value
                self.eq_band8.set_property('bandwidth', self.freqs[8] / value)
            elif obj_id == 'eq_band9_qfactor':
                self.qfactors[9] = value
                self.eq_band9.set_property('bandwidth', self.freqs[9] / value)
            elif obj_id == 'eq_band10_qfactor':
                self.qfactors[10] = value
                self.eq_band10.set_property(
                    'bandwidth', self.freqs[10] / value)
            elif obj_id == 'eq_band11_qfactor':
                self.qfactors[11] = value
                self.eq_band11.set_property(
                    'bandwidth', self.freqs[11] / value)
            elif obj_id == 'eq_band12_qfactor':
                self.qfactors[12] = value
                self.eq_band12.set_property(
                    'bandwidth', self.freqs[12] / value)
            elif obj_id == 'eq_band13_qfactor':
                self.qfactors[13] = value
                self.eq_band13.set_property(
                    'bandwidth', self.freqs[13] / value)
            elif obj_id == 'eq_band14_qfactor':
                self.qfactors[14] = value
                self.eq_band14.set_property(
                    'bandwidth', self.freqs[14] / value)

            out = GLib.Variant('ad', self.qfactors)
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

    def on_eq_flat_response_button_clicked(self, obj):
        self.apply_eq_preset([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0])

    def on_eq_reset_freqs_button_clicked(self, obj):
        self.settings.reset('equalizer-freqs')
        self.init_eq_freq_and_qfactors()

    def on_eq_reset_qfactors_button_clicked(self, obj):
        self.settings.reset('equalizer-qfactors')
        self.init_eq_freq_and_qfactors()

    def on_eq_calibrate_button_clicked(self, obj):
        c = Calibration()
        c.run()

    def reset(self):
        self.settings.reset('equalizer-state')
        self.settings.reset('equalizer-input-gain')
        self.settings.reset('equalizer-output-gain')
        self.settings.reset('equalizer-user')
        self.settings.reset('equalizer-freqs')
        self.settings.reset('equalizer-qfactors')
