# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gst, GstInsertBin, Gtk
from PulseEffectsCalibration.application import Application as Calibration


Gst.init(None)


class Equalizer():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.builder = Gtk.Builder()

        self.builder.add_from_file(self.module_path + '/ui/equalizer.glade')

        self.build_equalizer_bin()

        self.load_ui()

        self.builder.connect_signals(self)

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
        self.ui_window = self.builder.get_object('window')

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

    def init_ui(self):
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

    def save_eq_user(self, idx, value):
        self.eq_band_user[idx] = value

        out = GLib.Variant('ad', self.eq_band_user)

        self.settings.set_value('equalizer-user', out)

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
        self.settings.reset('equalizer-input-gain')
        self.settings.reset('equalizer-output-gain')
        self.settings.reset('equalizer-user')
        self.settings.reset('equalizer-freqs')
        self.settings.reset('equalizer-qfactors')
