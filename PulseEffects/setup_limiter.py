# -*- coding: utf-8 -*-

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gtk


class SetupLimiter():

    def __init__(self, app):
        self.app = app
        self.builder = app.builder
        self.gst = app.gst
        self.settings = app.settings

        self.handlers = {
            'on_limiter_input_gain_value_changed':
                self.on_limiter_input_gain_value_changed,
            'on_limiter_limit_value_changed':
                self.on_limiter_limit_value_changed,
            'on_limiter_release_time_value_changed':
                self.on_limiter_release_time_value_changed,
            'on_limiter_preset_toggled':
                self.on_limiter_preset_toggled,
            'on_autovolume_enable_state_set':
                self.on_autovolume_enable_state_set
        }

        self.gst.connect('new_level_before_limiter',
                         self.on_new_level_before_limiter)
        self.gst.connect('new_level_after_limiter',
                         self.on_new_level_after_limiter)
        self.gst.connect('new_limiter_attenuation',
                         self.on_new_limiter_attenuation)
        self.gst.connect('new_autovolume', self.on_new_autovolume)

        self.limiter_user = self.settings.get_value(
            'limiter-user').unpack()

        self.limiter_input_gain = self.builder.get_object(
            'limiter_input_gain')
        self.limiter_limit = self.builder.get_object(
            'limiter_limit')
        self.limiter_release_time = self.builder.get_object(
            'limiter_release_time')
        self.limiter_attenuation_levelbar = self.builder.get_object(
            'limiter_attenuation_levelbar')

        self.limiter_scale_input_gain = self.builder.get_object(
            'limiter_scale_input_gain')
        self.limiter_scale_limit = self.builder.get_object(
            'limiter_scale_limit')
        self.limiter_scale_release_time = self.builder.get_object(
            'limiter_scale_release_time')

        self.limiter_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_LOW', 20)
        self.limiter_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_HIGH', 50)
        self.limiter_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_FULL', 70)

        self.limiter_level_before_left = self.builder.get_object(
            'limiter_level_before_left')
        self.limiter_level_before_right = self.builder.get_object(
            'limiter_level_before_right')
        self.limiter_level_after_left = self.builder.get_object(
            'limiter_level_after_left')
        self.limiter_level_after_right = self.builder.get_object(
            'limiter_level_after_right')

        self.limiter_level_label_before_left = self.builder.get_object(
            'limiter_level_label_before_left')
        self.limiter_level_label_before_right = self.builder.get_object(
            'limiter_level_label_before_right')
        self.limiter_level_label_after_left = self.builder.get_object(
            'limiter_level_label_after_left')
        self.limiter_level_label_after_right = self.builder.get_object(
            'limiter_level_label_after_right')
        self.limiter_attenuation_level_label = self.builder.get_object(
            'limiter_attenuation_level_label')

        autovolume_state_obj = self.builder.get_object('autovolume_state')

        autovolume_state = self.settings.get_value('autovolume-state').unpack()

        autovolume_state_obj.set_state(autovolume_state)

        self.init_menu()

        self.init_limiter()

    def apply_limiter_preset(self, values):
        self.limiter_input_gain.set_value(values[0])
        self.limiter_limit.set_value(values[1])
        self.limiter_release_time.set_value(values[2])

    def on_limiter_preset_toggled(self, obj):
        if obj.get_active():
            obj_id = Gtk.Buildable.get_name(obj)

            if obj_id == 'limiter_default':
                value = self.settings.get_value('limiter-default')
                self.apply_limiter_preset(value)

    def save_limiter_user(self, idx, value):
        self.limiter_user[idx] = value

        out = GLib.Variant('ad', self.limiter_user)

        self.settings.set_value('limiter-user', out)

    def on_limiter_input_gain_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_limiter_input_gain(value)
        self.save_limiter_user(0, value)

    def on_limiter_limit_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_limiter_limit(value)
        self.save_limiter_user(1, value)

    def on_limiter_release_time_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_limiter_release_time(value)
        self.save_limiter_user(2, value)

    def init_limiter(self):
        self.apply_limiter_preset(self.limiter_user)

        # we need this when saved value is equal to widget default value
        self.gst.set_limiter_input_gain(self.limiter_user[0])
        self.gst.set_limiter_limit(self.limiter_user[1])
        self.gst.set_limiter_release_time(self.limiter_user[2])

    def on_autovolume_enable_state_set(self, obj, state):
        self.gst.set_autovolume_state(state)

        if state:
            self.limiter_input_gain.set_value(-10)
            self.limiter_limit.set_value(-10)
            self.limiter_release_time.set_value(2.0)

            self.limiter_scale_input_gain.set_sensitive(False)
            self.limiter_scale_limit.set_sensitive(False)
            self.limiter_scale_release_time.set_sensitive(False)
        else:
            self.limiter_input_gain.set_value(-10)
            self.limiter_limit.set_value(0)
            self.limiter_release_time.set_value(1.0)

            self.limiter_scale_input_gain.set_sensitive(True)
            self.limiter_scale_limit.set_sensitive(True)
            self.limiter_scale_release_time.set_sensitive(True)

        out = GLib.Variant('b', state)
        self.settings.set_value('autovolume-state', out)

    def on_new_level_before_limiter(self, obj, left, right):
        if self.app.ui_initialized:
            if left >= -99:
                l_value = 10**(left / 20)
                self.limiter_level_before_left.set_value(l_value)
                self.limiter_level_label_before_left.set_text(str(round(left)))
            else:
                self.limiter_level_before_left.set_value(0)
                self.limiter_level_label_before_left.set_text('-99')

            if right >= -99:
                r_value = 10**(right / 20)
                self.limiter_level_before_right.set_value(r_value)
                self.limiter_level_label_before_right.set_text(
                    str(round(right)))
            else:
                self.limiter_level_before_right.set_value(0)
                self.limiter_level_label_before_right.set_text('-99')

    def on_new_level_after_limiter(self, obj, left, right):
        if self.app.ui_initialized:
            if left >= -99:
                l_value = 10**(left / 20)
                self.limiter_level_after_left.set_value(l_value)
                self.limiter_level_label_after_left.set_text(str(round(left)))
            else:
                self.limiter_level_after_left.set_value(0)
                self.limiter_level_label_after_left.set_text('-99')

            if right >= -99:
                r_value = 10**(right / 20)
                self.limiter_level_after_right.set_value(r_value)
                self.limiter_level_label_after_right.set_text(
                    str(round(right)))
            else:
                self.limiter_level_after_right.set_value(0)
                self.limiter_level_label_after_right.set_text('-99')

    def on_new_limiter_attenuation(self, obj, attenuation):
        if self.app.ui_initialized:
            self.limiter_attenuation_levelbar.set_value(attenuation)
            self.limiter_attenuation_level_label.set_text(
                str(round(attenuation)))

    def on_new_autovolume(self, obj, gain):
        if self.app.ui_initialized:
            self.limiter_input_gain.set_value(gain)

    def init_menu(self):
        button = self.builder.get_object('limiter_popover')
        menu = self.builder.get_object('limiter_menu')
        limiter_no_selection = self.builder.get_object('limiter_no_selection')

        popover = Gtk.Popover.new(button)
        popover.props.transitions_enabled = True
        popover.add(menu)

        def button_clicked(arg):
            if popover.get_visible():
                popover.hide()
            else:
                popover.show_all()
                limiter_no_selection.set_active(True)
                limiter_no_selection.hide()

        button.connect("clicked", button_clicked)
