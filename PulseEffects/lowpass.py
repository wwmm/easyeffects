# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gst, GstInsertBin, Gtk

Gst.init(None)


class Lowpass():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.builder = Gtk.Builder()

        self.builder.add_from_file(self.module_path + '/ui/lowpass.glade')

        self.build_lowpass_bin()

        self.load_ui()

        self.builder.connect_signals(self)

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_lowpass_bin(self):
        self.lowpass = Gst.ElementFactory.make('audiocheblimit', None)
        lowpass_input_level = Gst.ElementFactory.make('level',
                                                      'lowpass_input_level')
        lowpass_output_level = Gst.ElementFactory.make('level',
                                                       'lowpass_output_level')

        self.lowpass.set_property('mode', 'low-pass')
        self.lowpass.set_property('type', 1)
        self.lowpass.set_property('ripple', 0)

        self.bin = GstInsertBin.InsertBin.new('lowpass_bin')
        self.bin.append(self.lowpass, self.on_filter_added, None)
        self.bin.append(lowpass_input_level, self.on_filter_added, None)
        self.bin.append(lowpass_output_level, self.on_filter_added, None)

    def load_ui(self):
        self.ui_window = self.builder.get_object('window')

        self.ui_lowpass_controls = self.builder.get_object(
            'lowpass_controls')

        self.ui_lowpass_enable = self.builder.get_object('lowpass_enable')
        self.ui_lowpass_cutoff = self.builder.get_object('lowpass_cutoff')
        self.ui_lowpass_poles = self.builder.get_object('lowpass_poles')

        self.ui_lowpass_input_level_left = self.builder.get_object(
            'lowpass_input_level_left')
        self.ui_lowpass_input_level_right = self.builder.get_object(
            'lowpass_input_level_right')
        self.ui_lowpass_output_level_left = self.builder.get_object(
            'lowpass_output_level_left')
        self.ui_lowpass_output_level_right = self.builder.get_object(
            'lowpass_output_level_right')

        self.ui_lowpass_input_level_left_label = self.builder.get_object(
            'lowpass_input_level_left_label')
        self.ui_lowpass_input_level_right_label = self.builder.get_object(
            'lowpass_input_level_right_label')
        self.ui_lowpass_output_level_left_label = self.builder.get_object(
            'lowpass_output_level_left_label')
        self.ui_lowpass_output_level_right_label = self.builder.get_object(
            'lowpass_output_level_right_label')

    def init_ui(self):
        enabled = self.settings.get_value('lowpass-state').unpack()
        lowpass_cutoff_user = self.settings.get_value(
            'lowpass-cutoff').unpack()
        lowpass_poles_user = self.settings.get_value(
            'lowpass-poles').unpack()

        self.ui_lowpass_enable.set_state(enabled)
        self.ui_lowpass_cutoff.set_value(lowpass_cutoff_user)
        self.ui_lowpass_poles.set_value(lowpass_poles_user)

        self.lowpass.set_property('cutoff', lowpass_cutoff_user)
        self.lowpass.set_property('poles', lowpass_poles_user)

    def on_lowpass_enable_state_set(self, obj, state):
        self.ui_lowpass_controls.set_sensitive(state)

        out = GLib.Variant('b', state)
        self.settings.set_value('lowpass-state', out)

    def on_lowpass_cutoff_value_changed(self, obj):
        value = obj.get_value()
        self.lowpass.set_property('cutoff', value)

        out = GLib.Variant('i', value)

        self.settings.set_value('lowpass-cutoff', out)

    def on_lowpass_poles_value_changed(self, obj):
        value = obj.get_value()
        self.lowpass.set_property('poles', value)

        out = GLib.Variant('i', value)

        self.settings.set_value('lowpass-poles', out)

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

    def ui_update_lowpass_input_level(self, peak):
        widgets = [self.ui_lowpass_input_level_left,
                   self.ui_lowpass_input_level_right,
                   self.ui_lowpass_input_level_left_label,
                   self.ui_lowpass_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_lowpass_output_level(self, peak):
        widgets = [self.ui_lowpass_output_level_left,
                   self.ui_lowpass_output_level_right,
                   self.ui_lowpass_output_level_left_label,
                   self.ui_lowpass_output_level_right_label]

        self.ui_update_level(widgets, peak)

    def reset(self):
        self.settings.reset('lowpass-state')
        self.settings.reset('lowpass-cutoff')
        self.settings.reset('lowpass-poles')
