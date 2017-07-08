# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gtk


class SetupLimiter():

    def __init__(self, app_builder, effects, settings):
        self.app_builder = app_builder
        self.effects = effects
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.handlers = {
            'on_limiter_input_gain_value_changed':
                self.on_limiter_input_gain_value_changed,
            'on_limiter_limit_value_changed':
                self.on_limiter_limit_value_changed,
            'on_limiter_release_time_value_changed':
                self.on_limiter_release_time_value_changed
        }

        self.limiter_input_gain = self.app_builder.get_object(
            'limiter_input_gain')
        self.limiter_limit = self.app_builder.get_object(
            'limiter_limit')
        self.limiter_release_time = self.app_builder.get_object(
            'limiter_release_time')
        self.limiter_attenuation_levelbar = self.app_builder.get_object(
            'limiter_attenuation_levelbar')

        self.limiter_scale_input_gain = self.app_builder.get_object(
            'limiter_scale_input_gain')
        self.limiter_scale_limit = self.app_builder.get_object(
            'limiter_scale_limit')
        self.limiter_scale_release_time = self.app_builder.get_object(
            'limiter_scale_release_time')

        self.limiter_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_LOW', 20)
        self.limiter_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_HIGH', 50)
        self.limiter_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_FULL', 70)

        self.limiter_input_level_left = self.app_builder.get_object(
            'limiter_input_level_left')
        self.limiter_input_level_right = self.app_builder.get_object(
            'limiter_input_level_right')
        self.limiter_output_level_left = self.app_builder.get_object(
            'limiter_output_level_left')
        self.limiter_output_level_right = self.app_builder.get_object(
            'limiter_output_level_right')

        self.limiter_input_level_left_label = self.app_builder.get_object(
            'limiter_input_level_left_label')
        self.limiter_input_level_right_label = self.app_builder.get_object(
            'limiter_input_level_right_label')
        self.limiter_output_level_left_label = self.app_builder.get_object(
            'limiter_output_level_left_label')
        self.limiter_output_level_right_label = self.app_builder.get_object(
            'limiter_output_level_right_label')
        self.limiter_attenuation_level_label = self.app_builder.get_object(
            'limiter_attenuation_level_label')

        self.init_menu()

    def init(self):
        self.limiter_user = self.settings.get_value(
            'limiter-user').unpack()

        self.apply_limiter_preset(self.limiter_user)

        # we need this when saved value is equal to widget default value
        self.effects.set_limiter_input_gain(self.limiter_user[0])
        self.effects.set_limiter_limit(self.limiter_user[1])
        self.effects.set_limiter_release_time(self.limiter_user[2])

    def connect_signals(self):
        self.effects.connect('new_limiter_input_level',
                             self.on_new_limiter_input_level)
        self.effects.connect('new_limiter_output_level',
                             self.on_new_limiter_output_level)
        self.effects.connect('new_limiter_attenuation',
                             self.on_new_limiter_attenuation)

    def apply_limiter_preset(self, values):
        self.limiter_input_gain.set_value(values[0])
        self.limiter_limit.set_value(values[1])
        self.limiter_release_time.set_value(values[2])

    def on_limiter_preset_clicked(self, obj):
        obj_id = Gtk.Buildable.get_name(obj)

        if obj_id == 'default':
            value = self.settings.get_value('limiter-default')
            self.apply_limiter_preset(value)

    def save_limiter_user(self, idx, value):
        self.limiter_user[idx] = value

        out = GLib.Variant('ad', self.limiter_user)

        self.settings.set_value('limiter-user', out)

    def on_limiter_input_gain_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_limiter_input_gain(value)
        self.save_limiter_user(0, value)

    def on_limiter_limit_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_limiter_limit(value)
        self.save_limiter_user(1, value)

    def on_limiter_release_time_value_changed(self, obj):
        value = obj.get_value()
        self.effects.set_limiter_release_time(value)
        self.save_limiter_user(2, value)

    def on_new_limiter_input_level(self, obj, left, right):
        if left >= -99:
            l_value = 10**(left / 20)
            self.limiter_input_level_left.set_value(l_value)
            self.limiter_input_level_left_label.set_text(str(round(left)))
        else:
            self.limiter_input_level_left.set_value(0)
            self.limiter_input_level_left_label.set_text('-99')

        if right >= -99:
            r_value = 10**(right / 20)
            self.limiter_input_level_right.set_value(r_value)
            self.limiter_input_level_right_label.set_text(
                str(round(right)))
        else:
            self.limiter_input_level_right.set_value(0)
            self.limiter_input_level_right_label.set_text('-99')

    def on_new_limiter_output_level(self, obj, left, right):
        if left >= -99:
            l_value = 10**(left / 20)
            self.limiter_output_level_left.set_value(l_value)
            self.limiter_output_level_left_label.set_text(str(round(left)))
        else:
            self.limiter_output_level_left.set_value(0)
            self.limiter_output_level_left_label.set_text('-99')

        if right >= -99:
            r_value = 10**(right / 20)
            self.limiter_output_level_right.set_value(r_value)
            self.limiter_output_level_right_label.set_text(
                str(round(right)))
        else:
            self.limiter_output_level_right.set_value(0)
            self.limiter_output_level_right_label.set_text('-99')

    def on_new_limiter_attenuation(self, obj, attenuation):
        self.limiter_attenuation_levelbar.set_value(attenuation)
        self.limiter_attenuation_level_label.set_text(
            str(round(attenuation)))

    def init_menu(self):
        builder = Gtk.Builder()

        builder.add_from_file(self.module_path + '/ui/limiter_menu.glade')

        builder.connect_signals(self)

        menu = builder.get_object('menu')

        self.popover_button = self.app_builder.get_object('limiter_popover')

        popover = Gtk.Popover.new(self.popover_button)
        popover.props.transitions_enabled = True
        popover.add(menu)

        def button_clicked(arg):
            if popover.get_visible():
                popover.hide()
            else:
                popover.show_all()

        self.popover_button.connect("clicked", button_clicked)

    def reset(self):
        self.settings.reset('limiter-user')

        self.init()
