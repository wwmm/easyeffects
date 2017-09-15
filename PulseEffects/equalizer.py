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
            setattr(self, 'band' + str(n), equalizer.get_child_by_index(n))

        # It seems there is a bug in the low shelf filter.
        # When we increase the lower shelf gain higher frequencies
        # are attenuated. Setting the first band to peak type instead of
        # shelf fixes this.

        self.band0.set_property('type', 0)  # 0: peak type
        self.band14.set_property('type', 0)  # 0: peak type

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
            setattr(self, 'ui_band' + str(n) + '_g',
                    self.builder.get_object('band' + str(n) + '_g'))

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

        # band menu
        for n in range(15):
            menu_builder = Gtk.Builder()
            menu_builder.add_from_file(self.module_path +
                                       '/ui/equalizer_band_menu.glade')

            menu_builder.connect_signals(self)

            menu_button = self.builder.get_object('band' + str(n) +
                                                  '_menu_button')
            band_f = menu_builder.get_object('band_f')
            band_q = menu_builder.get_object('band_q')

            band_f.connect('value-changed', self.on_frequency_changed, n)
            band_q.connect('value-changed', self.on_quality_changed, n)

            band_menu = menu_builder.get_object('menu')

            band_menu.set_relative_to(self.ui_window)
            menu_button.set_popover(band_menu)

            setattr(self, 'ui_band' + str(n) + '_f', band_f)
            setattr(self, 'ui_band' + str(n) + '_q', band_q)

    def bind(self):
        # binding ui widgets to gstreamer plugins

        flag = GObject.BindingFlags.DEFAULT

        for n in range(15):
            getattr(self, 'ui_band' + str(n) + '_g').bind_property(
                'value', getattr(self, 'band' + str(n)), 'gain', flag)

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

        for n in range(15):
            ui_band_g = getattr(self, 'ui_band' + str(n) + '_g')
            ui_band_f = getattr(self, 'ui_band' + str(n) + '_f')
            ui_band_q = getattr(self, 'ui_band' + str(n) + '_q')

            prop = 'equalizer-band' + str(n) + '-gain'
            self.settings.bind(prop, ui_band_g, 'value', flag)

            prop = 'equalizer-band' + str(n) + '-frequency'
            self.settings.bind(prop, ui_band_f, 'value', flag)

            prop = 'equalizer-band' + str(n) + '-quality'
            self.settings.bind(prop, ui_band_q, 'value', flag)

    def init_ui(self):
        # self.init_eq_freq_and_qfactors()
        pass

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
            getattr(self, 'band' + str(n)).set_property(
                'freq', self.freqs[n])

            w = self.freqs[n] / self.qfactors[n]

            getattr(self, 'band' + str(n)).set_property('bandwidth', w)

    def print_eq_freqs_and_widths(self):
        for n in range(15):
            f = getattr(self, 'band' + str(n)).get_property('freq')
            w = getattr(self, 'band' + str(n)).get_property('bandwidth')

            print(f, w)

    def on_input_gain_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20.0)

        self.input_gain.set_property('volume', value_linear)

    def on_output_gain_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20.0)

        self.output_gain.set_property('volume', value_linear)

    def on_frequency_changed(self, obj, idx):
        value = obj.get_value()

        band = getattr(self, 'band' + str(idx))

        q = self.settings.get_value('equalizer-band' + str(idx) + '-quality')

        band.set_property('freq', value)
        band.set_property('bandwidth', value / q.unpack())

    def on_quality_changed(self, obj, idx):
        value = obj.get_value()

        band = getattr(self, 'band' + str(idx))

        f = self.settings.get_value('equalizer-band' + str(idx) + '-frequency')

        band.set_property('bandwidth', f.unpack() / value)

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
        for n in range(15):
            self.settings.reset('equalizer-band' + str(n) + '-gain')

    def on_eq_reset_freqs_button_clicked(self, obj):
        for n in range(15):
            self.settings.reset('equalizer-band' + str(n) + '-frequency')

    def on_eq_reset_qfactors_button_clicked(self, obj):
        for n in range(15):
            self.settings.reset('equalizer-band' + str(n) + '-quality')

    def on_eq_calibrate_button_clicked(self, obj):
        c = Calibration()
        c.run()

    def reset(self):
        self.settings.reset('equalizer-state')
        self.settings.reset('equalizer-input-gain')
        self.settings.reset('equalizer-output-gain')

        for n in range(15):
            self.settings.reset('equalizer-band' + str(n) + '-gain')
            self.settings.reset('equalizer-band' + str(n) + '-frequency')
            self.settings.reset('equalizer-band' + str(n) + '-quality')
