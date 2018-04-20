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


class OutputLimiter():

    def __init__(self):
        self.settings = None
        self.module_path = os.path.dirname(__file__)

        self.log = logging.getLogger('PulseEffects')

        if Gst.ElementFactory.make(
                'calf-sourceforge-net-plugins-Limiter'):
            self.is_installed = True
        else:
            self.is_installed = False

            self.log.warn('Limiter plugin was not found. Disabling it!')

        self.build_bin()

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_bin(self):
        self.limiter = Gst.ElementFactory.make(
            'calf-sourceforge-net-plugins-Limiter', None)
        self.in_level = Gst.ElementFactory.make('level',
                                                'output_limiter_input_level')
        self.out_level = Gst.ElementFactory.make('level',
                                                 'output_limiter_output_level')

        self.bin = GstInsertBin.InsertBin.new('output_limiter_bin')

        if self.is_installed:
            # booleans are inverted in GStreamer versions older than 1.12.5

            registry = Gst.Registry().get()
            self.use_workaround = not registry\
                .check_feature_version('pulsesrc', 1, 12, 5)

            if self.use_workaround:
                self.comprelimiterssor.set_property('bypass', True)
            else:
                self.limiter.set_property('bypass', False)

            self.bin.append(self.in_level, self.on_filter_added, None)
            self.bin.append(self.limiter, self.on_filter_added, None)
            self.bin.append(self.out_level, self.on_filter_added, None)

    def post_messages(self, state):
        self.in_level.set_property('post-messages', state)
        self.out_level.set_property('post-messages', state)

    def init_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/output_limiter.glade')
        self.builder.connect_signals(self)

        self.ui_window = self.builder.get_object('window')
        self.ui_controls = self.builder.get_object('controls')
        self.ui_listbox_control = self.builder.get_object('listbox_control')

        self.ui_limiter_enable = self.builder.get_object('limiter_enable')
        self.ui_img_state = self.builder.get_object('img_state')
        self.ui_input_gain = self.builder.get_object('input_gain')
        self.ui_limit = self.builder.get_object('limit')
        self.ui_lookahead = self.builder.get_object('lookahead')
        self.ui_release = self.builder.get_object('release')
        self.ui_oversampling = self.builder.get_object('oversampling')
        self.ui_asc = self.builder.get_object('asc')
        self.ui_asc_level = self.builder.get_object('asc_level')
        self.ui_attenuation_levelbar = self.builder.get_object(
            'attenuation_levelbar')

        self.ui_input_level_left = self.builder.get_object('input_level_left')
        self.ui_input_level_right = self.builder.get_object(
            'input_level_right')
        self.ui_output_level_left = self.builder.get_object(
            'output_level_left')
        self.ui_output_level_right = self.builder.get_object(
            'output_level_right')

        self.ui_input_level_left_label = self.builder.get_object(
            'input_level_left_label')
        self.ui_input_level_right_label = self.builder.get_object(
            'input_level_right_label')
        self.ui_output_level_left_label = self.builder.get_object(
            'output_level_left_label')
        self.ui_output_level_right_label = self.builder.get_object(
            'output_level_right_label')
        self.ui_attenuation_level_label = self.builder.get_object(
            'attenuation_level_label')

    def bind(self):
        # binding ui widgets to gstreamer plugins

        flag = Gio.SettingsBindFlags.DEFAULT

        self.settings.bind('state', self.ui_limiter_enable, 'active', flag)
        self.settings.bind('state', self.ui_img_state, 'visible', flag)
        self.settings.bind('state', self.ui_controls, 'sensitive',
                           Gio.SettingsBindFlags.GET)
        self.settings.bind('input-gain', self.ui_input_gain, 'value', flag)
        self.settings.bind('limit', self.ui_limit, 'value', flag)
        self.settings.bind('lookahead', self.ui_lookahead, 'value', flag)
        self.settings.bind('release', self.ui_release, 'value', flag)
        self.settings.bind('oversampling', self.ui_oversampling, 'value', flag)
        self.settings.bind('asc', self.ui_asc, 'active', flag)
        self.settings.bind('asc-level', self.ui_asc_level, 'value', flag)

        # binding ui widgets to gstreamer plugins

        flag = GObject.BindingFlags.BIDIRECTIONAL | \
            GObject.BindingFlags.SYNC_CREATE

        self.ui_asc.bind_property('active', self.limiter, 'asc', flag)
        self.ui_asc_level.bind_property('value', self.limiter, 'asc-coeff',
                                        flag)
        self.ui_lookahead.bind_property('value', self.limiter, 'attack', flag)
        self.ui_release.bind_property('value', self.limiter, 'release', flag)
        self.ui_oversampling.bind_property('value', self.limiter,
                                           'oversampling', flag)

    def on_input_gain_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20.0)

        self.limiter.set_property('level-in', value_linear)

    def on_limit_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20.0)

        self.limiter.set_property('limit', value_linear)

        # calf limiter does automatic makeup gain by the same amount given as
        # limit. See https://github.com/calf-studio-gear/calf/issues/162
        # that is why we reduce the output level accordingly

        self.limiter.set_property('level-out', value_linear)

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

    def ui_update_limiter_input_level(self, peak):
        widgets = [self.ui_input_level_left, self.ui_input_level_right,
                   self.ui_input_level_left_label,
                   self.ui_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_limiter_output_level(self, peak):
        widgets = [self.ui_output_level_left, self.ui_output_level_right,
                   self.ui_output_level_left_label,
                   self.ui_output_level_right_label]

        self.ui_update_level(widgets, peak)

        attenuation = self.limiter.get_property('att')

        self.ui_attenuation_levelbar.set_value(1 - attenuation)

        if attenuation > 0:
            attenuation = 20 * np.log10(attenuation)

            self.ui_attenuation_level_label.set_text(
                str(round(attenuation, 1)))

    def reset(self):
        self.settings.reset('state')
        self.settings.reset('input-gain')
        self.settings.reset('limit')
        self.settings.reset('lookahead')
        self.settings.reset('release')
        self.settings.reset('oversampling')
        self.settings.reset('asc')
        self.settings.reset('asc-level')
