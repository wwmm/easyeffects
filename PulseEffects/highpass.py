# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gst, GstInsertBin, Gtk

Gst.init(None)


class Highpass():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.builder = Gtk.Builder()

        self.builder.add_from_file(self.module_path + '/ui/highpass.glade')

        self.build_highpass_bin()

        self.load_ui()

        self.builder.connect_signals(self)

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_highpass_bin(self):
        self.highpass = Gst.ElementFactory.make('audiocheblimit', None)
        highpass_input_level = Gst.ElementFactory.make('level',
                                                       'highpass_input_level')
        highpass_output_level = Gst.ElementFactory.make(
            'level', 'highpass_output_level')

        self.highpass.set_property('mode', 'high-pass')
        self.highpass.set_property('type', 1)
        self.highpass.set_property('ripple', 0)

        self.bin = GstInsertBin.InsertBin.new('highpass_bin')
        self.bin.append(self.highpass, self.on_filter_added, None)
        self.bin.append(highpass_input_level, self.on_filter_added, None)
        self.bin.append(highpass_output_level, self.on_filter_added, None)

    def load_ui(self):
        self.ui_window = self.builder.get_object('window')

        self.ui_highpass_controls = self.builder.get_object(
            'highpass_controls')

        self.ui_highpass_enable = self.builder.get_object('highpass_enable')
        self.ui_highpass_cutoff = self.builder.get_object('highpass_cutoff')
        self.ui_highpass_poles = self.builder.get_object('highpass_poles')

        self.ui_highpass_input_level_left = self.builder.get_object(
            'highpass_input_level_left')
        self.ui_highpass_input_level_right = self.builder.get_object(
            'highpass_input_level_right')
        self.ui_highpass_output_level_left = self.builder.get_object(
            'highpass_output_level_left')
        self.ui_highpass_output_level_right = self.builder.get_object(
            'highpass_output_level_right')

        self.ui_highpass_input_level_left_label = self.builder.get_object(
            'highpass_input_level_left_label')
        self.ui_highpass_input_level_right_label = self.builder.get_object(
            'highpass_input_level_right_label')
        self.ui_highpass_output_level_left_label = self.builder.get_object(
            'highpass_output_level_left_label')
        self.ui_highpass_output_level_right_label = self.builder.get_object(
            'highpass_output_level_right_label')

    def init_ui(self):
        enabled = self.settings.get_value('highpass-state').unpack()
        highpass_cutoff_user = self.settings.get_value(
            'highpass-cutoff').unpack()
        highpass_poles_user = self.settings.get_value(
            'highpass-poles').unpack()

        self.ui_highpass_enable.set_state(enabled)
        self.ui_highpass_cutoff.set_value(highpass_cutoff_user)
        self.ui_highpass_poles.set_value(highpass_poles_user)

        self.highpass.set_property('cutoff', highpass_cutoff_user)
        self.highpass.set_property('poles', highpass_poles_user)

    def on_highpass_enable_state_set(self, obj, state):
        self.ui_highpass_controls.set_sensitive(state)

        out = GLib.Variant('b', state)
        self.settings.set_value('highpass-state', out)

    def on_highpass_cutoff_value_changed(self, obj):
        value = obj.get_value()
        self.highpass.set_property('cutoff', value)

        out = GLib.Variant('i', value)

        self.settings.set_value('highpass-cutoff', out)

    def on_highpass_poles_value_changed(self, obj):
        value = obj.get_value()
        self.highpass.set_property('poles', value)

        out = GLib.Variant('i', value)

        self.settings.set_value('highpass-poles', out)

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

    def ui_update_highpass_input_level(self, peak):
        widgets = [self.ui_highpass_input_level_left,
                   self.ui_highpass_input_level_right,
                   self.ui_highpass_input_level_left_label,
                   self.ui_highpass_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_highpass_output_level(self, peak):
        widgets = [self.ui_highpass_output_level_left,
                   self.ui_highpass_output_level_right,
                   self.ui_highpass_output_level_left_label,
                   self.ui_highpass_output_level_right_label]

        self.ui_update_level(widgets, peak)

    def reset(self):
        self.settings.reset('highpass-state')
        self.settings.reset('highpass-cutoff')
        self.settings.reset('highpass-poles')
