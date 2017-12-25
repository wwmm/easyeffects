# -*- coding: utf-8 -*-

import logging
import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GObject, Gst, GstInsertBin, Gtk

Gst.init(None)


class Compressor():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.log = logging.getLogger('PulseEffects')

        self.old_compressor_gain_reduction = 0

        if Gst.ElementFactory.make('ladspa-sc4-1882-so-sc4'):
            self.is_installed = True
        else:
            self.is_installed = False

            self.log.warn('Compressor plugin was not found. Disabling it!')

        self.build_bin()

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_bin(self):
        self.compressor = Gst.ElementFactory.make(
            'ladspa-sc4-1882-so-sc4', None)
        self.input_level = Gst.ElementFactory.make('level',
                                                   'compressor_input_level')
        self.output_level = Gst.ElementFactory.make('level',
                                                    'compressor_output_level')

        self.bin = GstInsertBin.InsertBin.new('compressor_bin')

        if self.is_installed:
            self.bin.append(self.input_level, self.on_filter_added, None)
            self.bin.append(self.compressor, self.on_filter_added, None)
            self.bin.append(self.output_level, self.on_filter_added, None)

    def post_messages(self, state):
        self.input_level.set_property('post-messages', state)
        self.output_level.set_property('post-messages', state)

    def init_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/compressor.glade')
        self.builder.connect_signals(self)

        self.ui_window = self.builder.get_object('window')
        self.ui_controls = self.builder.get_object('controls')
        self.ui_listbox_control = self.builder.get_object('listbox_control')

        self.ui_enable = self.builder.get_object('enable')
        self.ui_img_state = self.builder.get_object('img_state')
        self.ui_compressor_rms = self.builder.get_object('compressor_rms')
        self.ui_compressor_peak = self.builder.get_object('compressor_peak')
        self.ui_attack = self.builder.get_object('attack')
        self.ui_release = self.builder.get_object('release')
        self.ui_threshold = self.builder.get_object('threshold')
        self.ui_ratio = self.builder.get_object('ratio')
        self.ui_knee = self.builder.get_object('knee')
        self.ui_makeup = self.builder.get_object('makeup')

        self.ui_input_level_left = self.builder.get_object('input_level_left')
        self.ui_input_level_right = self.builder.get_object(
            'input_level_right')
        self.ui_output_level_left = self.builder.get_object(
            'output_level_left')
        self.ui_output_level_right = self.builder.get_object(
            'output_level_right')
        self.ui_gain_reduction_levelbar = self.builder.get_object(
            'gain_reduction_levelbar')

        self.ui_input_level_left_label = self.builder.get_object(
            'input_level_left_label')
        self.ui_input_level_right_label = self.builder.get_object(
            'input_level_right_label')
        self.ui_output_level_left_label = self.builder.get_object(
            'output_level_left_label')
        self.ui_output_level_right_label = self.builder.get_object(
            'output_level_right_label')
        self.ui_gain_reduction_level_label = self.builder.get_object(
            'gain_reduction_level_label')

        self.ui_gain_reduction_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_LOW', 6)
        self.ui_gain_reduction_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_HIGH', 18)
        self.ui_gain_reduction_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_FULL', 24)

    def bind(self):
        # binding ui widgets to gstreamer plugins

        flag = GObject.BindingFlags.BIDIRECTIONAL | \
            GObject.BindingFlags.SYNC_CREATE

        self.ui_attack.bind_property('value', self.compressor, 'attack-time',
                                     flag)
        self.ui_release.bind_property('value', self.compressor, 'release-time',
                                      flag)
        self.ui_threshold.bind_property('value', self.compressor,
                                        'threshold-level', flag)
        self.ui_ratio.bind_property('value', self.compressor, 'ratio', flag)
        self.ui_knee.bind_property('value', self.compressor, 'knee-radius',
                                   flag)
        self.ui_makeup.bind_property('value', self.compressor, 'makeup-gain',
                                     flag)

        # binding ui widgets to gsettings

        flag = Gio.SettingsBindFlags.DEFAULT

        self.settings.bind('compressor-state', self.ui_enable, 'active', flag)
        self.settings.bind('compressor-state', self.ui_img_state, 'visible',
                           flag)
        self.settings.bind('compressor-state', self.ui_controls, 'sensitive',
                           Gio.SettingsBindFlags.GET)

        self.settings.bind('compressor-use-peak', self.ui_compressor_peak,
                           'active', flag)
        self.settings.bind('compressor-use-peak', self.ui_compressor_rms,
                           'active',
                           flag | Gio.SettingsBindFlags.INVERT_BOOLEAN)

        self.settings.bind('compressor-attack', self.ui_attack, 'value', flag)
        self.settings.bind('compressor-release', self.ui_release, 'value',
                           flag)
        self.settings.bind('compressor-threshold', self.ui_threshold, 'value',
                           flag)
        self.settings.bind('compressor-ratio', self.ui_ratio, 'value', flag)
        self.settings.bind('compressor-knee', self.ui_knee, 'value', flag)
        self.settings.bind('compressor-makeup', self.ui_makeup, 'value', flag)

    def apply_compressor_preset(self, values):
        if values[0] == 0:
            self.ui_compressor_rms.set_active(True)
        elif values[0] == 1:
            self.ui_compressor_peak.set_active(True)

        self.ui_attack.set_value(values[1])
        self.ui_release.set_value(values[2])
        self.ui_threshold.set_value(values[3])
        self.ui_ratio.set_value(values[4])
        self.ui_knee.set_value(values[5])
        self.ui_makeup.set_value(values[6])

    def on_compressor_measurement_type(self, obj):
        if obj.get_active():
            label = obj.get_label()

            if label == 'rms':
                self.compressor.set_property('rms-peak', 0.0)
            elif label == 'peak':
                self.compressor.set_property('rms-peak', 1.0)

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

    def ui_update_compressor_input_level(self, peak):
        widgets = [self.ui_input_level_left, self.ui_input_level_right,
                   self.ui_input_level_left_label,
                   self.ui_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_compressor_output_level(self, peak):
        widgets = [self.ui_output_level_left, self.ui_output_level_right,
                   self.ui_output_level_left_label,
                   self.ui_output_level_right_label]

        self.ui_update_level(widgets, peak)

        gain_reduction = abs(round(
            self.compressor.get_property('gain-reduction')))

        if gain_reduction != self.old_compressor_gain_reduction:
            self.old_compressor_gain_reduction = gain_reduction

            self.ui_gain_reduction_levelbar.set_value(gain_reduction)
            self.ui_gain_reduction_level_label.set_text(
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
