# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gst, GstInsertBin, Gtk

Gst.init(None)


class Limiter():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.old_limiter_attenuation = 0

        self.autovolume_enabled = False
        self.autovolume_target = -12  # dB
        self.autovolume_tolerance = 1  # dB
        self.autovolume_threshold = -50  # autovolume only if avg > threshold

        self.builder = Gtk.Builder()

        self.builder.add_from_file(self.module_path + '/ui/limiter.glade')

        self.build_limiter_bin()

        self.load_ui()

        self.builder.connect_signals(self)

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_limiter_bin(self):
        self.limiter = Gst.ElementFactory.make(
            'ladspa-fast-lookahead-limiter-1913-so-fastlookaheadlimiter', None)
        limiter_input_level = Gst.ElementFactory.make('level',
                                                      'limiter_input_level')
        limiter_output_level = Gst.ElementFactory.make('level',
                                                       'limiter_output_level')
        self.autovolume_level = Gst.ElementFactory.make('level', 'autovolume')

        self.bin = GstInsertBin.InsertBin.new('limiter_bin')
        self.bin.append(self.limiter, self.on_filter_added, None)
        self.bin.append(limiter_input_level, self.on_filter_added, None)
        self.bin.append(limiter_output_level, self.on_filter_added, None)
        self.bin.append(self.autovolume_level, self.on_filter_added, None)

    def load_ui(self):
        self.ui_window = self.builder.get_object('window')

        self.ui_limiter_input_gain = self.builder.get_object(
            'limiter_input_gain')
        self.ui_limiter_limit = self.builder.get_object(
            'limiter_limit')
        self.ui_limiter_release_time = self.builder.get_object(
            'limiter_release_time')
        self.ui_limiter_attenuation_levelbar = self.builder.get_object(
            'limiter_attenuation_levelbar')

        self.ui_limiter_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_LOW', 20)
        self.ui_limiter_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_HIGH', 50)
        self.ui_limiter_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_FULL', 70)

        self.ui_limiter_input_level_left = self.builder.get_object(
            'limiter_input_level_left')
        self.ui_limiter_input_level_right = self.builder.get_object(
            'limiter_input_level_right')
        self.ui_limiter_output_level_left = self.builder.get_object(
            'limiter_output_level_left')
        self.ui_limiter_output_level_right = self.builder.get_object(
            'limiter_output_level_right')

        self.ui_limiter_input_level_left_label = self.builder.get_object(
            'limiter_input_level_left_label')
        self.ui_limiter_input_level_right_label = self.builder.get_object(
            'limiter_input_level_right_label')
        self.ui_limiter_output_level_left_label = self.builder.get_object(
            'limiter_output_level_left_label')
        self.ui_limiter_output_level_right_label = self.builder.get_object(
            'limiter_output_level_right_label')
        self.ui_limiter_attenuation_level_label = self.builder.get_object(
            'limiter_attenuation_level_label')

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

    def init_ui(self):
        self.limiter_user = self.settings.get_value('limiter-user').unpack()
        self.apply_limiter_preset(self.limiter_user)

        self.init_autovolume_ui()

    def init_autovolume_ui(self):
        self.autovolume_enabled = self.settings.get_value(
            'autovolume-state').unpack()
        autovolume_window = self.settings.get_value(
            'autovolume-window').unpack()
        self.autovolume_target = self.settings.get_value(
            'autovolume-target').unpack()
        self.autovolume_tolerance = self.settings.get_value(
            'autovolume-tolerance').unpack()
        self.autovolume_threshold = self.settings.get_value(
            'autovolume-threshold').unpack()

        self.ui_autovolume_enable.set_state(self.autovolume_enabled)
        self.ui_autovolume_window.set_value(autovolume_window)
        self.ui_autovolume_target.set_value(self.autovolume_target)
        self.ui_autovolume_tolerance.set_value(self.autovolume_tolerance)
        self.ui_autovolume_threshold.set_value(self.autovolume_threshold)

        if self.autovolume_enabled:
            self.enable_autovolume(True)

    def apply_limiter_preset(self, values):
        self.ui_limiter_input_gain.set_value(values[0])
        self.ui_limiter_limit.set_value(values[1])
        self.ui_limiter_release_time.set_value(values[2])

        # we need this when on value changed is not called
        self.limiter.set_property('input-gain', values[0])
        self.limiter.set_property('limit', values[1])
        self.limiter.set_property('release-time', values[2])

    def save_limiter_user(self, idx, value):
        self.limiter_user[idx] = value

        out = GLib.Variant('ad', self.limiter_user)

        self.settings.set_value('limiter-user', out)

    def on_limiter_input_gain_value_changed(self, obj):
        value = obj.get_value()
        self.limiter.set_property('input-gain', value)
        self.save_limiter_user(0, value)

    def on_limiter_limit_value_changed(self, obj):
        value = obj.get_value()
        self.limiter.set_property('limit', value)
        self.save_limiter_user(1, value)

    def on_limiter_release_time_value_changed(self, obj):
        value = obj.get_value()
        self.limiter.set_property('release-time', value)
        self.save_limiter_user(2, value)

    def enable_autovolume(self, state):
        self.autovolume_enabled = state

        if state:
            window = self.settings.get_value('autovolume-window').unpack()
            target = self.settings.get_value('autovolume-target').unpack()
            tolerance = self.settings.get_value(
                'autovolume-tolerance').unpack()

            self.ui_limiter_input_gain.set_value(-10)
            self.ui_limiter_limit.set_value(target + tolerance)
            self.ui_limiter_release_time.set_value(window)

            self.ui_limiter_input_gain.set_sensitive(False)
            self.ui_limiter_limit.set_sensitive(False)
            self.ui_limiter_release_time.set_sensitive(False)
        else:
            self.ui_limiter_input_gain.set_value(-10)
            self.ui_limiter_limit.set_value(0)
            self.ui_limiter_release_time.set_value(1.0)

            self.ui_limiter_input_gain.set_sensitive(True)
            self.ui_limiter_limit.set_sensitive(True)
            self.ui_limiter_release_time.set_sensitive(True)

        out = GLib.Variant('b', state)
        self.settings.set_value('autovolume-state', out)

    def on_autovolume_enable_state_set(self, obj, state):
        self.enable_autovolume(state)

    def on_autovolume_window_value_changed(self, obj):
        value = obj.get_value()

        # value must be in seconds
        self.autovolume_level.set_property('interval', int(value * 1000000000))

        self.ui_limiter_release_time.set_value(value)

        out = GLib.Variant('d', value)
        self.settings.set_value('autovolume-window', out)

    def on_autovolume_target_value_changed(self, obj):
        value = obj.get_value()

        self.autovolume_target = value

        tolerance = self.settings.get_value('autovolume-tolerance').unpack()

        self.ui_limiter_limit.set_value(value + tolerance)

        out = GLib.Variant('i', value)
        self.settings.set_value('autovolume-target', out)

    def on_autovolume_tolerance_value_changed(self, obj):
        value = obj.get_value()

        self.autovolume_tolerance = value

        target = self.settings.get_value('autovolume-target').unpack()

        self.ui_limiter_limit.set_value(target + value)

        out = GLib.Variant('i', value)
        self.settings.set_value('autovolume-tolerance', out)

    def on_autovolume_threshold_value_changed(self, obj):
        value = obj.get_value()

        self.autovolume_threshold = value

        out = GLib.Variant('i', value)
        self.settings.set_value('autovolume-threshold', out)

    def auto_gain(self, max_value):
        max_value = int(max_value)

        if max_value > self.autovolume_target + self.autovolume_tolerance:
            gain = self.limiter.get_property('input-gain')

            if gain - 1 >= -20:
                gain = gain - 1

                self.ui_limiter_input_gain.set_value(gain)
        elif max_value < self.autovolume_target - self.autovolume_tolerance:
            gain = self.limiter.get_property('input-gain')

            if gain + 1 <= 20:
                gain = gain + 1

                self.ui_limiter_input_gain.set_value(gain)

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
        widgets = [self.ui_limiter_input_level_left,
                   self.ui_limiter_input_level_right,
                   self.ui_limiter_input_level_left_label,
                   self.ui_limiter_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_limiter_output_level(self, peak):
        widgets = [self.ui_limiter_output_level_left,
                   self.ui_limiter_output_level_right,
                   self.ui_limiter_output_level_left_label,
                   self.ui_limiter_output_level_right_label]

        self.ui_update_level(widgets, peak)

        attenuation = round(self.limiter.get_property('attenuation'))

        if attenuation != self.old_limiter_attenuation:
            self.old_limiter_attenuation = attenuation

            self.ui_limiter_attenuation_levelbar.set_value(attenuation)
            self.ui_limiter_attenuation_level_label.set_text(
                str(round(attenuation)))

    def reset(self):
        self.settings.reset('limiter-user')
        self.settings.reset('autovolume-state')
        self.settings.reset('autovolume-window')
        self.settings.reset('autovolume-target')
        self.settings.reset('autovolume-tolerance')
        self.settings.reset('autovolume-threshold')
