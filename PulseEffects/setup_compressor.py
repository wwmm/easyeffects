# -*- coding: utf-8 -*-

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gtk


class SetupCompressor():

    def __init__(self, app):
        self.app = app
        self.app_builder = app.builder
        self.gst = app.gst
        self.settings = app.settings
        self.module_path = app.module_path

        self.handlers = {
            'on_compressor_measurement_type':
            self.on_compressor_measurement_type,
            'on_compressor_attack_time_value_changed':
            self.on_compressor_attack_time_value_changed,
            'on_compressor_release_time_value_changed':
            self.on_compressor_release_time_value_changed,
            'on_compressor_threshold_value_changed':
            self.on_compressor_threshold_value_changed,
            'on_compressor_ratio_value_changed':
            self.on_compressor_ratio_value_changed,
            'on_compressor_knee_value_changed':
            self.on_compressor_knee_value_changed,
            'on_compressor_makeup_value_changed':
            self.on_compressor_makeup_value_changed
        }

        self.gst.connect('new_compressor_input_level',
                         self.on_new_compressor_input_level)
        self.gst.connect('new_compressor_output_level',
                         self.on_new_compressor_output_level)
        self.gst.connect('new_compressor_gain_reduction',
                         self.on_new_compressor_gain_reduction)

        self.compressor_rms = self.app_builder.get_object('compressor_rms')
        self.compressor_peak = self.app_builder.get_object('compressor_peak')
        self.compressor_attack = self.app_builder.get_object(
            'compressor_attack')
        self.compressor_release = self.app_builder.get_object(
            'compressor_release')
        self.compressor_threshold = self.app_builder.get_object(
            'compressor_threshold')
        self.compressor_ratio = self.app_builder.get_object(
            'compressor_ratio')
        self.compressor_knee = self.app_builder.get_object(
            'compressor_knee')
        self.compressor_makeup = self.app_builder.get_object(
            'compressor_makeup')

        self.compressor_input_level_left = self.app_builder.get_object(
            'compressor_input_level_left')
        self.compressor_input_level_right = self.app_builder.get_object(
            'compressor_input_level_right')
        self.compressor_output_level_left = self.app_builder.get_object(
            'compressor_output_level_left')
        self.compressor_output_level_right = self.app_builder.get_object(
            'compressor_output_level_right')
        self.compressor_gain_reduction_levelbar = self.app_builder.get_object(
            'compressor_gain_reduction_levelbar')

        self.compressor_input_level_left_label = self.app_builder.get_object(
            'compressor_input_level_left_label')
        self.compressor_input_level_right_label = self.app_builder.get_object(
            'compressor_input_level_right_label')
        self.compressor_output_level_left_label = self.app_builder.get_object(
            'compressor_output_level_left_label')
        self.compressor_output_level_right_label = self.app_builder.get_object(
            'compressor_output_level_right_label')
        self.compressor_gain_reduction_level_label = \
            self.app_builder.get_object(
                'compressor_gain_reduction_level_label')

        self.compressor_gain_reduction_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_LOW', 6)
        self.compressor_gain_reduction_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_HIGH', 18)
        self.compressor_gain_reduction_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_FULL', 24)

        self.init_menu()

    def init_menu(self):
        builder = Gtk.Builder()

        builder.add_from_file(self.module_path + '/ui/compressor_menu.glade')

        builder.connect_signals(self)

        menu = builder.get_object('menu')
        compressor_no_selection = builder.get_object(
            'compressor_no_selection')

        button = self.app_builder.get_object('compressor_popover')

        popover = Gtk.Popover.new(button)
        popover.props.transitions_enabled = True
        popover.add(menu)

        def button_clicked(arg):
            if popover.get_visible():
                popover.hide()
            else:
                popover.show_all()
                compressor_no_selection.set_active(True)
                compressor_no_selection.hide()

        button.connect("clicked", button_clicked)

    def init(self):
        self.compressor_user = self.settings.get_value(
            'compressor-user').unpack()

        self.apply_compressor_preset(self.compressor_user)

        # we need this when saved value is equal to widget default value
        self.gst.set_compressor_measurement_type(self.compressor_user[0])
        self.gst.set_compressor_attack(self.compressor_user[1])
        self.gst.set_compressor_release(self.compressor_user[2])
        self.gst.set_compressor_threshold(self.compressor_user[3])
        self.gst.set_compressor_ratio(self.compressor_user[4])
        self.gst.set_compressor_knee(self.compressor_user[5])
        self.gst.set_compressor_makeup(self.compressor_user[6])

    def on_new_compressor_input_level(self, obj, left, right):
        if self.app.ui_initialized:
            if left >= -99:
                l_value = 10**(left / 20)
                self.compressor_input_level_left.set_value(l_value)
                self.compressor_input_level_left_label.set_text(
                    str(round(left)))
            else:
                self.compressor_input_level_left.set_value(0)
                self.compressor_input_level_left_label.set_text('-99')

            if right >= -99:
                r_value = 10**(right / 20)
                self.compressor_input_level_right.set_value(r_value)
                self.compressor_input_level_right_label.set_text(
                    str(round(right)))
            else:
                self.compressor_input_level_right.set_value(0)
                self.compressor_input_level_right_label.set_text('-99')

    def on_new_compressor_output_level(self, obj, left, right):
        if self.app.ui_initialized:
            if left >= -99:
                l_value = 10**(left / 20)
                self.compressor_output_level_left.set_value(l_value)
                self.compressor_output_level_left_label.set_text(
                    str(round(left)))
            else:
                self.compressor_output_level_left.set_value(0)
                self.compressor_output_level_left_label.set_text('-99')

            if right >= -99:
                r_value = 10**(right / 20)
                self.compressor_output_level_right.set_value(r_value)
                self.compressor_output_level_right_label.set_text(
                    str(round(right)))
            else:
                self.compressor_output_level_right.set_value(0)
                self.compressor_output_level_right_label.set_text('-99')

    def on_new_compressor_gain_reduction(self, obj, gain_reduction):
        if self.app.ui_initialized:
            value = abs(gain_reduction)
            self.compressor_gain_reduction_levelbar.set_value(value)
            self.compressor_gain_reduction_level_label.set_text(
                str(round(value)))

    def apply_compressor_preset(self, values):
        if values[0] == 0:
            self.compressor_rms.set_active(True)
        elif values[0] == 1:
            self.compressor_peak.set_active(True)

        self.compressor_attack.set_value(values[1])
        self.compressor_release.set_value(values[2])
        self.compressor_threshold.set_value(values[3])
        self.compressor_ratio.set_value(values[4])
        self.compressor_knee.set_value(values[5])
        self.compressor_makeup.set_value(values[6])

    def on_compressor_preset_toggled(self, obj):
        if obj.get_active():
            obj_id = Gtk.Buildable.get_name(obj)

            if obj_id == 'compressor_preset_none':
                value = self.settings.get_value('compressor-no-compression')
                self.apply_compressor_preset(value)

    def save_compressor_user(self, idx, value):
        self.compressor_user[idx] = value

        out = GLib.Variant('ad', self.compressor_user)

        self.settings.set_value('compressor-user', out)

    def on_compressor_measurement_type(self, obj):
        if obj.get_active():
            label = obj.get_label()

            if label == 'rms':
                self.gst.set_compressor_measurement_type(0)
                self.save_compressor_user(0, 0)
            elif label == 'peak':
                self.gst.set_compressor_measurement_type(1)
                self.save_compressor_user(0, 1)

    def on_compressor_attack_time_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_compressor_attack(value)
        self.save_compressor_user(1, value)

    def on_compressor_release_time_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_compressor_release(value)
        self.save_compressor_user(2, value)

    def on_compressor_threshold_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_compressor_threshold(value)
        self.save_compressor_user(3, value)

    def on_compressor_ratio_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_compressor_ratio(value)
        self.save_compressor_user(4, value)

    def on_compressor_knee_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_compressor_knee(value)
        self.save_compressor_user(5, value)

    def on_compressor_makeup_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_compressor_makeup(value)
        self.save_compressor_user(6, value)

    def reset(self):
        self.settings.reset('compressor-user')

        self.init()
