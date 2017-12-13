# -*- coding: utf-8 -*-

import logging
import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GObject, Gst, GstInsertBin, Gtk

Gst.init(None)


class Limiter():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.log = logging.getLogger('PulseEffects')

        self.old_limiter_attenuation = 0

        self.autovolume_target = -12  # dB
        self.autovolume_tolerance = 1  # dB
        self.autovolume_threshold = -50  # autovolume only if avg > threshold

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
        self.input_level = Gst.ElementFactory.make('level',
                                                   'limiter_input_level')
        self.output_level = Gst.ElementFactory.make('level',
                                                    'limiter_output_level')
        self.autovolume_level = Gst.ElementFactory.make('level', 'autovolume')

        self.bin = GstInsertBin.InsertBin.new('limiter_bin')

        if self.is_installed:
            self.bin.append(self.input_level, self.on_filter_added, None)
            self.bin.append(self.limiter, self.on_filter_added, None)
            self.bin.append(self.output_level, self.on_filter_added, None)
            self.bin.append(self.autovolume_level, self.on_filter_added, None)

    def post_messages(self, state):
        self.input_level.set_property('post-messages', state)
        self.output_level.set_property('post-messages', state)

    def init_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/limiter.glade')
        self.builder.connect_signals(self)

        self.ui_window = self.builder.get_object('window')
        self.ui_controls = self.builder.get_object('controls')
        self.ui_listbox_control = self.builder.get_object('listbox_control')
        self.ui_limiter_controls = self.builder.get_object('limiter_controls')

        self.ui_autovolume_box = self.builder.get_object('autovolume_box')
        self.ui_autovolume_controls = self.builder.get_object(
            'autovolume_controls')

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

        self.ui_autovolume_enable = self.builder.get_object(
            'autovolume_enable')
        self.ui_autovolume_window = self.builder.get_object(
            'autovolume_window')
        self.ui_autovolume_target = self.builder.get_object(
            'autovolume_target')
        self.ui_autovolume_tolerance = self.builder.get_object(
            'autovolume_tolerance')
        self.ui_autovolume_threshold = self.builder.get_object(
            'autovolume_threshold')

    def bind(self):
        # binding ui widgets to gstreamer plugins

        flag = GObject.BindingFlags.BIDIRECTIONAL

        self.ui_input_gain.bind_property('value', self.limiter, 'input-gain',
                                         flag)
        self.ui_limit.bind_property('value', self.limiter, 'limit', flag)
        self.ui_release_time.bind_property('value', self.limiter,
                                           'release-time', flag)

        # binding ui widgets to gsettings

        flag = Gio.SettingsBindFlags.DEFAULT

        self.settings.bind('limiter-state', self.ui_limiter_enable, 'active',
                           flag)
        self.settings.bind('limiter-state', self.ui_img_state, 'visible', flag)
        self.settings.bind('limiter-state', self.ui_controls,
                           'sensitive', Gio.SettingsBindFlags.GET)
        self.settings.bind('limiter-state', self.ui_autovolume_box,
                           'sensitive', Gio.SettingsBindFlags.GET)
        self.settings.bind('limiter-input-gain', self.ui_input_gain, 'value',
                           flag)
        self.settings.bind('limiter-limit', self.ui_limit, 'value', flag)
        self.settings.bind('limiter-release-time', self.ui_release_time,
                           'value', flag)

        self.settings.bind('autovolume-state', self.ui_autovolume_enable,
                           'active', flag)
        self.settings.bind('autovolume-state', self.autovolume_level,
                           'post-messages', flag)
        self.settings.bind('autovolume-state', self.ui_autovolume_controls,
                           'sensitive', Gio.SettingsBindFlags.GET)

        self.settings.bind('autovolume-window', self.ui_autovolume_window,
                           'value', flag)
        self.settings.bind('autovolume-target', self.ui_autovolume_target,
                           'value', flag)
        self.settings.bind('autovolume-tolerance',
                           self.ui_autovolume_tolerance, 'value', flag)
        self.settings.bind('autovolume-threshold',
                           self.ui_autovolume_threshold, 'value', flag)

    def on_limiter_enable_state_set(self, obj, state):
        autovolume_enabled = self.settings.get_value(
            'autovolume-state').unpack()

        if state and autovolume_enabled:
            self.ui_limiter_controls.set_sensitive(False)
        elif state and not autovolume_enabled:
            self.ui_limiter_controls.set_sensitive(True)
        else:
            self.ui_limiter_controls.set_sensitive(False)

    def enable_autovolume(self, state):
        if state:
            window = self.settings.get_value('autovolume-window').unpack()
            target = self.settings.get_value('autovolume-target').unpack()
            tolerance = self.settings.get_value(
                'autovolume-tolerance').unpack()

            self.ui_input_gain.set_value(-10)
            self.ui_limit.set_value(target + tolerance)
            self.ui_release_time.set_value(window)

            self.ui_limiter_controls.set_sensitive(False)
        else:
            self.ui_input_gain.set_value(-10)
            self.ui_limit.set_value(0)
            self.ui_release_time.set_value(1.0)

            self.ui_limiter_controls.set_sensitive(True)

    def on_autovolume_enable_state_set(self, obj, state):
        self.enable_autovolume(state)

    def on_autovolume_window_value_changed(self, obj):
        value = obj.get_value()

        # value must be in seconds
        self.autovolume_level.set_property('interval', int(value * 1000000000))

        self.ui_release_time.set_value(value)

    def on_autovolume_target_value_changed(self, obj):
        value = obj.get_value()

        self.autovolume_target = value

        tolerance = self.settings.get_value('autovolume-tolerance').unpack()

        self.ui_limit.set_value(value + tolerance)

    def on_autovolume_tolerance_value_changed(self, obj):
        value = obj.get_value()

        self.autovolume_tolerance = value

        target = self.settings.get_value('autovolume-target').unpack()

        self.ui_limit.set_value(target + value)

    def on_autovolume_threshold_value_changed(self, obj):
        self.autovolume_threshold = obj.get_value()

    def auto_gain(self, max_value):
        max_value = int(max_value)

        if max_value > self.autovolume_target + self.autovolume_tolerance:
            gain = self.limiter.get_property('input-gain')

            if gain - 1 >= -20:
                gain = gain - 1

                # using ui_input_gain has no effect in service mode because
                # the ui is destroyed

                self.limiter.set_property('input-gain', gain)
        elif max_value < self.autovolume_target - self.autovolume_tolerance:
            gain = self.limiter.get_property('input-gain')

            if gain + 1 <= 20:
                gain = gain + 1

                self.limiter.set_property('input-gain', gain)

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

        attenuation = round(self.limiter.get_property('attenuation'))

        if attenuation != self.old_limiter_attenuation:
            self.old_limiter_attenuation = attenuation

            self.ui_attenuation_levelbar.set_value(attenuation)
            self.ui_attenuation_level_label.set_text(str(round(attenuation)))

    def reset(self):
        self.settings.reset('limiter-state')
        self.settings.reset('limiter-input-gain')
        self.settings.reset('limiter-limit')
        self.settings.reset('limiter-release-time')
        self.settings.reset('autovolume-state')
        self.settings.reset('autovolume-window')
        self.settings.reset('autovolume-target')
        self.settings.reset('autovolume-tolerance')
        self.settings.reset('autovolume-threshold')
