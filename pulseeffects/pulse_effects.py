#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, Gtk

from gst import GstEffects
from pulse_manager import PulseManager


class PulseEffects(Gtk.Application):
    """Main class."""

    def __init__(self):
        app_id = 'com.github.wwmm.PulseEffects'

        Gtk.Application.__init__(self, application_id=app_id)

        self.ui_initialized = False
        self.spectrum_magnitudes = []

        self.pm = PulseManager()

        self.pm.connect('sink_inputs_changed', self.on_sink_inputs_changed)

        self.gst = GstEffects()

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

        # default values

        # gstreamer equalizer-10bands
        # 29Hz,59Hz,119Hz,237Hz,474Hz,947Hz,1889Hz,3770Hz,7523Hz,15011Hz
        self.eq_bands_current = [12, 12, 12, 0, 0, 0, 0, 2, 4, 6]
        self.eq_bands_default = [12, 10, 8, 0, 0, 0, 0, 2, 4, 6]
        self.eq_bands_ballad = [4, 3.75, 2.5, 0, -4, -6, -3, 0, 2.5, 9]
        self.eq_bands_classic = [0, 0, 0, 0, 0, 0, -6, -7, -7, -9.5]
        self.eq_bands_club = [0, 0, 8, 6, 5.5, 5, 3, 0, 0, 0]
        self.eq_bands_dance = [9.6, 7, 2.5, 0, 0, -5.6, -7, -7, 0, 0]
        self.eq_bands_pop = [-1.6, 4.5, 7, 8, 5.6, 0, -2.5, -2, -1.6, -1.5]
        self.eq_bands_reggae = [0, 0, 0, -5.5, 0, 6.5, 6.5, 0, 0, 0]
        self.eq_bands_rock = [8, 5, -5.5, -8, -3, 4, 8, 11, 11, 11.5]
        self.eq_bands_ska = [-2.5, -5, -4, 0, 4, 5.5, 8, 9, 11, 9]
        self.eq_bands_soft = [5, 1.5, 0, -2.5, 0, 4, 8, 9, 11, 12]
        self.eq_bands_techno = [8, 5.5, 0, -5.5, -5, 0, 8, 10, 10, 9]
        self.eq_bands_party = [7, 7, 0, 0, 0, 0, 0, 0, 7, 7]
        self.eq_bands_more_bass = [-8, 10, 10, 5.5, 1.5, -4, -8, -10, -11, -11]
        self.eq_bands_more_bass_and_treble = [8, 5.5, 0, -7, -5, 1.5, 8, 11.2,
                                              12, 12]
        self.eq_bands_more_treble = [-10, -10, -10, -4, 2.5, 11, 16, 16, 16,
                                     18]

        # fast lookahead limiter
        # input-gain,limit,release-time
        self.compressor_default = [0, -10, 1.0]

        # freeverb
        # room-size,damping,width,level
        self.reverb_default = [0.5, 0.2, 1, 0.5]
        self.reverb_cathedral = [0.85, 0, 1.0, 0.5]
        self.reverb_engine_room = [1.0, 0.25, 1.0, 1.0]
        self.reverb_small_room = [0.8, 0.25, 0.3, 0.3]

    def do_startup(self):
        Gtk.Application.do_startup(self)

        main_ui_builder = Gtk.Builder()
        headerbar_builder = Gtk.Builder()

        main_ui_builder.add_from_file('ui/main_ui.glade')
        headerbar_builder.add_from_file('ui/headerbar.glade')

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

        main_ui_builder.connect_signals(main_ui_handlers)

        headerbar = headerbar_builder.get_object('headerbar')

        self.window = main_ui_builder.get_object('MainWindow')
        self.window.set_titlebar(headerbar)
        self.window.set_application(self)

        self.create_appmenu()

        self.init_reverb_menu(main_ui_builder)
        self.init_equalizer_menu(main_ui_builder)

        self.apps_box = main_ui_builder.get_object('apps_box')
        self.spectrum = main_ui_builder.get_object('spectrum')

        self.build_apps_list()

        # limiter

        self.limiter_input_gain = main_ui_builder.get_object(
            'limiter_input_gain')
        self.limiter_limit = main_ui_builder.get_object(
            'limiter_limit')
        self.limiter_release_time = main_ui_builder.get_object(
            'limiter_release_time')
        self.limiter_attenuation_levelbar = main_ui_builder.get_object(
            'limiter_attenuation_levelbar')

        self.limiter_input_gain.set_value(self.compressor_default[0])
        self.limiter_limit.set_value(self.compressor_default[1])
        self.limiter_release_time.set_value(self.compressor_default[2])

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
        self.reverb_custom = main_ui_builder.get_object('reverb_custom')

        self.reverb_left_level = main_ui_builder.get_object(
            'reverb_left_level')
        self.reverb_right_level = main_ui_builder.get_object(
            'reverb_right_level')

        self.apply_reverb_preset(self.reverb_default)
        self.reverb_custom.set_active(True)

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
        self.eq_custom = main_ui_builder.get_object('eq_custom')

        self.apply_eq_preset(self.eq_bands_default)
        self.eq_custom.set_active(True)

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

    def do_activate(self):
        self.window.present()

        self.ui_initialized = True

    def on_MainWindow_delete_event(self, event, data):
        self.gst.stop()

        self.pm.unload_sink()

        self.quit()

    def init_reverb_menu(self, builder):
        button = builder.get_object('reverb_popover')
        menu = builder.get_object('reverb_menu')

        popover = Gtk.Popover.new(button)
        popover.props.transitions_enabled = True
        popover.add(menu)

        def button_clicked(arg):
            if popover.get_visible():
                popover.hide()
            else:
                popover.show_all()

        button.connect("clicked", button_clicked)

    def init_equalizer_menu(self, builder):
        button = builder.get_object('equalizer_popover')
        menu = builder.get_object('equalizer_menu')

        popover = Gtk.Popover.new(button)
        popover.props.transitions_enabled = True
        popover.add(menu)

        def button_clicked(arg):
            if popover.get_visible():
                popover.hide()
            else:
                popover.show_all()

        button.connect("clicked", button_clicked)

    def on_drift_tolerance_value_changed(self, obj):
        self.gst.set_drift_tolerance(obj.get_value())

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
                    bar_height = self.spectrum_magnitudes[n] * height

                    ctx.rectangle(n * dx, height - bar_height, dx, bar_height)

            color = style.lookup_color('theme_selected_bg_color')[1]
            ctx.set_source_rgba(color.red, color.green, color.blue, 1.0)
            ctx.stroke()

    def on_sink_inputs_changed(self, obj):
        if self.ui_initialized:
            self.build_apps_list()

        if len(self.pm.sink_inputs) > 0:
            self.gst.play()
        else:
            self.gst.pause()

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

    def on_limiter_input_gain_value_changed(self, obj):
        self.gst.set_limiter_input_gain(obj.get_value())

    def on_limiter_limit_value_changed(self, obj):
        self.gst.set_limiter_limit(obj.get_value())

    def on_limiter_release_time_value_changed(self, obj):
        self.gst.set_limiter_release_time(obj.get_value())

    def apply_reverb_preset(self, values):
        self.reverb_room_size.set_value(values[0])
        self.reverb_damping.set_value(values[1])
        self.reverb_width.set_value(values[2])
        self.reverb_level.set_value(values[3])

    def on_reverb_preset_toggled(self, obj):
        if obj.get_active():
            label = obj.get_label()

            if label == 'cathedral':
                self.apply_reverb_preset(self.reverb_cathedral)
            elif label == 'custom':
                self.apply_reverb_preset(self.reverb_default)
            elif label == 'engine room':
                self.apply_reverb_preset(self.reverb_engine_room)
            elif label == 'small room':
                self.apply_reverb_preset(self.reverb_small_room)

    def on_reverb_room_size_value_changed(self, obj):
        self.gst.set_reverb_room_size(obj.get_value())

    def on_reverb_damping_value_changed(self, obj):
        self.gst.set_reverb_damping(obj.get_value())

    def on_reverb_width_value_changed(self, obj):
        self.gst.set_reverb_width(obj.get_value())

    def on_reverb_level_value_changed(self, obj):
        self.gst.set_reverb_level(obj.get_value())

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
                self.apply_eq_preset(self.eq_bands_ballad)
            elif label == 'classic':
                self.apply_eq_preset(self.eq_bands_classic)
            elif label == 'club':
                self.apply_eq_preset(self.eq_bands_club)
            elif label == 'custom':
                self.apply_eq_preset(self.eq_bands_default)
            elif label == 'dance':
                self.apply_eq_preset(self.eq_bands_dance)
            elif label == 'more bass':
                self.apply_eq_preset(self.eq_bands_more_bass)
            elif label == 'more bass and treble':
                self.apply_eq_preset(self.eq_bands_more_bass_and_treble)
            elif label == 'more treble':
                self.apply_eq_preset(self.eq_bands_more_treble)
            elif label == 'party':
                self.apply_eq_preset(self.eq_bands_party)
            elif label == 'pop':
                self.apply_eq_preset(self.eq_bands_pop)
            elif label == 'reggae':
                self.apply_eq_preset(self.eq_bands_reggae)
            elif label == 'rock':
                self.apply_eq_preset(self.eq_bands_rock)
            elif label == 'ska':
                self.apply_eq_preset(self.eq_bands_ska)
            elif label == 'soft':
                self.apply_eq_preset(self.eq_bands_soft)
            elif label == 'techno':
                self.apply_eq_preset(self.eq_bands_techno)

    def on_eq_band0_value_changed(self, obj):
        self.gst.set_eq_band0(obj.get_value())

    def on_eq_band1_value_changed(self, obj):
        self.gst.set_eq_band1(obj.get_value())

    def on_eq_band2_value_changed(self, obj):
        self.gst.set_eq_band2(obj.get_value())

    def on_eq_band3_value_changed(self, obj):
        self.gst.set_eq_band3(obj.get_value())

    def on_eq_band4_value_changed(self, obj):
        self.gst.set_eq_band4(obj.get_value())

    def on_eq_band5_value_changed(self, obj):
        self.gst.set_eq_band5(obj.get_value())

    def on_eq_band6_value_changed(self, obj):
        self.gst.set_eq_band6(obj.get_value())

    def on_eq_band7_value_changed(self, obj):
        self.gst.set_eq_band7(obj.get_value())

    def on_eq_band8_value_changed(self, obj):
        self.gst.set_eq_band8(obj.get_value())

    def on_eq_band9_value_changed(self, obj):
        self.gst.set_eq_band9(obj.get_value())

    def onAbout(self, action, parameter):
        builder = Gtk.Builder()

        builder.add_from_file('ui/about.glade')

        dialog = builder.get_object('about_dialog')

        dialog.set_transient_for(self.window)

        dialog.show()


if __name__ == '__main__':
    w = PulseEffects()

    exit_status = w.run(sys.argv)

    sys.exit(exit_status)
