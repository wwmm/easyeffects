# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GObject, Gst, GstInsertBin, Gtk

Gst.init(None)


class Lowpass():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.build_bin()
        self.load_ui()

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_bin(self):
        self.lowpass = Gst.ElementFactory.make('audiocheblimit', None)
        input_level = Gst.ElementFactory.make('level', 'lowpass_input_level')
        output_level = Gst.ElementFactory.make('level', 'lowpass_output_level')

        self.lowpass.set_property('mode', 'low-pass')
        self.lowpass.set_property('type', 1)
        self.lowpass.set_property('ripple', 0)

        self.bin = GstInsertBin.InsertBin.new('lowpass_bin')
        self.bin.append(self.lowpass, self.on_filter_added, None)
        self.bin.append(input_level, self.on_filter_added, None)
        self.bin.append(output_level, self.on_filter_added, None)

    def load_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/lowpass.glade')

        self.ui_window = self.builder.get_object('window')
        self.ui_listbox_control = self.builder.get_object('listbox_control')

        self.ui_enable = self.builder.get_object('enable')
        self.ui_cutoff = self.builder.get_object('cutoff')
        self.ui_poles = self.builder.get_object('poles')

        self.ui_input_level_left = self.builder.get_object(
            'input_level_left')
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

    def bind(self):
        # binding ui widgets to gstreamer plugins

        flag = GObject.BindingFlags.DEFAULT

        self.ui_cutoff.bind_property('value', self.lowpass, 'cutoff', flag)
        self.ui_poles.bind_property('value', self.lowpass, 'poles', flag)

        # binding ui widgets to gsettings

        flag = Gio.SettingsBindFlags.DEFAULT

        self.settings.bind('lowpass-state', self.ui_enable, 'active', flag)
        self.settings.bind('lowpass-state', self.ui_window, 'sensitive',
                           Gio.SettingsBindFlags.GET)
        self.settings.bind('lowpass-cutoff', self.ui_cutoff, 'value', flag)
        self.settings.bind('lowpass-poles', self.ui_poles, 'value', flag)

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
        widgets = [self.ui_input_level_left, self.ui_input_level_right,
                   self.ui_input_level_left_label,
                   self.ui_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_lowpass_output_level(self, peak):
        widgets = [self.ui_output_level_left, self.ui_output_level_right,
                   self.ui_output_level_left_label,
                   self.ui_output_level_right_label]

        self.ui_update_level(widgets, peak)

    def reset(self):
        self.settings.reset('lowpass-state')
        self.settings.reset('lowpass-cutoff')
        self.settings.reset('lowpass-poles')
