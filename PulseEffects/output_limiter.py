# -*- coding: utf-8 -*-

import logging
import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GObject, Gst, GstInsertBin, Gtk

Gst.init(None)


class OutputLimiter():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.log = logging.getLogger('PulseEffects')

        self.old_limiter_attenuation = 0

        if Gst.ElementFactory.make(
                'ladspa-fast-lookahead-limiter-1913-so-fastlookaheadlimiter'):
            self.is_installed = True
        else:
            self.is_installed = False

            self.log.warn('Limiter plugin was not found. Disabling it!')

        self.build_bin()

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_bin(self):
        self.limiter = Gst.ElementFactory.make(
            'ladspa-fast-lookahead-limiter-1913-so-fastlookaheadlimiter', None)
        input_level = Gst.ElementFactory.make('level',
                                              'output_limiter_input_level')
        output_level = Gst.ElementFactory.make('level',
                                               'output_limiter_output_level')

        self.bin = GstInsertBin.InsertBin.new('output_limiter_bin')

        if self.is_installed:
            self.bin.append(input_level, self.on_filter_added, None)
            self.bin.append(self.limiter, self.on_filter_added, None)
            self.bin.append(output_level, self.on_filter_added, None)

    def init_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/output_limiter.glade')
        self.builder.connect_signals(self)

        self.ui_window = self.builder.get_object('window')
        self.ui_controls = self.builder.get_object('controls')
        self.ui_listbox_control = self.builder.get_object('listbox_control')
        self.ui_limiter_controls = self.builder.get_object('limiter_controls')

        self.ui_limiter_enable = self.builder.get_object('limiter_enable')
        self.ui_img_state = self.builder.get_object('img_state')
        self.ui_input_gain = self.builder.get_object('input_gain')
        self.ui_limit = self.builder.get_object('limit')
        self.ui_release_time = self.builder.get_object('release_time')
        self.ui_attenuation_levelbar = self.builder.get_object(
            'attenuation_levelbar')

        self.ui_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_LOW', 20)
        self.ui_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_HIGH', 50)
        self.ui_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_FULL', 70)

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

        flag = GObject.BindingFlags.DEFAULT

        self.ui_input_gain.bind_property('value', self.limiter, 'input-gain',
                                         flag)
        self.ui_limit.bind_property('value', self.limiter, 'limit', flag)
        self.ui_release_time.bind_property('value', self.limiter,
                                           'release-time', flag)

        # binding ui widgets to gstreamer plugins

        flag = Gio.SettingsBindFlags.DEFAULT

        self.settings.bind('output-limiter-state', self.ui_limiter_enable,
                           'active', flag)
        self.settings.bind('output-limiter-state', self.ui_img_state,
                           'visible', flag)
        self.settings.bind('output-limiter-state', self.ui_controls,
                           'sensitive', Gio.SettingsBindFlags.GET)
        self.settings.bind('output-limiter-input-gain', self.ui_input_gain,
                           'value', flag)
        self.settings.bind('output-limiter-limit', self.ui_limit, 'value',
                           flag)
        self.settings.bind('output-limiter-release-time', self.ui_release_time,
                           'value', flag)

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

        attenuation = round(self.limiter.get_property('attenuation'))

        if attenuation != self.old_limiter_attenuation:
            self.old_limiter_attenuation = attenuation

            self.ui_attenuation_levelbar.set_value(attenuation)
            self.ui_attenuation_level_label.set_text(str(round(attenuation)))

    def reset(self):
        self.settings.reset('output-limiter-state')
        self.settings.reset('output-limiter-input-gain')
        self.settings.reset('output-limiter-limit')
        self.settings.reset('output-limiter-release-time')
