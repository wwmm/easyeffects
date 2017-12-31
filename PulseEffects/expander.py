# -*- coding: utf-8 -*-

import logging
import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GObject, Gst, GstInsertBin, Gtk

Gst.init(None)


class Expander():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.log = logging.getLogger('PulseEffects')

        self.old_expander_gain_expansion = 0

        if Gst.ElementFactory.make('ladspa-se4-1883-so-se4'):
            self.is_installed = True
        else:
            self.is_installed = False

            self.log.warn('Expander plugin was not found. Disabling it!')

        self.build_bin()

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_bin(self):
        self.expander = Gst.ElementFactory.make(
            'ladspa-se4-1883-so-se4', None)
        self.input_level = Gst.ElementFactory.make('level',
                                                   'expander_input_level')
        self.output_level = Gst.ElementFactory.make('level',
                                                    'expander_output_level')

        self.bin = GstInsertBin.InsertBin.new('expander_bin')

        if self.is_installed:
            self.bin.append(self.input_level, self.on_filter_added, None)
            self.bin.append(self.expander, self.on_filter_added, None)
            self.bin.append(self.output_level, self.on_filter_added, None)

    def post_messages(self, state):
        self.input_level.set_property('post-messages', state)
        self.output_level.set_property('post-messages', state)

    def init_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/expander.glade')
        self.builder.connect_signals(self)

        self.ui_window = self.builder.get_object('window')
        self.ui_controls = self.builder.get_object('controls')
        self.ui_listbox_control = self.builder.get_object('listbox_control')

        self.ui_enable = self.builder.get_object('enable')
        self.ui_img_state = self.builder.get_object('img_state')
        self.ui_rms = self.builder.get_object('rms')
        self.ui_peak = self.builder.get_object('peak')
        self.ui_attack = self.builder.get_object('attack')
        self.ui_release = self.builder.get_object('release')
        self.ui_threshold = self.builder.get_object('threshold')
        self.ui_ratio = self.builder.get_object('ratio')
        self.ui_knee = self.builder.get_object('knee')
        self.ui_attenuation = self.builder.get_object('attenuation')

        self.ui_input_level_left = self.builder.get_object('input_level_left')
        self.ui_input_level_right = self.builder.get_object(
            'input_level_right')
        self.ui_output_level_left = self.builder.get_object(
            'output_level_left')
        self.ui_output_level_right = self.builder.get_object(
            'output_level_right')
        self.ui_gain_expansion_levelbar = self.builder.get_object(
            'gain_expansion_levelbar')

        self.ui_input_level_left_label = self.builder.get_object(
            'input_level_left_label')
        self.ui_input_level_right_label = self.builder.get_object(
            'input_level_right_label')
        self.ui_output_level_left_label = self.builder.get_object(
            'output_level_left_label')
        self.ui_output_level_right_label = self.builder.get_object(
            'output_level_right_label')
        self.ui_gain_expansion_level_label = self.builder.get_object(
            'gain_expansion_level_label')

        self.ui_gain_expansion_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_LOW', 6)
        self.ui_gain_expansion_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_HIGH', 18)
        self.ui_gain_expansion_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_FULL', 24)

    def bind(self):
        # binding ui widgets to gsettings

        flag = Gio.SettingsBindFlags.DEFAULT

        self.settings.bind('expander-state', self.ui_enable, 'active', flag)
        self.settings.bind('expander-state', self.ui_img_state, 'visible',
                           flag)
        self.settings.bind('expander-state', self.ui_controls, 'sensitive',
                           Gio.SettingsBindFlags.GET)

        self.settings.bind('expander-use-peak', self.ui_peak, 'active', flag)
        self.settings.bind('expander-use-peak', self.ui_rms, 'active',
                           flag | Gio.SettingsBindFlags.INVERT_BOOLEAN)

        self.settings.bind('expander-attack', self.ui_attack, 'value', flag)
        self.settings.bind('expander-release', self.ui_release, 'value', flag)
        self.settings.bind('expander-threshold', self.ui_threshold, 'value',
                           flag)
        self.settings.bind('expander-ratio', self.ui_ratio, 'value', flag)
        self.settings.bind('expander-knee', self.ui_knee, 'value', flag)
        self.settings.bind('expander-attenuation', self.ui_attenuation,
                           'value', flag)

        # binding ui widgets to gstreamer plugins

        flag = GObject.BindingFlags.BIDIRECTIONAL | \
            GObject.BindingFlags.SYNC_CREATE

        self.ui_attack.bind_property('value', self.expander, 'attack-time',
                                     flag)
        self.ui_release.bind_property('value', self.expander, 'release-time',
                                      flag)
        self.ui_threshold.bind_property('value', self.expander,
                                        'threshold-level', flag)
        self.ui_ratio.bind_property('value', self.expander, 'ratio', flag)
        self.ui_knee.bind_property('value', self.expander, 'knee-radius',
                                   flag)
        self.ui_attenuation.bind_property('value', self.expander,
                                          'attenuation', flag)

    def on_expander_measurement_type(self, obj):
        if obj.get_active():
            label = obj.get_label()

            if label == 'rms':
                self.expander.set_property('rms-peak', 0.0)
            elif label == 'peak':
                self.expander.set_property('rms-peak', 1.0)

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

        gain_expansion = abs(round(
            self.expander.get_property('gain-expansion')))

        if gain_expansion != self.old_expander_gain_expansion:
            self.old_expander_gain_expansion = gain_expansion

            self.ui_gain_expansion_levelbar.set_value(gain_expansion)
            self.ui_gain_expansion_level_label.set_text(
                str(round(gain_expansion)))

    def reset(self):
        self.settings.reset('expander-state')
        self.settings.reset('expander-use-peak')
        self.settings.reset('expander-attack')
        self.settings.reset('expander-release')
        self.settings.reset('expander-threshold')
        self.settings.reset('expander-ratio')
        self.settings.reset('expander-knee')
        self.settings.reset('expander-attenuation')
