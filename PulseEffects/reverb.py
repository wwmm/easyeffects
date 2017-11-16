# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GObject, Gst, GstInsertBin, Gtk

Gst.init(None)


class Reverb():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.build_bin()

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_bin(self):
        self.reverb = Gst.ElementFactory.make('freeverb', None)
        input_level = Gst.ElementFactory.make('level', 'reverb_input_level')
        output_level = Gst.ElementFactory.make('level', 'reverb_output_level')

        self.bin = GstInsertBin.InsertBin.new('reverb_bin')
        self.bin.append(input_level, self.on_filter_added, None)
        self.bin.append(self.reverb, self.on_filter_added, None)
        self.bin.append(output_level, self.on_filter_added, None)

    def init_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/reverb.glade')
        self.builder.connect_signals(self)

        self.ui_window = self.builder.get_object('window')
        self.ui_listbox_control = self.builder.get_object('listbox_control')

        self.ui_enable = self.builder.get_object('enable')
        self.ui_room_size = self.builder.get_object('room_size')
        self.ui_damping = self.builder.get_object('damping')
        self.ui_width = self.builder.get_object('width')
        self.ui_level = self.builder.get_object('level')

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

        self.ui_room_size.bind_property('value', self.reverb, 'room-size',
                                        flag)
        self.ui_damping.bind_property('value', self.reverb, 'damping', flag)
        self.ui_width.bind_property('value', self.reverb, 'width', flag)
        self.ui_level.bind_property('value', self.reverb, 'level', flag)

        # binding ui widgets to gsettings

        flag = Gio.SettingsBindFlags.DEFAULT

        self.settings.bind('reverb-state', self.ui_enable, 'active', flag)
        self.settings.bind('reverb-state', self.ui_window, 'sensitive',
                           Gio.SettingsBindFlags.GET)
        self.settings.bind('reverb-room-size', self.ui_room_size, 'value',
                           flag)
        self.settings.bind('reverb-damping', self.ui_damping, 'value', flag)
        self.settings.bind('reverb-width', self.ui_width, 'value', flag)
        self.settings.bind('reverb-level', self.ui_level, 'value', flag)

    def apply_reverb_preset(self, values):
        self.ui_room_size.set_value(values[0])
        self.ui_damping.set_value(values[1])
        self.ui_width.set_value(values[2])
        self.ui_level.set_value(values[3])

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
        widgets = [self.ui_input_level_left, self.ui_input_level_right,
                   self.ui_input_level_left_label,
                   self.ui_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_reverb_output_level(self, peak):
        widgets = [self.ui_output_level_left, self.ui_output_level_right,
                   self.ui_output_level_left_label,
                   self.ui_output_level_right_label]

        self.ui_update_level(widgets, peak)

    def reset(self):
        self.settings.reset('reverb-state')
        self.settings.reset('reverb-room-size')
        self.settings.reset('reverb-damping')
        self.settings.reset('reverb-width')
        self.settings.reset('reverb-level')
