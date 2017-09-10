# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GObject, GLib, Gst, GstInsertBin, Gtk
from PulseEffectsCalibration.application import Application as Calibration


Gst.init(None)


class Equalizer():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.build_bin()
        self.load_ui()

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_bin(self):
        self.input_gain = Gst.ElementFactory.make('volume', None)
        self.output_gain = Gst.ElementFactory.make('volume', None)
        equalizer = Gst.ElementFactory.make('equalizer-nbands', None)
        input_level = Gst.ElementFactory.make('level', 'equalizer_input_level')
        output_level = Gst.ElementFactory.make('level',
                                               'equalizer_output_level')

        equalizer.set_property('num-bands', 15)

        for n in range(15):
            setattr(self, 'eq_band' + str(n), equalizer.get_child_by_index(n))

        # It seems there is a bug in the low shelf filter.
        # When we increase the lower shelf gain higher frequencies
        # are attenuated. Setting the first band to peak type instead of
        # shelf fixes this.

        self.eq_band0.set_property('type', 0)  # 0: peak type
        self.eq_band14.set_property('type', 0)  # 0: peak type

        self.bin = GstInsertBin.InsertBin.new('equalizer_bin')

        self.bin.append(self.input_gain, self.on_filter_added, None)
        self.bin.append(input_level, self.on_filter_added, None)
        self.bin.append(equalizer, self.on_filter_added, None)
        self.bin.append(self.output_gain, self.on_filter_added, None)
        self.bin.append(output_level, self.on_filter_added, None)

    def load_ui(self):
        self.builder = Gtk.Builder()
        self.builder.add_from_file(self.module_path + '/ui/equalizer.glade')
        self.builder.connect_signals(self)

        self.ui_window = self.builder.get_object('window')
        self.ui_controls = self.builder.get_object('controls')

        self.ui_enable = self.builder.get_object('enable')

        self.ui_input_gain = self.builder.get_object('input_gain')
        self.ui_output_gain = self.builder.get_object('output_gain')

        for n in range(15):
            setattr(self, 'ui_band' + str(n),
                    self.builder.get_object('band' + str(n) + '_g'))

            setattr(self, 'ui_band' + str(n) + '_f',
                    self.builder.get_object('band' + str(n) + '_f'))

            setattr(self, 'ui_band' + str(n) + '_q',
                    self.builder.get_object('band' + str(n) + '_q'))

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
        # binding ui widgets to gstreamer plugins

        flag = GObject.BindingFlags.DEFAULT

        # self.ui_attack.bind_property('value', self.compressor, 'attack-time',
        #                              flag)

        # binding ui widgets to gsettings

        flag = Gio.SettingsBindFlags.DEFAULT

        self.settings.bind('equalizer-state', self.ui_enable,
                           'active', flag)
        self.settings.bind('equalizer-state', self.ui_controls,
                           'sensitive', Gio.SettingsBindFlags.GET)
        self.settings.bind('equalizer-input-gain', self.ui_input_gain, 'value',
                           flag)
        self.settings.bind('equalizer-output-gain', self.ui_output_gain,
                           'value', flag)

    def init_ui(self):
        self.eq_band_user = self.settings.get_value('equalizer-user').unpack()

        self.apply_eq_preset(self.eq_band_user)

        self.init_eq_freq_and_qfactors()

    def init_eq_freq_and_qfactors(self):
        self.freqs = self.settings.get_value('equalizer-freqs').unpack()
        self.qfactors = self.settings.get_value(
            'equalizer-qfactors').unpack()

        for n in range(len(self.freqs)):
            # init frequencies widgets
            getattr(self, 'ui_band' + str(n) + '_f').set_text(
                '{0:g}'.format(self.freqs[n]))

            # init quality factors widgets
            getattr(self, 'ui_band' + str(n) + '_q').set_text(
                str(self.qfactors[n]))

            # init plugin properties
            getattr(self, 'eq_band' + str(n)).set_property(
                'freq', self.freqs[n])

            w = self.freqs[n] / self.qfactors[n]

            getattr(self, 'eq_band' + str(n)).set_property('bandwidth', w)

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
        for n in range(len(values)):
            getattr(self, 'ui_band' + str(n)).set_value(values[n])

    def save_eq_user(self, idx, value):
        self.eq_band_user[idx] = value

        out = GLib.Variant('ad', self.eq_band_user)

        self.settings.set_value('equalizer-user', out)

    def on_input_gain_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20.0)

        self.input_gain.set_property('volume', value_linear)

    def on_output_gain_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20.0)

        self.output_gain.set_property('volume', value_linear)

    def on_band_gain_changed(self, obj):
        obj_id = Gtk.Buildable.get_name(obj)
        value = obj.get_value()

        # example glade id: band0_g
        idx = int(obj_id.split('_')[0].split('d')[1])

        getattr(self, 'eq_band' + str(idx)).set_property('gain', value)
        self.save_eq_user(idx, value)

    def on_eq_freq_changed(self, obj):
        try:
            value = float(obj.get_text())

            obj_id = Gtk.Buildable.get_name(obj)

            if obj_id == 'band0_f':
                self.freqs[0] = value
                qfactor = self.qfactors[0]
                self.eq_band0.set_property('freq', value)
                self.eq_band0.set_property('bandwidth', value / qfactor)
            elif obj_id == 'band1_f':
                self.freqs[1] = value
                qfactor = self.qfactors[1]
                self.eq_band1.set_property('freq', value)
                self.eq_band1.set_property('bandwidth', value / qfactor)
            elif obj_id == 'band2_f':
                self.freqs[2] = value
                qfactor = self.qfactors[2]
                self.eq_band2.set_property('freq', value)
                self.eq_band2.set_property('bandwidth', value / qfactor)
            elif obj_id == 'band3_f':
                self.freqs[3] = value
                qfactor = self.qfactors[3]
                self.eq_band3.set_property('freq', value)
                self.eq_band3.set_property('bandwidth', value / qfactor)
            elif obj_id == 'band4_f':
                self.freqs[4] = value
                qfactor = self.qfactors[4]
                self.eq_band4.set_property('freq', value)
                self.eq_band4.set_property('bandwidth', value / qfactor)
            elif obj_id == 'band5_f':
                self.freqs[5] = value
                qfactor = self.qfactors[5]
                self.eq_band5.set_property('freq', value)
                self.eq_band5.set_property('bandwidth', value / qfactor)
            elif obj_id == 'band6_f':
                self.freqs[6] = value
                qfactor = self.qfactors[6]
                self.eq_band6.set_property('freq', value)
                self.eq_band6.set_property('bandwidth', value / qfactor)
            elif obj_id == 'band7_f':
                self.freqs[7] = value
                qfactor = self.qfactors[7]
                self.eq_band7.set_property('freq', value)
                self.eq_band7.set_property('bandwidth', value / qfactor)
            elif obj_id == 'band8_f':
                self.freqs[8] = value
                qfactor = self.qfactors[8]
                self.eq_band8.set_property('freq', value)
                self.eq_band8.set_property('bandwidth', value / qfactor)
            elif obj_id == 'band9_f':
                self.freqs[9] = value
                qfactor = self.qfactors[9]
                self.eq_band9.set_property('freq', value)
                self.eq_band9.set_property('bandwidth', value / qfactor)
            elif obj_id == 'band10_q':
                self.freqs[10] = value
                qfactor = self.qfactors[10]
                self.eq_band10.set_property('freq', value)
                self.eq_band10.set_property('bandwidth', value / qfactor)
            elif obj_id == 'band11_q':
                self.freqs[11] = value
                qfactor = self.qfactors[11]
                self.eq_band11.set_property('freq', value)
                self.eq_band11.set_property('bandwidth', value / qfactor)
            elif obj_id == 'band12_q':
                self.freqs[12] = value
                qfactor = self.qfactors[12]
                self.eq_band12.set_property('freq', value)
                self.eq_band12.set_property('bandwidth', value / qfactor)
            elif obj_id == 'band13_q':
                self.freqs[13] = value
                qfactor = self.qfactors[13]
                self.eq_band13.set_property('freq', value)
                self.eq_band13.set_property('bandwidth', value / qfactor)
            elif obj_id == 'band14_f':
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

            if obj_id == 'band0_q':
                self.qfactors[0] = value
                self.eq_band0.set_property('bandwidth', self.freqs[0] / value)
            elif obj_id == 'band1_q':
                self.qfactors[1] = value
                self.eq_band1.set_property('bandwidth', self.freqs[1] / value)
            elif obj_id == 'band2_q':
                self.qfactors[2] = value
                self.eq_band2.set_property('bandwidth', self.freqs[2] / value)
            elif obj_id == 'band3_q':
                self.qfactors[3] = value
                self.eq_band3.set_property('bandwidth', self.freqs[3] / value)
            elif obj_id == 'band4_q':
                self.qfactors[4] = value
                self.eq_band4.set_property('bandwidth', self.freqs[4] / value)
            elif obj_id == 'band5_q':
                self.qfactors[5] = value
                self.eq_band5.set_property('bandwidth', self.freqs[5] / value)
            elif obj_id == 'band6_q':
                self.qfactors[6] = value
                self.eq_band6.set_property('bandwidth', self.freqs[6] / value)
            elif obj_id == 'band7_q':
                self.qfactors[7] = value
                self.eq_band7.set_property('bandwidth', self.freqs[7] / value)
            elif obj_id == 'band8_q':
                self.qfactors[8] = value
                self.eq_band8.set_property('bandwidth', self.freqs[8] / value)
            elif obj_id == 'band9_q':
                self.qfactors[9] = value
                self.eq_band9.set_property('bandwidth', self.freqs[9] / value)
            elif obj_id == 'band10_q':
                self.qfactors[10] = value
                self.eq_band10.set_property(
                    'bandwidth', self.freqs[10] / value)
            elif obj_id == 'band11_q':
                self.qfactors[11] = value
                self.eq_band11.set_property(
                    'bandwidth', self.freqs[11] / value)
            elif obj_id == 'band12_q':
                self.qfactors[12] = value
                self.eq_band12.set_property(
                    'bandwidth', self.freqs[12] / value)
            elif obj_id == 'band13_q':
                self.qfactors[13] = value
                self.eq_band13.set_property(
                    'bandwidth', self.freqs[13] / value)
            elif obj_id == 'band14_q':
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
