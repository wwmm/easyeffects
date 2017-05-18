# -*- coding: utf-8 -*-

import configparser
import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GLib, Gtk
from PulseEffects.gst import GstEffects
from PulseEffects.pulse_manager import PulseManager
from PulseEffects.test_signal import TestSignal

from PulseEffects.setup_limiter import SetupLimiter
from PulseEffects.setup_compressor import SetupCompressor
from PulseEffects.setup_reverb import SetupReverb


class Application(Gtk.Application):

    def __init__(self):
        app_id = 'com.github.wwmm.pulseeffects'

        Gtk.Application.__init__(self, application_id=app_id)

        self.ui_initialized = False
        self.generating_test_signal = False
        self.spectrum_magnitudes = []
        self.module_path = os.path.dirname(__file__)

        self.settings = Gio.Settings('com.github.wwmm.pulseeffects')

        # pulseaudio

        self.pm = PulseManager()

        self.pm.connect('sink_inputs_changed', self.on_sink_inputs_changed)

        # gstreamer audio effects

        self.gst = GstEffects(self.pm.default_sink_rate)

        self.gst.set_output_sink_name(self.pm.default_sink_name)

        self.gst.connect('new_level_after_eq', self.on_new_level_after_eq)
        self.gst.connect('new_spectrum', self.on_new_spectrum)

        # gstreamer test signal (sine wave)

        self.test_signal = TestSignal()

        # reading last used configurations

        self.eq_band_user = self.settings.get_value('equalizer-user').unpack()
        self.eq_preamp_user = self.settings.get_value(
            'equalizer-preamp').unpack()

        # creating user presets folder
        self.user_config_dir = os.path.expanduser('~/.config/PulseEffects')
        os.makedirs(self.user_config_dir, exist_ok=True)

    def do_startup(self):
        Gtk.Application.do_startup(self)

        self.builder = Gtk.Builder()

        ui_handlers = {
            'on_MainWindow_delete_event': self.on_MainWindow_delete_event,
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
            'on_eq_preset_toggled': self.on_eq_preset_toggled,
            'on_spectrum_draw': self.on_spectrum_draw,
            'on_test_signal_switch_state_set':
                self.on_test_signal_switch_state_set,
            'on_test_signal_freq_toggled': self.on_test_signal_freq_toggled,
            'on_buffer_time_value_changed': self.on_buffer_time_value_changed,
            'on_save_user_preset_clicked': self.on_save_user_preset_clicked,
            'on_load_user_preset_clicked': self.on_load_user_preset_clicked
        }

        self.builder.add_from_file(self.module_path + '/ui/main_ui.glade')
        self.builder.add_from_file(self.module_path + '/ui/headerbar.glade')

        headerbar = self.builder.get_object('headerbar')

        self.window = self.builder.get_object('MainWindow')
        self.window.set_titlebar(headerbar)
        self.window.set_application(self)

        self.create_appmenu()

        self.setup_limiter = SetupLimiter(self)
        self.setup_compressor = SetupCompressor(self)
        self.setup_reverb = SetupReverb(self)

        ui_handlers.update(self.setup_limiter.handlers)
        ui_handlers.update(self.setup_compressor.handlers)
        ui_handlers.update(self.setup_reverb.handlers)

        self.builder.connect_signals(ui_handlers)

        self.setup_limiter.init()
        self.setup_compressor.init()
        self.setup_reverb.init()

        self.init_settings_menu(self.builder)

        self.init_equalizer_menu(self.builder)
        self.init_test_signal_menu(self.builder)

        self.apps_box = self.builder.get_object('apps_box')
        self.spectrum = self.builder.get_object('spectrum')

        self.build_apps_list()

        # buffer-time
        buffer_time_obj = self.builder.get_object('buffer_time')

        buffer_time = self.settings.get_value('buffer-time').unpack()

        buffer_time_obj.set_value(buffer_time)

        # equalizer

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

        self.eq_left_level = self.builder.get_object('eq_left_level')
        self.eq_right_level = self.builder.get_object('eq_right_level')

        self.eq_left_level_label = self.builder.get_object(
            'eq_left_level_label')
        self.eq_right_level_label = self.builder.get_object(
            'eq_right_level_label')

        self.init_equalizer()

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

    def init_equalizer_menu(self, builder):
        button = builder.get_object('equalizer_popover')
        menu = builder.get_object('equalizer_menu')
        eq_no_selection = builder.get_object('eq_no_selection')

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

    def init_test_signal_menu(self, builder):
        button = builder.get_object('test_signal_popover')
        menu = builder.get_object('test_signal_menu')
        default = builder.get_object('test_signal_band5')

        default.set_active(True)

        popover = Gtk.Popover.new(button)
        popover.props.transitions_enabled = True
        popover.add(menu)

        def button_clicked(arg):
            if popover.get_visible():
                popover.hide()
            else:
                popover.show_all()

        button.connect("clicked", button_clicked)

    def on_buffer_time_value_changed(self, obj):
        value = obj.get_value()

        out = GLib.Variant('i', value)
        self.settings.set_value('buffer-time', out)

        if self.ui_initialized:
            self.gst.set_buffer_time(value * 1000)
        else:
            self.gst.init_buffer_time(value * 1000)

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

        if len(self.pm.sink_inputs) > 0 or self.generating_test_signal:
            self.gst.set_state('playing')
        else:
            self.gst.set_state('paused')

    def on_new_level_after_eq(self, obj, left, right):
        if self.ui_initialized:
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

    def on_new_spectrum(self, obj, magnitudes):
        self.spectrum_magnitudes = magnitudes

        self.spectrum.queue_draw()

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

    def init_equalizer(self):
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

    def on_eq_preset_toggled(self, obj):
        if obj.get_active():
            obj_id = Gtk.Buildable.get_name(obj)

            if obj_id == 'eq_ballad':
                value = self.settings.get_value('equalizer-ballad')
                self.apply_eq_preset(value)
            elif obj_id == 'eq_classic':
                value = self.settings.get_value('equalizer-classic')
                self.apply_eq_preset(value)
            elif obj_id == 'eq_club':
                value = self.settings.get_value('equalizer-club')
                self.apply_eq_preset(value)
            elif obj_id == 'eq_dance':
                value = self.settings.get_value('equalizer-dance')
                self.apply_eq_preset(value)
            elif obj_id == 'eq_equal_loudness_20':
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
            elif obj_id == 'eq_default':
                value = self.settings.get_value('equalizer-default')
                self.apply_eq_preset(value)
            elif obj_id == 'eq_more_bass_and_treble':
                value = self.settings.get_value(
                    'equalizer-more-bass-and-treble')
                self.apply_eq_preset(value)
            elif obj_id == 'eq_party':
                value = self.settings.get_value('equalizer-party')
                self.apply_eq_preset(value)
            elif obj_id == 'eq_pop':
                value = self.settings.get_value('equalizer-pop')
                self.apply_eq_preset(value)
            elif obj_id == 'eq_reggae':
                value = self.settings.get_value('equalizer-reggae')
                self.apply_eq_preset(value)
            elif obj_id == 'eq_rock':
                value = self.settings.get_value('equalizer-rock')
                self.apply_eq_preset(value)
            elif obj_id == 'eq_ska':
                value = self.settings.get_value('equalizer-ska')
                self.apply_eq_preset(value)
            elif obj_id == 'eq_soft':
                value = self.settings.get_value('equalizer-soft')
                self.apply_eq_preset(value)
            elif obj_id == 'eq_techno':
                value = self.settings.get_value('equalizer-techno')
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

    def on_test_signal_switch_state_set(self, obj, state):
        if state:
            self.generating_test_signal = True
            self.test_signal.set_state('playing')
        else:
            self.generating_test_signal = False
            self.test_signal.set_state('null')

    def on_test_signal_freq_toggled(self, obj):
        if obj.get_active():
            obj_id = Gtk.Buildable.get_name(obj)

            if obj_id == 'test_signal_band0':
                self.test_signal.set_freq(0.03, 28, 30)
            elif obj_id == 'test_signal_band1':
                self.test_signal.set_freq(0.06, 58, 60)
            elif obj_id == 'test_signal_band2':
                self.test_signal.set_freq(0.125, 118, 120)
            elif obj_id == 'test_signal_band3':
                self.test_signal.set_freq(0.25, 236, 238)
            elif obj_id == 'test_signal_band4':
                self.test_signal.set_freq(0.5, 473, 475)
            elif obj_id == 'test_signal_band5':
                self.test_signal.set_freq(1, 946, 948)
            elif obj_id == 'test_signal_band6':
                self.test_signal.set_freq(2, 1888, 1890)
            elif obj_id == 'test_signal_band7':
                self.test_signal.set_freq(4, 3769, 3771)
            elif obj_id == 'test_signal_band8':
                self.test_signal.set_freq(8, 7522, 7524)
            elif obj_id == 'test_signal_band9':
                self.test_signal.set_freq(16, 15010, 15012)

    def add_file_filter(self, dialog):
        file_filter = Gtk.FileFilter()
        file_filter.set_name("preset")
        file_filter.add_mime_type("text/plain")

        dialog.add_filter(file_filter)

    def on_save_user_preset_clicked(self, obj):
        dialog = Gtk.FileChooserDialog('', self.window,
                                       Gtk.FileChooserAction.SAVE,
                                       (Gtk.STOCK_CANCEL,
                                        Gtk.ResponseType.CANCEL,
                                        Gtk.STOCK_SAVE, Gtk.ResponseType.OK))

        dialog.set_current_folder(self.user_config_dir)
        dialog.set_current_name('user.preset')

        self.add_file_filter(dialog)

        response = dialog.run()

        if response == Gtk.ResponseType.OK:
            path = dialog.get_filename()

            if not path.endswith(".preset"):
                path += ".preset"

            output = open(path, 'w')

            config = configparser.ConfigParser()

            limiter = self.settings.get_value('limiter-user')

            config['limiter'] = {'input gain': str(limiter[0]),
                                 'limit': str(limiter[1]),
                                 'release time': str(limiter[2])}

            compressor = self.settings.get_value('compressor-user')

            config['compressor'] = {'rms-peak': str(compressor[0]),
                                    'attack': str(compressor[1]),
                                    'release': str(compressor[2]),
                                    'threshold': str(compressor[3]),
                                    'ratio': str(compressor[4]),
                                    'knee': str(compressor[5]),
                                    'makeup': str(compressor[6])}

            reverb = self.settings.get_value('reverb-user')

            config['reverb'] = {'room size': str(reverb[0]),
                                'damping': str(reverb[1]),
                                'width': str(reverb[2]),
                                'level': str(reverb[3])}

            equalizer_preamp = self.settings.get_value('equalizer-preamp')
            equalizer = self.settings.get_value('equalizer-user')

            config['equalizer'] = {'preamp': str(equalizer_preamp),
                                   'band0': str(equalizer[0]),
                                   'band1': str(equalizer[1]),
                                   'band2': str(equalizer[2]),
                                   'band3': str(equalizer[3]),
                                   'band4': str(equalizer[4]),
                                   'band5': str(equalizer[5]),
                                   'band6': str(equalizer[6]),
                                   'band7': str(equalizer[7]),
                                   'band8': str(equalizer[8]),
                                   'band9': str(equalizer[9])}

            config.write(output)

        dialog.destroy()

    def on_load_user_preset_clicked(self, obj):
        dialog = Gtk.FileChooserDialog('', self.window,
                                       Gtk.FileChooserAction.OPEN,
                                       (Gtk.STOCK_CANCEL,
                                        Gtk.ResponseType.CANCEL,
                                        Gtk.STOCK_OPEN, Gtk.ResponseType.OK))

        dialog.set_current_folder(self.user_config_dir)

        self.add_file_filter(dialog)

        response = dialog.run()

        if response == Gtk.ResponseType.OK:
            path = dialog.get_filename()

            config = configparser.ConfigParser()

            config.read(path)

            limiter = dict(config['limiter']).values()
            limiter = [float(v) for v in limiter]
            self.setup_limiter.apply_limiter_preset(limiter)

            compressor = dict(config['compressor']).values()
            compressor = [float(v) for v in compressor]
            self.setup_compressor.apply_compressor_preset(compressor)

            reverb = dict(config['reverb']).values()
            reverb = [float(v) for v in reverb]
            self.setup_reverb.apply_reverb_preset(reverb)

            equalizer = list(dict(config['equalizer']).values())

            if len(equalizer) == 11:  # one day this check will be removed...
                equalizer_preamp = float(equalizer.pop(0))
                self.eq_preamp.set_value(equalizer_preamp)

            equalizer = [float(v) for v in equalizer]
            self.apply_eq_preset(equalizer)

        dialog.destroy()

    def onAbout(self, action, parameter):
        builder = Gtk.Builder()

        builder.add_from_file(self.module_path + '/ui/about.glade')

        dialog = builder.get_object('about_dialog')

        dialog.set_transient_for(self.window)

        dialog.show()
