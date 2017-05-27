# -*- coding: utf-8 -*-

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gtk


class SetupEqualizer():

    def __init__(self, app):
        self.app = app
        self.builder = app.builder
        self.gst = app.gst
        self.settings = app.settings

        self.handlers = {
            'on_eq_preamp_value_changed': self.on_eq_preamp_value_changed,
            'on_eq_band0_value_changed': self.on_eq_band0_value_changed,
            'on_eq_band1_value_changed': self.on_eq_band1_value_changed,
            'on_eq_band2_value_changed': self.on_eq_band2_value_changed,
            'on_eq_band3_value_changed': self.on_eq_band3_value_changed,
            'on_eq_band4_value_changed': self.on_eq_band4_value_changed,
            'on_eq_band5_value_changed': self.on_eq_band5_value_changed,
            'on_eq_band6_value_changed': self.on_eq_band6_value_changed,
            'on_eq_band7_value_changed': self.on_eq_band7_value_changed,
            'on_eq_band8_value_changed': self.on_eq_band8_value_changed,
            'on_eq_band9_value_changed': self.on_eq_band9_value_changed,
            'on_eq_band10_value_changed': self.on_eq_band10_value_changed,
            'on_eq_band11_value_changed': self.on_eq_band11_value_changed,
            'on_eq_band12_value_changed': self.on_eq_band12_value_changed,
            'on_eq_band13_value_changed': self.on_eq_band13_value_changed,
            'on_eq_band14_value_changed': self.on_eq_band14_value_changed,
            'on_eq_preset_toggled': self.on_eq_preset_toggled,
        }

        self.gst.connect('new_level_after_eq', self.on_new_level_after_eq)

        self.eq_preamp = self.builder.get_object('eq_preamp')
        self.eq_band0 = self.builder.get_object('eq_band0')
        self.eq_band1 = self.builder.get_object('eq_band1')
        self.eq_band2 = self.builder.get_object('eq_band2')
        self.eq_band3 = self.builder.get_object('eq_band3')
        self.eq_band4 = self.builder.get_object('eq_band4')
        self.eq_band5 = self.builder.get_object('eq_band5')
        self.eq_band6 = self.builder.get_object('eq_band6')
        self.eq_band7 = self.builder.get_object('eq_band7')
        self.eq_band8 = self.builder.get_object('eq_band8')
        self.eq_band9 = self.builder.get_object('eq_band9')
        self.eq_band10 = self.builder.get_object('eq_band10')
        self.eq_band11 = self.builder.get_object('eq_band11')
        self.eq_band12 = self.builder.get_object('eq_band12')
        self.eq_band13 = self.builder.get_object('eq_band13')
        self.eq_band14 = self.builder.get_object('eq_band14')

        self.eq_left_level = self.builder.get_object('eq_left_level')
        self.eq_right_level = self.builder.get_object('eq_right_level')

        self.eq_left_level_label = self.builder.get_object(
            'eq_left_level_label')
        self.eq_right_level_label = self.builder.get_object(
            'eq_right_level_label')

        self.init_menu()

    def init_menu(self):
        button = self.builder.get_object('equalizer_popover')
        menu = self.builder.get_object('equalizer_menu')
        eq_no_selection = self.builder.get_object('eq_no_selection')

        popover = Gtk.Popover.new(button)
        popover.props.transitions_enabled = True
        popover.add(menu)

        def button_clicked(arg):
            if popover.get_visible():
                popover.hide()
            else:
                popover.show_all()
                eq_no_selection.set_active(True)
                eq_no_selection.hide()

        button.connect("clicked", button_clicked)

    def init(self):
        self.eq_band_user = self.settings.get_value('equalizer-user').unpack()

        self.eq_preamp_user = self.settings.get_value(
            'equalizer-preamp').unpack()

        self.eq_preamp.set_value(self.eq_preamp_user)
        self.apply_eq_preset(self.eq_band_user)

        # we need this when saved value is equal to widget default value

        value_linear = 10**(self.eq_preamp_user / 20)
        self.gst.set_eq_preamp(value_linear)

        self.gst.set_eq_band0(self.eq_band_user[0])
        self.gst.set_eq_band1(self.eq_band_user[1])
        self.gst.set_eq_band2(self.eq_band_user[2])
        self.gst.set_eq_band3(self.eq_band_user[3])
        self.gst.set_eq_band4(self.eq_band_user[4])
        self.gst.set_eq_band5(self.eq_band_user[5])
        self.gst.set_eq_band6(self.eq_band_user[6])
        self.gst.set_eq_band7(self.eq_band_user[7])
        self.gst.set_eq_band8(self.eq_band_user[8])
        self.gst.set_eq_band9(self.eq_band_user[9])
        self.gst.set_eq_band10(self.eq_band_user[10])
        self.gst.set_eq_band11(self.eq_band_user[11])
        self.gst.set_eq_band12(self.eq_band_user[12])
        self.gst.set_eq_band13(self.eq_band_user[13])
        self.gst.set_eq_band14(self.eq_band_user[14])

    def on_new_level_after_eq(self, obj, left, right):
        if self.app.ui_initialized:
            if left >= -99:
                l_value = 10**(left / 20)
                self.eq_left_level.set_value(l_value)
                self.eq_left_level_label.set_text(str(round(left)))
            else:
                self.eq_left_level.set_value(0)
                self.eq_left_level_label.set_text('-99')

            if right >= -99:
                r_value = 10**(right / 20)
                self.eq_right_level.set_value(r_value)
                self.eq_right_level_label.set_text(str(round(right)))
            else:
                self.eq_right_level.set_value(0)
                self.eq_right_level_label.set_text('-99')

    def apply_eq_preset(self, values):
        self.eq_band0.set_value(values[0])
        self.eq_band1.set_value(values[1])
        self.eq_band2.set_value(values[2])
        self.eq_band3.set_value(values[3])
        self.eq_band4.set_value(values[4])
        self.eq_band5.set_value(values[5])
        self.eq_band6.set_value(values[6])
        self.eq_band7.set_value(values[7])
        self.eq_band8.set_value(values[8])
        self.eq_band9.set_value(values[9])
        self.eq_band10.set_value(values[10])
        self.eq_band11.set_value(values[11])
        self.eq_band12.set_value(values[12])
        self.eq_band13.set_value(values[13])
        self.eq_band14.set_value(values[14])

    def on_eq_preset_toggled(self, obj):
        if obj.get_active():
            obj_id = Gtk.Buildable.get_name(obj)

            if obj_id == 'eq_equal_loudness_20':
                value = self.settings.get_value('equalizer-equal-loudness-20')
                self.apply_eq_preset(value)
            elif obj_id == 'eq_equal_loudness_40':
                value = self.settings.get_value('equalizer-equal-loudness-40')
                self.apply_eq_preset(value)
            elif obj_id == 'eq_equal_loudness_60':
                value = self.settings.get_value('equalizer-equal-loudness-60')
                self.apply_eq_preset(value)
            elif obj_id == 'eq_equal_loudness_80':
                value = self.settings.get_value('equalizer-equal-loudness-80')
                self.apply_eq_preset(value)
            elif obj_id == 'eq_equal_loudness_100':
                value = self.settings.get_value('equalizer-equal-loudness-100')
                self.apply_eq_preset(value)
            elif obj_id == 'eq_flat':
                value = self.settings.get_value('equalizer-flat')
                self.apply_eq_preset(value)

    def save_eq_user(self, idx, value):
        self.eq_band_user[idx] = value

        out = GLib.Variant('ad', self.eq_band_user)

        self.settings.set_value('equalizer-user', out)

    def on_eq_preamp_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20)

        self.gst.set_eq_preamp(value_linear)

        out = GLib.Variant('d', value_db)

        self.settings.set_value('equalizer-preamp', out)

    def on_eq_band0_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_eq_band0(value)
        self.save_eq_user(0, value)

    def on_eq_band1_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_eq_band1(value)
        self.save_eq_user(1, value)

    def on_eq_band2_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_eq_band2(value)
        self.save_eq_user(2, value)

    def on_eq_band3_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_eq_band3(value)
        self.save_eq_user(3, value)

    def on_eq_band4_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_eq_band4(value)
        self.save_eq_user(4, value)

    def on_eq_band5_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_eq_band5(value)
        self.save_eq_user(5, value)

    def on_eq_band6_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_eq_band6(value)
        self.save_eq_user(6, value)

    def on_eq_band7_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_eq_band7(value)
        self.save_eq_user(7, value)

    def on_eq_band8_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_eq_band8(value)
        self.save_eq_user(8, value)

    def on_eq_band9_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_eq_band9(value)
        self.save_eq_user(9, value)

    def on_eq_band10_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_eq_band10(value)
        self.save_eq_user(10, value)

    def on_eq_band11_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_eq_band11(value)
        self.save_eq_user(11, value)

    def on_eq_band12_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_eq_band12(value)
        self.save_eq_user(12, value)

    def on_eq_band13_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_eq_band13(value)
        self.save_eq_user(13, value)

    def on_eq_band14_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_eq_band14(value)
        self.save_eq_user(14, value)
