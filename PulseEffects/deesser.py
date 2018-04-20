# -*- coding: utf-8 -*-

import logging
import os

import gi
import numpy as np
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GObject, Gst, GstInsertBin, Gtk

Gst.init(None)


class Deesser():

    def __init__(self):
        self.settings = None
        self.module_path = os.path.dirname(__file__)

        self.log = logging.getLogger('PulseEffects')

        if Gst.ElementFactory.make('calf-sourceforge-net-plugins-Deesser'):
            self.is_installed = True
        else:
            self.is_installed = False

            self.log.warn('Deesser plugin was not found. Disabling it!')

        self.build_bin()

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_bin(self):
        self.deesser = Gst.ElementFactory.make(
            'calf-sourceforge-net-plugins-Deesser', None)
        self.input_level = Gst.ElementFactory.make('level',
                                                   'deesser_input_level')
        self.output_level = Gst.ElementFactory.make('level',
                                                    'deesser_output_level')

        self.bin = GstInsertBin.InsertBin.new('deesser_bin')

        if self.is_installed:
            # booleans are inverted in GStreamer versions older than 1.12.5

            registry = Gst.Registry().get()
            self.use_workaround = not registry\
                .check_feature_version('pulsesrc', 1, 12, 5)

            if self.use_workaround:
                self.deesser.set_property('bypass', True)
                self.deesser.set_property('sc-listen', True)
            else:
                self.deesser.set_property('bypass', False)
                self.deesser.set_property('sc-listen', False)

            self.bin.append(self.input_level, self.on_filter_added, None)
            self.bin.append(self.deesser, self.on_filter_added, None)
            self.bin.append(self.output_level, self.on_filter_added, None)

    def post_messages(self, state):
        self.input_level.set_property('post-messages', state)
        self.output_level.set_property('post-messages', state)

    def init_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/deesser.glade')
        self.builder.connect_signals(self)

        self.ui_window = self.builder.get_object('window')
        self.ui_controls = self.builder.get_object('controls')
        self.ui_listbox_control = self.builder.get_object('listbox_control')

        self.ui_enable = self.builder.get_object('enable')
        self.ui_img_state = self.builder.get_object('img_state')
        self.ui_detection_rms = self.builder.get_object('detection_rms')
        self.ui_detection_peak = self.builder.get_object('detection_peak')
        self.ui_mode_wide = self.builder.get_object('mode_wide')
        self.ui_mode_split = self.builder.get_object('mode_split')
        self.ui_laxity = self.builder.get_object('laxity')
        self.ui_threshold = self.builder.get_object('threshold')
        self.ui_ratio = self.builder.get_object('ratio')
        self.ui_makeup = self.builder.get_object('makeup')
        self.ui_f1_freq = self.builder.get_object('f1_freq')
        self.ui_f1_level = self.builder.get_object('f1_level')
        self.ui_f2_freq = self.builder.get_object('f2_freq')
        self.ui_f2_level = self.builder.get_object('f2_level')
        self.ui_f2_q = self.builder.get_object('f2_q')

        self.ui_input_level_left = self.builder.get_object('input_level_left')
        self.ui_input_level_right = self.builder.get_object(
            'input_level_right')
        self.ui_output_level_left = self.builder.get_object(
            'output_level_left')
        self.ui_output_level_right = self.builder.get_object(
            'output_level_right')
        self.ui_compression_levelbar = self.builder.get_object(
            'compression_levelbar')
        self.ui_detected_levelbar = self.builder.get_object(
            'detected_levelbar')

        self.ui_input_level_left_label = self.builder.get_object(
            'input_level_left_label')
        self.ui_input_level_right_label = self.builder.get_object(
            'input_level_right_label')
        self.ui_output_level_left_label = self.builder.get_object(
            'output_level_left_label')
        self.ui_output_level_right_label = self.builder.get_object(
            'output_level_right_label')
        self.ui_compression_level_label = self.builder.get_object(
            'compression_level_label')
        self.ui_detected_level_label = self.builder.get_object(
            'detected_level_label')

    def bind(self):
        # binding ui widgets to gsettings

        flag = Gio.SettingsBindFlags.DEFAULT

        self.settings.bind('state', self.ui_enable, 'active', flag)
        self.settings.bind('state', self.ui_img_state, 'visible', flag)
        self.settings.bind('state', self.ui_controls, 'sensitive',
                           Gio.SettingsBindFlags.GET)

        self.settings.bind('detection-rms', self.ui_detection_rms, 'active',
                           flag)
        self.settings.bind('detection-rms', self.ui_detection_peak,
                           'active',
                           flag | Gio.SettingsBindFlags.INVERT_BOOLEAN)

        self.settings.bind('mode-wide', self.ui_mode_wide, 'active', flag)
        self.settings.bind('mode-wide', self.ui_mode_split, 'active',
                           flag | Gio.SettingsBindFlags.INVERT_BOOLEAN)

        self.settings.bind('laxity', self.ui_laxity, 'value', flag)
        self.settings.bind('threshold', self.ui_threshold, 'value', flag)
        self.settings.bind('makeup', self.ui_makeup, 'value', flag)
        self.settings.bind('ratio', self.ui_ratio, 'value', flag)
        self.settings.bind('f1', self.ui_f1_freq, 'value', flag)
        self.settings.bind('f1-level', self.ui_f1_level, 'value', flag)
        self.settings.bind('f2', self.ui_f2_freq, 'value', flag)
        self.settings.bind('f2-level', self.ui_f2_level, 'value', flag)
        self.settings.bind('f2-q', self.ui_f2_q, 'value', flag)

        # binding ui widgets to gstreamer plugins

        flag = GObject.BindingFlags.BIDIRECTIONAL | \
            GObject.BindingFlags.SYNC_CREATE

        self.ui_ratio.bind_property('value', self.deesser, 'ratio', flag)
        self.ui_laxity.bind_property('value', self.deesser, 'laxity', flag)
        self.ui_f1_freq.bind_property('value', self.deesser, 'f1-freq', flag)
        self.ui_f2_freq.bind_property('value', self.deesser, 'f2-freq', flag)
        self.ui_f2_q.bind_property('value', self.deesser, 'f2-q', flag)

    def on_threshold_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20.0)

        self.deesser.set_property('threshold', value_linear)

    def on_makeup_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20.0)

        self.deesser.set_property('makeup', value_linear)

    def on_f1_level_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20.0)

        self.deesser.set_property('f1-level', value_linear)

    def on_f2_level_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20.0)

        self.deesser.set_property('f2-level', value_linear)

    def on_new_detection_type(self, obj):
        if obj.get_active():
            label = obj.get_label()

            if label == 'rms':
                self.deesser.set_property('detection', 'RMS')
            elif label == 'peak':
                self.deesser.set_property('detection', 'Peak')

    def on_new_mode(self, obj):
        if obj.get_active():
            label = obj.get_label()

            if label == 'wide':
                self.deesser.set_property('mode', 'Wide')
            elif label == 'split':
                self.deesser.set_property('mode', 'Split')

    def ui_update_level(self, widgets, peak):
        left, right = peak[0], peak[1]

        widget_level_left = widgets[0]
        widget_level_right = widgets[1]
        widget_level_left_label = widgets[2]
        widget_level_right_label = widgets[3]

        if left >= -99:
            l_value = 10**(left / 10)
            widget_level_left.set_value(l_value)
            widget_level_left_label.set_text(str(round(left)))
        else:
            widget_level_left.set_value(0)
            widget_level_left_label.set_text('-99')

        if right >= -99:
            r_value = 10**(right / 10)
            widget_level_right.set_value(r_value)
            widget_level_right_label.set_text(str(round(right)))
        else:
            widget_level_right.set_value(0)
            widget_level_right_label.set_text('-99')

    def ui_update_input_level(self, peak):
        widgets = [self.ui_input_level_left, self.ui_input_level_right,
                   self.ui_input_level_left_label,
                   self.ui_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_output_level(self, peak):
        widgets = [self.ui_output_level_left, self.ui_output_level_right,
                   self.ui_output_level_left_label,
                   self.ui_output_level_right_label]

        self.ui_update_level(widgets, peak)

        compression = self.deesser.get_property('compression')
        detected = self.deesser.get_property('detected')

        self.ui_compression_levelbar.set_value(1 - compression)
        self.ui_detected_levelbar.set_value(detected)

        if compression > 0:
            compression = 20 * np.log10(compression)

            self.ui_compression_level_label.set_text(str(round(compression)))

        if detected > 0:
            detected = 20 * np.log10(detected)

            self.ui_detected_level_label.set_text(str(round(detected)))

    def reset(self):
        self.settings.reset('state')
        self.settings.reset('detection-rms')
        self.settings.reset('mode-wide')
        self.settings.reset('threshold')
        self.settings.reset('makeup')
        self.settings.reset('ratio')
        self.settings.reset('f1')
        self.settings.reset('f1-level')
        self.settings.reset('f2')
        self.settings.reset('f2-level')
        self.settings.reset('f2-q')
