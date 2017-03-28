# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GLib, Gtk

from PulseEffects.gst import GstEffects
from PulseEffects.pulse_manager import PulseManager


class Application(Gtk.Application):

    def __init__(self):
        app_id = 'com.github.wwmm.pulseeffects'

        Gtk.Application.__init__(self, application_id=app_id)

        self.ui_initialized = False
        self.spectrum_magnitudes = []
        self.module_path = os.path.dirname(__file__)

        self.settings = Gio.Settings('com.github.wwmm.pulseeffects')

        self.pm = PulseManager()

        self.pm.connect('sink_inputs_changed', self.on_sink_inputs_changed)

        self.gst = GstEffects()

        self.gst.set_output_sink_name(self.pm.default_sink_name)

        self.gst.connect('new_level_before_limiter',
                         self.on_new_level_before_limiter)
        self.gst.connect('new_level_after_limiter',
                         self.on_new_level_after_limiter)
        self.gst.connect('new_limiter_attenuation',
                         self.on_new_limiter_attenuation)
        self.gst.connect('new_level_after_reverb',
                         self.on_new_level_after_reverb)
        self.gst.connect('new_level_after_eq',
                         self.on_new_level_after_eq)
        self.gst.connect('new_spectrum', self.on_new_spectrum)

        self.limiter_user = self.settings.get_value('limiter-user').unpack()
        self.reverb_user = self.settings.get_value('reverb-user').unpack()
        self.eq_band_user = self.settings.get_value('equalizer-user').unpack()

    def do_startup(self):
        Gtk.Application.do_startup(self)

        main_ui_builder = Gtk.Builder()
        headerbar_builder = Gtk.Builder()

        main_ui_builder.add_from_file(self.module_path + '/ui/main_ui.glade')
        headerbar_builder.add_from_file(
            self.module_path + '/ui/headerbar.glade')

        main_ui_handlers = {
            'on_MainWindow_delete_event': self.on_MainWindow_delete_event,
            'on_limiter_input_gain_value_changed':
                self.on_limiter_input_gain_value_changed,
            'on_limiter_limit_value_changed':
                self.on_limiter_limit_value_changed,
            'on_limiter_release_time_value_changed':
                self.on_limiter_release_time_value_changed,
            'on_reverb_room_size_value_changed':
                self.on_reverb_room_size_value_changed,
            'on_reverb_damping_value_changed':
                self.on_reverb_damping_value_changed,
            'on_reverb_width_value_changed':
                self.on_reverb_width_value_changed,
            'on_reverb_level_value_changed':
                self.on_reverb_level_value_changed,
            'on_reverb_preset_toggled': self.on_reverb_preset_toggled,
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
            'on_eq_preset_toggled': self.on_eq_preset_toggled,
            'on_spectrum_draw': self.on_spectrum_draw
        }

        headerbar_ui_handlers = {
            'on_buffer_time_value_changed': self.on_buffer_time_value_changed
        }

        main_ui_builder.connect_signals(main_ui_handlers)
        headerbar_builder.connect_signals(headerbar_ui_handlers)

        headerbar = headerbar_builder.get_object('headerbar')

        self.window = main_ui_builder.get_object('MainWindow')
        self.window.set_titlebar(headerbar)
        self.window.set_application(self)

        self.create_appmenu()

        self.init_settings_menu(headerbar_builder)
        self.init_reverb_menu(main_ui_builder)
        self.init_equalizer_menu(main_ui_builder)

        self.apps_box = main_ui_builder.get_object('apps_box')
        self.spectrum = main_ui_builder.get_object('spectrum')

        self.build_apps_list()

        # buffer-time
        buffer_time_obj = headerbar_builder.get_object('buffer_time')

        buffer_time = self.settings.get_value('buffer-time').unpack()

        buffer_time_obj.set_value(buffer_time)

        # limiter

        self.limiter_input_gain = main_ui_builder.get_object(
            'limiter_input_gain')
        self.limiter_limit = main_ui_builder.get_object(
            'limiter_limit')
        self.limiter_release_time = main_ui_builder.get_object(
            'limiter_release_time')
        self.limiter_attenuation_levelbar = main_ui_builder.get_object(
            'limiter_attenuation_levelbar')

        self.limiter_input_gain.set_value(self.limiter_user[0])
        self.limiter_limit.set_value(self.limiter_user[1])
        self.limiter_release_time.set_value(self.limiter_user[2])

        self.limiter_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_LOW', 20)
        self.limiter_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_HIGH', 50)
        self.limiter_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_FULL', 70)

        # reverb
        self.reverb_room_size = main_ui_builder.get_object('reverb_room_size')
        self.reverb_damping = main_ui_builder.get_object('reverb_damping')
        self.reverb_width = main_ui_builder.get_object('reverb_width')
        self.reverb_level = main_ui_builder.get_object('reverb_level')

        self.reverb_left_level = main_ui_builder.get_object(
            'reverb_left_level')
        self.reverb_right_level = main_ui_builder.get_object(
            'reverb_right_level')

        self.apply_reverb_preset(self.reverb_user)

        # equalizer

        self.eq_band0 = main_ui_builder.get_object('eq_band0')
        self.eq_band1 = main_ui_builder.get_object('eq_band1')
        self.eq_band2 = main_ui_builder.get_object('eq_band2')
        self.eq_band3 = main_ui_builder.get_object('eq_band3')
        self.eq_band4 = main_ui_builder.get_object('eq_band4')
        self.eq_band5 = main_ui_builder.get_object('eq_band5')
        self.eq_band6 = main_ui_builder.get_object('eq_band6')
        self.eq_band7 = main_ui_builder.get_object('eq_band7')
        self.eq_band8 = main_ui_builder.get_object('eq_band8')
        self.eq_band9 = main_ui_builder.get_object('eq_band9')

        self.apply_eq_preset(self.eq_band_user)

        # equalizer vu meter

        self.eq_left_level = main_ui_builder.get_object('eq_left_level')
        self.eq_right_level = main_ui_builder.get_object('eq_right_level')
        self.limiter_level_before_left = main_ui_builder.get_object(
            'limiter_level_before_left')
        self.limiter_level_before_right = main_ui_builder.get_object(
            'limiter_level_before_right')
        self.limiter_level_after_left = main_ui_builder.get_object(
            'limiter_level_after_left')
        self.limiter_level_after_right = main_ui_builder.get_object(
            'limiter_level_after_right')

        # now that all elements were initialized we set pipeline to ready
        self.gst.set_state('ready')

    def do_activate(self):
        self.window.present()

        self.ui_initialized = True

    def on_MainWindow_delete_event(self, event, data):
        self.gst.set_state('null')

        self.pm.unload_sink()

        self.quit()

    def create_appmenu(self):
        menu = Gio.Menu()

        menu.append('About', 'app.about')
        menu.append('Quit', 'app.quit')

        self.set_app_menu(menu)

        about_action = Gio.SimpleAction.new('about', None)
        about_action.connect('activate', self.onAbout)
        self.add_action(about_action)

        quit_action = Gio.SimpleAction.new('quit', None)
        quit_action.connect('activate', self.on_MainWindow_delete_event)
        self.add_action(quit_action)

    def init_settings_menu(self, builder):
        button = builder.get_object('settings_popover_button')
        menu = builder.get_object('settings_menu')

        popover = Gtk.Popover.new(button)
        popover.props.transitions_enabled = True
        popover.add(menu)

        def button_clicked(arg):
            if popover.get_visible():
                popover.hide()
            else:
                popover.show_all()

        button.connect("clicked", button_clicked)

    def init_reverb_menu(self, builder):
        button = builder.get_object('reverb_popover')
        menu = builder.get_object('reverb_menu')
        reverb_none = builder.get_object('reverb_none')

        popover = Gtk.Popover.new(button)
        popover.props.transitions_enabled = True
        popover.add(menu)

        def button_clicked(arg):
            if popover.get_visible():
                popover.hide()
            else:
                popover.show_all()
                reverb_none.set_active(True)
                reverb_none.hide()

        button.connect("clicked", button_clicked)

    def init_equalizer_menu(self, builder):
        button = builder.get_object('equalizer_popover')
        menu = builder.get_object('equalizer_menu')
        eq_none = builder.get_object('eq_none')

        popover = Gtk.Popover.new(button)
        popover.props.transitions_enabled = True
        popover.add(menu)

        def button_clicked(arg):
            if popover.get_visible():
                popover.hide()
            else:
                popover.show_all()
                eq_none.set_active(True)
                eq_none.hide()

        button.connect("clicked", button_clicked)

    def on_buffer_time_value_changed(self, obj):
        value = obj.get_value()

        out = GLib.Variant('i', value)
        self.settings.set_value('buffer-time', out)

        self.gst.set_buffer_time(value * 1000)

    def build_apps_list(self):
        children = self.apps_box.get_children()

        for child in children:
            self.apps_box.remove(child)

        sink_inputs = self.pm.sink_inputs

        for i in sink_inputs:
            idx = i[0]
            app_name = i[1]
            media_name = i[2]
            icon_name = i[3]
            connected = i[4]

            row = Gtk.ListBoxRow()

            hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=2)

            row.add(hbox)

            # app icon
            icon_size = Gtk.IconSize.BUTTON
            icon = Gtk.Image.new_from_icon_name(icon_name, icon_size)

            hbox.pack_start(icon, False, False, 0)

            # label
            label_text = '<b>' + app_name + '</b>' + ': ' + media_name

            label = Gtk.Label(label_text, xalign=0)
            label.set_use_markup(True)

            hbox.pack_start(label, True, True, 0)

            # switch
            switch = Gtk.Switch()

            switch.set_active(connected)
            switch.set_name('switch_' + str(idx))

            def move_sink_input(obj, state):
                idx = int(obj.get_name().split('_')[1])

                if state:
                    self.pm.grab_input(idx)
                else:
                    self.pm.move_input_to_default_sink(idx)

            switch.connect('state-set', move_sink_input)

            hbox.pack_end(switch, False, False, 0)

            self.apps_box.add(row)

        self.apps_box.show_all()

    def on_spectrum_draw(self, drawing_area, ctx):
        ctx.paint()

        if self.spectrum_magnitudes:
            width = drawing_area.get_allocation().width
            height = drawing_area.get_allocation().height
            n_bars = len(self.spectrum_magnitudes)
            style = drawing_area.get_style_context()

            dx = width / n_bars

            for n in range(n_bars):
                mag = self.spectrum_magnitudes[n]

                if mag > 0:
                    bar_height = self.spectrum_magnitudes[n] * 1.5

                    ctx.rectangle(n * dx, height - bar_height, dx, bar_height)

            color = style.lookup_color('theme_selected_bg_color')[1]
            ctx.set_source_rgba(color.red, color.green, color.blue, 1.0)
            ctx.stroke()

    def on_sink_inputs_changed(self, obj):
        if self.ui_initialized:
            self.build_apps_list()

        if len(self.pm.sink_inputs) > 0:
            self.gst.set_state('playing')
        else:
            self.gst.set_state('paused')

    def on_new_level_before_limiter(self, obj, left, right):
        if self.ui_initialized:
            l_value = 10**(left / 20)
            r_value = 10**(right / 20)

            self.limiter_level_before_left.set_value(l_value)
            self.limiter_level_before_right.set_value(r_value)

    def on_new_level_after_limiter(self, obj, left, right):
        if self.ui_initialized:
            l_value = 10**(left / 20)
            r_value = 10**(right / 20)

            self.limiter_level_after_left.set_value(l_value)
            self.limiter_level_after_right.set_value(r_value)

    def on_new_limiter_attenuation(self, obj, attenuation):
        if self.ui_initialized:
            self.limiter_attenuation_levelbar.set_value(attenuation)

    def on_new_level_after_reverb(self, obj, left, right):
        if self.ui_initialized:
            l_value = 10**(left / 20)
            r_value = 10**(right / 20)

            self.reverb_left_level.set_value(l_value)
            self.reverb_right_level.set_value(r_value)

    def on_new_level_after_eq(self, obj, left, right):
        if self.ui_initialized:
            l_value = 10**(left / 20)
            r_value = 10**(right / 20)

            self.eq_left_level.set_value(l_value)
            self.eq_right_level.set_value(r_value)

    def on_new_spectrum(self, obj, magnitudes):
        self.spectrum_magnitudes = magnitudes

        self.spectrum.queue_draw()

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

    def apply_reverb_preset(self, values):
        self.reverb_room_size.set_value(values[0])
        self.reverb_damping.set_value(values[1])
        self.reverb_width.set_value(values[2])
        self.reverb_level.set_value(values[3])

    def on_reverb_preset_toggled(self, obj):
        if obj.get_active():
            label = obj.get_label()

            if label == 'cathedral':
                value = self.settings.get_value('reverb-cathedral')
                self.apply_reverb_preset(value)
            elif label == 'default':
                value = self.settings.get_value('reverb-default')
                self.apply_reverb_preset(value)
            elif label == 'engine room':
                value = self.settings.get_value('reverb-engine-room')
                self.apply_reverb_preset(value)
            elif label == 'small room':
                value = self.settings.get_value('reverb-small-room')
                self.apply_reverb_preset(value)

    def save_reverb_user(self, idx, value):
        self.reverb_user[idx] = value

        out = GLib.Variant('ad', self.reverb_user)

        self.settings.set_value('reverb-user', out)

    def on_reverb_room_size_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_reverb_room_size(value)
        self.save_reverb_user(0, value)

    def on_reverb_damping_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_reverb_damping(value)
        self.save_reverb_user(1, value)

    def on_reverb_width_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_reverb_width(value)
        self.save_reverb_user(2, value)

    def on_reverb_level_value_changed(self, obj):
        value = obj.get_value()
        self.gst.set_reverb_level(value)
        self.save_reverb_user(3, value)

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

    def on_eq_preset_toggled(self, obj):
        if obj.get_active():
            label = obj.get_label()

            if label == 'ballad':
                value = self.settings.get_value('equalizer-ballad')
                self.apply_eq_preset(value)
            elif label == 'classic':
                value = self.settings.get_value('equalizer-classic')
                self.apply_eq_preset(value)
            elif label == 'club':
                value = self.settings.get_value('equalizer-club')
                self.apply_eq_preset(value)
            elif label == 'dance':
                value = self.settings.get_value('equalizer-dance')
                self.apply_eq_preset(value)
            elif label == 'flat':
                value = self.settings.get_value('equalizer-flat')
                self.apply_eq_preset(value)
            elif label == 'default':
                value = self.settings.get_value('equalizer-default')
                self.apply_eq_preset(value)
            elif label == 'more bass':
                value = self.settings.get_value('equalizer-more-bass')
                self.apply_eq_preset(value)
            elif label == 'more bass and treble':
                value = self.settings.get_value(
                    'equalizer-more-bass-and-treble')
                self.apply_eq_preset(value)
            elif label == 'more treble':
                value = self.settings.get_value('equalizer-more-treble')
                self.apply_eq_preset(value)
            elif label == 'party':
                value = self.settings.get_value('equalizer-party')
                self.apply_eq_preset(value)
            elif label == 'pop':
                value = self.settings.get_value('equalizer-pop')
                self.apply_eq_preset(value)
            elif label == 'reggae':
                value = self.settings.get_value('equalizer-reggae')
                self.apply_eq_preset(value)
            elif label == 'rock':
                value = self.settings.get_value('equalizer-rock')
                self.apply_eq_preset(value)
            elif label == 'ska':
                value = self.settings.get_value('equalizer-ska')
                self.apply_eq_preset(value)
            elif label == 'soft':
                value = self.settings.get_value('equalizer-soft')
                self.apply_eq_preset(value)
            elif label == 'techno':
                value = self.settings.get_value('equalizer-techno')
                self.apply_eq_preset(value)

    def save_eq_user(self, idx, value):
        self.eq_band_user[idx] = value

        out = GLib.Variant('ad', self.eq_band_user)

        self.settings.set_value('equalizer-user', out)

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

    def onAbout(self, action, parameter):
        builder = Gtk.Builder()

        builder.add_from_file(self.module_path + '/ui/about.glade')

        dialog = builder.get_object('about_dialog')

        dialog.set_transient_for(self.window)

        dialog.show()
