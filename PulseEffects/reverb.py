# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gst, GstInsertBin, Gtk

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

    def init_ui(self):
        self.reverb_user = self.settings.get_value('reverb-user').unpack()
        self.apply_reverb_preset(self.reverb_user)

    def apply_reverb_preset(self, values):
        self.ui_reverb_room_size.set_value(values[0])
        self.ui_reverb_damping.set_value(values[1])
        self.ui_reverb_width.set_value(values[2])
        self.ui_reverb_level.set_value(values[3])

        # we need this when on value changed is not called
        self.reverb.set_property('room-size', values[0])
        self.reverb.set_property('damping', values[1])
        self.reverb.set_property('width', values[2])
        self.reverb.set_property('level', values[3])

    def save_reverb_user(self, idx, value):
        self.reverb_user[idx] = value

        out = GLib.Variant('ad', self.reverb_user)

        self.settings.set_value('reverb-user', out)

    def on_reverb_room_size_value_changed(self, obj):
        value = obj.get_value()
        self.reverb.set_property('room-size', value)
        self.save_reverb_user(0, value)

    def on_reverb_damping_value_changed(self, obj):
        value = obj.get_value()
        self.reverb.set_property('damping', value)
        self.save_reverb_user(1, value)

    def on_reverb_width_value_changed(self, obj):
        value = obj.get_value()
        self.reverb.set_property('width', value)
        self.save_reverb_user(2, value)

    def on_reverb_level_value_changed(self, obj):
        value = obj.get_value()
        self.reverb.set_property('level', value)
        self.save_reverb_user(3, value)

    def on_reverb_preset_clicked(self, obj):
        obj_id = Gtk.Buildable.get_name(obj)

        if obj_id == 'cathedral':
            value = self.settings.get_value('reverb-cathedral')
            self.apply_reverb_preset(value)
        elif obj_id == 'no_reverberation':
            value = self.settings.get_value('reverb-no-reverberation')
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
        self.settings.reset('reverb-user')
