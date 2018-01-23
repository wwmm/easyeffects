# -*- coding: utf-8 -*-

import logging
import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk

from PulseEffectsTest.microphone import Microphone
from PulseEffectsTest.setup_test_signal import SetupTestSignal
from PulseEffectsTest.spectrum import Spectrum
from PulseEffectsTest.test_signal import TestSignal


class Application(Gtk.Application):

    def __init__(self, pulse_manager):
        app_id = 'com.github.wwmm.pulseeffects.test'
        self.pm = pulse_manager

        Gtk.Application.__init__(self, application_id=app_id)

    def do_startup(self):
        Gtk.Application.do_startup(self)

        self.ui_initialized = False
        self.module_path = os.path.dirname(__file__)

        log_format = '%(asctime)s.%(msecs)d - %(name)s - %(levelname)s'
        log_format = log_format + ' - %(message)s'

        logging.basicConfig(format=log_format,
                            datefmt='%H:%M:%S',
                            level=logging.INFO)

        self.log = logging.getLogger('PulseEffectsTest')

        self.mic = Microphone(48000)
        self.ts = TestSignal(48000)

        self.mic.set_source_monitor_name(self.pm.default_source_name)

        self.mic.set_state('ready')
        self.ts.set_state('ready')

        self.builder = Gtk.Builder()
        self.test_signal_builder = Gtk.Builder()

        self.builder.add_from_file(self.module_path + '/ui/main_ui.glade')
        self.test_signal_builder.add_from_file(self.module_path +
                                               '/ui/test_signal.glade')

        self.window = self.builder.get_object('MainWindow')
        self.window.set_application(self)

        # main window handlers

        self.spectrum = Spectrum(self)

        main_ui_handlers = {}

        main_ui_handlers.update(self.spectrum.handlers)

        self.builder.connect_signals(main_ui_handlers)

        # init test signal widgets
        self.setup_test_signal = SetupTestSignal(self.test_signal_builder,
                                                 self.ts)

        # init stack widgets
        self.init_stack_widgets()

        # other initializations

        self.pm.connect('new_default_source', self.update_source_monitor_name)
        self.mic.connect('new_guideline_position',
                         self.spectrum.set_guideline_position)

        self.mic.set_state('playing')

    def do_activate(self):
        self.window.present()

        self.ui_initialized = True

    def do_shutdown(self):
        Gtk.Application.do_shutdown(self)

        self.mic.set_state('null')
        self.ts.set_state('null')

    def init_stack_widgets(self):
        stack = self.builder.get_object('stack')

        test_signal_ui = self.test_signal_builder.get_object('window')

        stack.add_named(test_signal_ui, "test_signal")
        stack.child_set_property(test_signal_ui, 'icon-name',
                                 'pulseeffects-sine-symbolic')

        stack.add_named(self.mic.ui_window, 'microphone')
        stack.child_set_property(self.mic.ui_window, 'icon-name',
                                 'audio-input-microphone-symbolic')

        self.stack_current_child_name = 'test_signal'

        self.spectrum_handler_id = self.ts.connect('new_spectrum',
                                                   self.spectrum
                                                   .on_new_spectrum)

        def on_visible_child_changed(stack, visible_child):
            name = stack.get_visible_child_name()

            if name == 'test_signal':
                if self.stack_current_child_name == 'microphone':
                    self.mic.disconnect(self.spectrum_handler_id)

                self.spectrum_handler_id = self.ts.connect('new_spectrum',
                                                           self.spectrum
                                                           .on_new_spectrum)

                self.spectrum.draw_guideline = False

                self.stack_current_child_name = 'test_signal'
            elif name == 'microphone':
                if self.stack_current_child_name == 'test_signal':
                    self.ts.disconnect(self.spectrum_handler_id)

                self.spectrum_handler_id = self.mic.connect('new_spectrum',
                                                            self.spectrum
                                                            .on_new_spectrum)

                self.spectrum.draw_guideline = True

                self.stack_current_child_name = 'microphone'

            self.spectrum.clear()

        stack.connect("notify::visible-child", on_visible_child_changed)

    def update_source_monitor_name(self, obj, name):
        self.mic.set_source_monitor_name(name)
