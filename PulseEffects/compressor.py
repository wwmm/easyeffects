# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, Gst, GstInsertBin, Gtk

Gst.init(None)


class Compressor():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.old_compressor_gain_reduction = 0

        self.builder = Gtk.Builder()

        self.builder.add_from_file(self.module_path + '/ui/compressor.glade')

        self.build_compressor_bin()

        self.load_ui()

        self.builder.connect_signals(self)

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_compressor_bin(self):
        self.compressor = Gst.ElementFactory.make(
            'ladspa-sc4-1882-so-sc4', None)
        compressor_input_level = Gst.ElementFactory.make(
            'level', 'compressor_input_level')
        compressor_output_level = Gst.ElementFactory.make(
            'level', 'compressor_output_level')

        self.bin = GstInsertBin.InsertBin.new('compressor_bin')
        self.bin.append(self.compressor, self.on_filter_added, None)
        self.bin.append(compressor_input_level, self.on_filter_added, None)
        self.bin.append(compressor_output_level, self.on_filter_added, None)

    def load_ui(self):
        self.ui_window = self.builder.get_object('window')

        self.ui_compressor_controls = self.builder.get_object(
            'compressor_controls')

        self.ui_compressor_enable = self.builder.get_object(
            'compressor_enable')
        self.ui_compressor_rms = self.builder.get_object('compressor_rms')
        self.ui_compressor_peak = self.builder.get_object('compressor_peak')
        self.ui_compressor_attack = self.builder.get_object(
            'compressor_attack')
        self.ui_compressor_release = self.builder.get_object(
            'compressor_release')
        self.ui_compressor_threshold = self.builder.get_object(
            'compressor_threshold')
        self.ui_compressor_ratio = self.builder.get_object(
            'compressor_ratio')
        self.ui_compressor_knee = self.builder.get_object('compressor_knee')
        self.ui_compressor_makeup = self.builder.get_object(
            'compressor_makeup')

        self.ui_compressor_input_level_left = self.builder.get_object(
            'compressor_input_level_left')
        self.ui_compressor_input_level_right = self.builder.get_object(
            'compressor_input_level_right')
        self.ui_compressor_output_level_left = self.builder.get_object(
            'compressor_output_level_left')
        self.ui_compressor_output_level_right = self.builder.get_object(
            'compressor_output_level_right')
        self.ui_compressor_gain_reduction_levelbar = self.builder.get_object(
            'compressor_gain_reduction_levelbar')

        self.ui_compressor_input_level_left_label = self.builder.get_object(
            'compressor_input_level_left_label')
        self.ui_compressor_input_level_right_label = self.builder.get_object(
            'compressor_input_level_right_label')
        self.ui_compressor_output_level_left_label = self.builder.get_object(
            'compressor_output_level_left_label')
        self.ui_compressor_output_level_right_label = self.builder.get_object(
            'compressor_output_level_right_label')
        self.ui_compressor_gain_reduction_level_label = \
            self.builder.get_object('compressor_gain_reduction_level_label')

        self.ui_compressor_gain_reduction_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_LOW', 6)
        self.ui_compressor_gain_reduction_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_HIGH', 18)
        self.ui_compressor_gain_reduction_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_FULL', 24)

    def bind(self):
        self.settings.bind('compressor-state', self.ui_compressor_enable,
                           'active', Gio.SettingsBindFlags.DEFAULT)
        self.settings.bind('compressor-state', self.ui_compressor_controls,
                           'sensitive', Gio.SettingsBindFlags.GET)
        self.settings.bind('compressor-use-peak', self.ui_compressor_peak,
                           'active', Gio.SettingsBindFlags.DEFAULT)
        self.settings.bind('compressor-attack', self.ui_compressor_attack,
                           'value', Gio.SettingsBindFlags.DEFAULT)
        self.settings.bind('compressor-release', self.ui_compressor_release,
                           'value', Gio.SettingsBindFlags.DEFAULT)
        self.settings.bind('compressor-threshold',
                           self.ui_compressor_threshold, 'value',
                           Gio.SettingsBindFlags.DEFAULT)
        self.settings.bind('compressor-ratio', self.ui_compressor_ratio,
                           'value', Gio.SettingsBindFlags.DEFAULT)
        self.settings.bind('compressor-knee', self.ui_compressor_knee,
                           'value', Gio.SettingsBindFlags.DEFAULT)
        self.settings.bind('compressor-makeup', self.ui_compressor_makeup,
                           'value', Gio.SettingsBindFlags.DEFAULT)

    def apply_compressor_preset(self, values):
        if values[0] == 0:
            self.ui_compressor_rms.set_active(True)
        elif values[0] == 1:
            self.ui_compressor_peak.set_active(True)

        self.ui_compressor_attack.set_value(values[1])
        self.ui_compressor_release.set_value(values[2])
        self.ui_compressor_threshold.set_value(values[3])
        self.ui_compressor_ratio.set_value(values[4])
        self.ui_compressor_knee.set_value(values[5])
        self.ui_compressor_makeup.set_value(values[6])

    def on_compressor_measurement_type(self, obj):
        if obj.get_active():
            label = obj.get_label()

            if label == 'rms':
                self.compressor.set_property('rms-peak', False)
            elif label == 'peak':
                self.compressor.set_property('rms-peak', True)

    def on_compressor_attack_time_value_changed(self, obj):
        self.compressor.set_property('attack-time', obj.get_value())

    def on_compressor_release_time_value_changed(self, obj):
        self.compressor.set_property('release-time', obj.get_value())

    def on_compressor_threshold_value_changed(self, obj):
        self.compressor.set_property('threshold-level', obj.get_value())

    def on_compressor_ratio_value_changed(self, obj):
        self.compressor.set_property('ratio', obj.get_value())

    def on_compressor_knee_value_changed(self, obj):
        self.compressor.set_property('knee-radius', obj.get_value())

    def on_compressor_makeup_value_changed(self, obj):
        self.compressor.set_property('makeup-gain', obj.get_value())

    def on_compressor_preset_clicked(self, obj):
        obj_id = Gtk.Buildable.get_name(obj)

        if obj_id == 'vlc':
            value = self.settings.get_value('compressor-vlc')
            self.apply_compressor_preset(value)

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

    def ui_update_compressor_input_level(self, peak):
        widgets = [self.ui_compressor_input_level_left,
                   self.ui_compressor_input_level_right,
                   self.ui_compressor_input_level_left_label,
                   self.ui_compressor_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_compressor_output_level(self, peak):
        widgets = [self.ui_compressor_output_level_left,
                   self.ui_compressor_output_level_right,
                   self.ui_compressor_output_level_left_label,
                   self.ui_compressor_output_level_right_label]

        self.ui_update_level(widgets, peak)

        gain_reduction = abs(round(
            self.compressor.get_property('gain-reduction')))

        if gain_reduction != self.old_compressor_gain_reduction:
            self.old_compressor_gain_reduction = gain_reduction

            self.ui_compressor_gain_reduction_levelbar.set_value(
                gain_reduction)
            self.ui_compressor_gain_reduction_level_label.set_text(
                str(round(gain_reduction)))

    def reset(self):
        self.settings.reset('compressor-state')
        self.settings.reset('compressor-use-peak')
        self.settings.reset('compressor-attack')
        self.settings.reset('compressor-release')
        self.settings.reset('compressor-threshold')
        self.settings.reset('compressor-ratio')
        self.settings.reset('compressor-knee')
        self.settings.reset('compressor-makeup')
