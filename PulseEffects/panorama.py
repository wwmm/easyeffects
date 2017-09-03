# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gst, GstInsertBin, Gtk

Gst.init(None)


class Panorama():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.builder = Gtk.Builder()

        self.builder.add_from_file(self.module_path + '/ui/panorama.glade')

        self.build_panorama_bin()

        self.load_ui()

        self.builder.connect_signals(self)

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_panorama_bin(self):
        self.panorama = Gst.ElementFactory.make('audiopanorama', None)

        panorama_input_level = Gst.ElementFactory.make(
            'level', 'panorama_input_level')
        panorama_output_level = Gst.ElementFactory.make(
            'level', 'panorama_output_level')

        self.panorama.set_property('method', 'psychoacoustic')

        self.bin = GstInsertBin.InsertBin.new('panorama_bin')
        self.bin.append(self.panorama, self.on_filter_added, None)
        self.bin.append(panorama_input_level, self.on_filter_added, None)
        self.bin.append(panorama_output_level, self.on_filter_added, None)

    def load_ui(self):
        self.ui_window = self.builder.get_object('window')

        self.ui_panorama_controls = self.builder.get_object(
            'panorama_controls')

        self.ui_panorama_enable = self.builder.get_object('panorama_enable')
        self.ui_panorama = self.builder.get_object('panorama_position')

        self.ui_panorama_input_level_left = self.builder.get_object(
            'panorama_input_level_left')
        self.ui_panorama_input_level_right = self.builder.get_object(
            'panorama_input_level_right')
        self.ui_panorama_output_level_left = self.builder.get_object(
            'panorama_output_level_left')
        self.ui_panorama_output_level_right = self.builder.get_object(
            'panorama_output_level_right')

        self.ui_panorama_input_level_left_label = self.builder.get_object(
            'panorama_input_level_left_label')
        self.ui_panorama_input_level_right_label = self.builder.get_object(
            'panorama_input_level_right_label')
        self.ui_panorama_output_level_left_label = self.builder.get_object(
            'panorama_output_level_left_label')
        self.ui_panorama_output_level_right_label = self.builder.get_object(
            'panorama_output_level_right_label')

    def init_ui(self):
        enabled = self.settings.get_value('panorama-state').unpack()
        panorama = self.settings.get_value('panorama-position').unpack()

        self.ui_panorama_enable.set_state(enabled)
        self.ui_panorama_controls.set_sensitive(enabled)
        self.ui_panorama.set_value(panorama)
        self.panorama.set_property('panorama', panorama)

    def on_panorama_enable_state_set(self, obj, state):
        self.ui_panorama_controls.set_sensitive(state)

        out = GLib.Variant('b', state)
        self.settings.set_value('panorama-state', out)

    def on_panorama_position_value_changed(self, obj):
        value = obj.get_value()

        self.panorama.set_property('panorama', value)

        out = GLib.Variant('d', value)
        self.settings.set_value('panorama-position', out)

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

    def ui_update_panorama_input_level(self, peak):
        widgets = [self.ui_panorama_input_level_left,
                   self.ui_panorama_input_level_right,
                   self.ui_panorama_input_level_left_label,
                   self.ui_panorama_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_panorama_output_level(self, peak):
        widgets = [self.ui_panorama_output_level_left,
                   self.ui_panorama_output_level_right,
                   self.ui_panorama_output_level_left_label,
                   self.ui_panorama_output_level_right_label]

        self.ui_update_level(widgets, peak)

    def reset(self):
        self.settings.reset('panorama-state')
        self.settings.reset('panorama-position')
