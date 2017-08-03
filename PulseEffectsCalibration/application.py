# -*- coding: utf-8 -*-

import logging
import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GLib, Gtk

from PulseEffectsCalibration.microphone_pipeline import MicrophonePipeline
from PulseEffectsCalibration.setup_equalizer import SetupEqualizer
from PulseEffectsCalibration.spectrum import Spectrum


class Application(Gtk.Application):

    def __init__(self):
        app_id = 'com.github.wwmm.pulseeffects_calibration'

        GLib.set_application_name('PulseEffectsCalibration')
        GLib.setenv('PULSE_PROP_media.role', 'production', True)
        GLib.setenv('PULSE_PROP_application.icon_name',
                    'pulseeffects_calibration', True)

        Gtk.Application.__init__(self, application_id=app_id)

        self.ui_initialized = False
        self.module_path = os.path.dirname(__file__)

        log_format = '%(asctime)s.%(msecs)d - %(name)s - %(levelname)s'
        log_format = log_format + ' - %(message)s'

        logging.basicConfig(format=log_format,
                            datefmt='%H:%M:%S',
                            level=logging.INFO)

        self.log = logging.getLogger('PulseEffectsCalibration')

        self.mp = MicrophonePipeline()

    def do_startup(self):
        Gtk.Application.do_startup(self)

        self.builder = Gtk.Builder()
        self.calibration_mic_builder = Gtk.Builder()

        self.builder.add_from_file(self.module_path + '/ui/main_ui.glade')
        self.builder.add_from_file(self.module_path + '/ui/headerbar.glade')
        self.calibration_mic_builder.add_from_file(
            self.module_path + '/ui/calibration_mic_plugins.glade')

        headerbar = self.builder.get_object('headerbar')

        self.window = self.builder.get_object('MainWindow')
        self.window.set_titlebar(headerbar)
        self.window.set_application(self)

        # app menu

        self.create_appmenu()

        # main window handlers

        self.spectrum = Spectrum(self)

        main_ui_handlers = {
            'on_MainWindow_delete_event': self.on_MainWindow_delete_event
        }

        main_ui_handlers.update(self.spectrum.handlers)

        self.builder.connect_signals(main_ui_handlers)

        self.setup_equalizer = SetupEqualizer(self.calibration_mic_builder,
                                              self.mp)

        # init stack widgets
        self.init_stack_widgets()

        self.mp.connect('new_spectrum',
                        self.spectrum
                        .on_new_spectrum)

        self.setup_equalizer.connect_signals()

        self.mp.set_state('playing')

    def do_activate(self):
        self.window.present()

        self.ui_initialized = True

    def on_MainWindow_delete_event(self, event, data):
        self.mp.set_state('null')

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

    def init_stack_widgets(self):
        stack_switcher = self.builder.get_object('stack_switcher')
        stack_box = self.builder.get_object('stack_box')

        calibration_mic_ui = self.calibration_mic_builder.get_object('window')

        stack = Gtk.Stack()
        stack.set_transition_type(Gtk.StackTransitionType.CROSSFADE)
        stack.set_transition_duration(250)
        stack.set_homogeneous(False)

        stack.add_named(calibration_mic_ui, 'calibration_mic')

        stack.child_set_property(calibration_mic_ui, 'icon-name',
                                 'audio-speakers-symbolic')

        def on_visible_child_changed(stack, visible_child):
            pass

        stack.connect("notify::visible-child", on_visible_child_changed)

        stack_switcher.set_stack(stack)

        stack_box.pack_start(stack, True, True, 0)
        stack_box.show_all()

    def onAbout(self, action, parameter):
        builder = Gtk.Builder()

        builder.add_from_file(self.module_path + '/ui/about.glade')

        dialog = builder.get_object('about_dialog')

        dialog.set_transient_for(self.window)

        dialog.run()

        dialog.destroy()
