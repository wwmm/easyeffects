# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, Gst, GstInsertBin, Gtk

Gst.init(None)


class Reverb():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.builder = Gtk.Builder()

        self.builder.add_from_file(self.module_path + '/ui/reverb.glade')

        self.build_reverb_bin()

        self.load_ui()

        self.builder.connect_signals(self)

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_reverb_bin(self):
        self.reverb = Gst.ElementFactory.make('freeverb', None)
        reverb_input_level = Gst.ElementFactory.make('level',
                                                     'reverb_input_level')
        reverb_output_level = Gst.ElementFactory.make('level',
                                                      'reverb_output_level')

        self.bin = GstInsertBin.InsertBin.new('reverb_bin')
        self.bin.append(self.reverb, self.on_filter_added, None)
        self.bin.append(reverb_input_level, self.on_filter_added, None)
        self.bin.append(reverb_output_level, self.on_filter_added, None)

    def load_ui(self):
        self.ui_window = self.builder.get_object('window')

        self.ui_reverb_controls = self.builder.get_object(
            'reverb_controls')

        self.ui_reverb_enable = self.builder.get_object('reverb_enable')
        self.ui_reverb_room_size = self.builder.get_object(
            'reverb_room_size')
        self.ui_reverb_damping = self.builder.get_object('reverb_damping')
        self.ui_reverb_width = self.builder.get_object('reverb_width')
        self.ui_reverb_level = self.builder.get_object('reverb_level')

        self.ui_reverb_input_level_left = self.builder.get_object(
            'reverb_input_level_left')
        self.ui_reverb_input_level_right = self.builder.get_object(
            'reverb_input_level_right')
        self.ui_reverb_output_level_left = self.builder.get_object(
            'reverb_output_level_left')
        self.ui_reverb_output_level_right = self.builder.get_object(
            'reverb_output_level_right')

        self.ui_reverb_input_level_left_label = self.builder.get_object(
            'reverb_input_level_left_label')
        self.ui_reverb_input_level_right_label = self.builder.get_object(
            'reverb_input_level_right_label')
        self.ui_reverb_output_level_left_label = self.builder.get_object(
            'reverb_output_level_left_label')
        self.ui_reverb_output_level_right_label = self.builder.get_object(
            'reverb_output_level_right_label')

    def bind(self):
        self.settings.bind('reverb-state', self.ui_reverb_enable, 'active',
                           Gio.SettingsBindFlags.DEFAULT)
        self.settings.bind('reverb-state', self.ui_reverb_controls,
                           'sensitive', Gio.SettingsBindFlags.DEFAULT)
        self.settings.bind('reverb-room-size', self.ui_reverb_room_size,
                           'value', Gio.SettingsBindFlags.DEFAULT)
        self.settings.bind('reverb-damping', self.ui_reverb_damping,
                           'value', Gio.SettingsBindFlags.DEFAULT)
        self.settings.bind('reverb-width', self.ui_reverb_width,
                           'value', Gio.SettingsBindFlags.DEFAULT)
        self.settings.bind('reverb-level', self.ui_reverb_level,
                           'value', Gio.SettingsBindFlags.DEFAULT)

    def apply_reverb_preset(self, values):
        self.ui_reverb_room_size.set_value(values[0])
        self.ui_reverb_damping.set_value(values[1])
        self.ui_reverb_width.set_value(values[2])
        self.ui_reverb_level.set_value(values[3])

    def on_reverb_room_size_value_changed(self, obj):
        self.reverb.set_property('room-size', obj.get_value())

    def on_reverb_damping_value_changed(self, obj):
        self.reverb.set_property('damping', obj.get_value())

    def on_reverb_width_value_changed(self, obj):
        self.reverb.set_property('width', obj.get_value())

    def on_reverb_level_value_changed(self, obj):
        self.reverb.set_property('level', obj.get_value())

    def on_reverb_preset_clicked(self, obj):
        obj_id = Gtk.Buildable.get_name(obj)

        if obj_id == 'cathedral':
            value = self.settings.get_value('reverb-cathedral')
            self.apply_reverb_preset(value)
        elif obj_id == 'engine_room':
            value = self.settings.get_value('reverb-engine-room')
            self.apply_reverb_preset(value)
        elif obj_id == 'small_room':
            value = self.settings.get_value('reverb-small-room')
            self.apply_reverb_preset(value)

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

    def ui_update_reverb_input_level(self, peak):
        widgets = [self.ui_reverb_input_level_left,
                   self.ui_reverb_input_level_right,
                   self.ui_reverb_input_level_left_label,
                   self.ui_reverb_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_reverb_output_level(self, peak):
        widgets = [self.ui_reverb_output_level_left,
                   self.ui_reverb_output_level_right,
                   self.ui_reverb_output_level_left_label,
                   self.ui_reverb_output_level_right_label]

        self.ui_update_level(widgets, peak)

    def reset(self):
        self.settings.reset('reverb-state')
        self.settings.reset('reverb-room-size')
        self.settings.reset('reverb-damping')
        self.settings.reset('reverb-width')
        self.settings.reset('reverb-level')
