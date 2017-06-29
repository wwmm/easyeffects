# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gtk


class SetupReverb():

    def __init__(self, app_builder, effects, settings):
        self.app_builder = app_builder
        self.effects = effects
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.handlers = {
            'on_reverb_room_size_value_changed':
            self.on_reverb_room_size_value_changed,
            'on_reverb_damping_value_changed':
            self.on_reverb_damping_value_changed,
            'on_reverb_width_value_changed':
            self.on_reverb_width_value_changed,
            'on_reverb_level_value_changed':
            self.on_reverb_level_value_changed
        }

        self.reverb_room_size = self.app_builder.get_object('reverb_room_size')
        self.reverb_damping = self.app_builder.get_object('reverb_damping')
        self.reverb_width = self.app_builder.get_object('reverb_width')
        self.reverb_level = self.app_builder.get_object('reverb_level')

        self.reverb_input_level_left = self.app_builder.get_object(
            'reverb_input_level_left')
        self.reverb_input_level_right = self.app_builder.get_object(
            'reverb_input_level_right')
        self.reverb_output_level_left = self.app_builder.get_object(
            'reverb_output_level_left')
        self.reverb_output_level_right = self.app_builder.get_object(
            'reverb_output_level_right')

        self.reverb_input_level_left_label = self.app_builder.get_object(
            'reverb_input_level_left_label')
        self.reverb_input_level_right_label = self.app_builder.get_object(
            'reverb_input_level_right_label')
        self.reverb_output_level_left_label = self.app_builder.get_object(
            'reverb_output_level_left_label')
        self.reverb_output_level_right_label = self.app_builder.get_object(
            'reverb_output_level_right_label')

        self.init_menu()

    def init_menu(self):
        builder = Gtk.Builder()

        builder.add_from_file(self.module_path + '/ui/reverb_menu.glade')

        builder.connect_signals(self)

        menu = builder.get_object('menu')
        reverb_no_selection = builder.get_object('reverb_no_selection')

        button = self.app_builder.get_object('reverb_popover')

        popover = Gtk.Popover.new(button)
        popover.props.transitions_enabled = True
        popover.add(menu)

        def button_clicked(arg):
            if popover.get_visible():
                popover.hide()
            else:
                popover.show_all()
                reverb_no_selection.set_active(True)
                reverb_no_selection.hide()

        button.connect("clicked", button_clicked)

    def init(self):
        self.reverb_user = self.settings.get_value('reverb-user').unpack()

        self.apply_reverb_preset(self.reverb_user)

        # we need this when saved value is equal to widget default value
        self.effects.set_reverb_room_size(self.reverb_user[0])
        self.effects.set_reverb_damping(self.reverb_user[1])
        self.effects.set_reverb_width(self.reverb_user[2])
        self.effects.set_reverb_level(self.reverb_user[3])

    def connect_signals(self):
        self.effects.connect('new_reverb_input_level',
                             self.on_new_reverb_input_level)
        self.effects.connect('new_reverb_output_level',
                             self.on_new_reverb_output_level)

    def on_new_reverb_input_level(self, obj, left, right):
        if left >= -99:
            l_value = 10**(left / 20)
            self.reverb_input_level_left.set_value(l_value)
            self.reverb_input_level_left_label.set_text(str(round(left)))
        else:
            self.reverb_input_level_left.set_value(0)
            self.reverb_input_level_left_label.set_text('-99')

        if right >= -99:
            r_value = 10**(right / 20)
            self.reverb_input_level_right.set_value(r_value)
            self.reverb_input_level_right_label.set_text(
                str(round(right)))
        else:
            self.reverb_input_level_right.set_value(0)
            self.reverb_input_level_right_label.set_text('-99')

    def on_new_reverb_output_level(self, obj, left, right):
        if left >= -99:
            l_value = 10**(left / 20)
            self.reverb_output_level_left.set_value(l_value)
            self.reverb_output_level_left_label.set_text(str(round(left)))
        else:
            self.reverb_output_level_left.set_value(0)
            self.reverb_output_level_left_label.set_text('-99')

        if right >= -99:
            r_value = 10**(right / 20)
            self.reverb_output_level_right.set_value(r_value)
            self.reverb_output_level_right_label.set_text(
                str(round(right)))
        else:
            self.reverb_output_level_right.set_value(0)
            self.reverb_output_level_right_label.set_text('-99')

    def apply_reverb_preset(self, values):
        self.reverb_room_size.set_value(values[0])
        self.reverb_damping.set_value(values[1])
        self.reverb_width.set_value(values[2])
        self.reverb_level.set_value(values[3])

    def on_reverb_preset_toggled(self, obj):
        if obj.get_active():
            obj_id = Gtk.Buildable.get_name(obj)

            if obj_id == 'reverb_cathedral':
                value = self.settings.get_value('reverb-cathedral')
                self.apply_reverb_preset(value)
            elif obj_id == 'reverb_no_reverberation':
                value = self.settings.get_value('reverb-no-reverberation')
                self.apply_reverb_preset(value)
            elif obj_id == 'reverb_engine_room':
                value = self.settings.get_value('reverb-engine-room')
                self.apply_reverb_preset(value)
            elif obj_id == 'reverb_small_room':
                value = self.settings.get_value('reverb-small-room')
                self.apply_reverb_preset(value)

    def save_reverb_user(self, idx, value):
        self.reverb_user[idx] = value

        out = GLib.Variant('ad', self.reverb_user)

        self.settings.set_value('reverb-user', out)

    def on_reverb_room_size_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_reverb_room_size(value)
        self.save_reverb_user(0, value)

    def on_reverb_damping_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_reverb_damping(value)
        self.save_reverb_user(1, value)

    def on_reverb_width_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_reverb_width(value)
        self.save_reverb_user(2, value)

    def on_reverb_level_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_reverb_level(value)
        self.save_reverb_user(3, value)

    def reset(self):
        self.settings.reset('reverb-user')

        self.init()
