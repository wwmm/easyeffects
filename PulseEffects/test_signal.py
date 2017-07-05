# -*- coding: utf-8 -*-
import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gst, Gtk

Gst.init(None)


class TestSignal():

    def __init__(self, app_builder, sie_effects, list_sink_inputs):
        self.app_builder = app_builder
        self.sie_effects = sie_effects
        self.list_sink_inputs = list_sink_inputs
        self.module_path = os.path.dirname(__file__)

        self.pipeline = self.build_pipeline()
        self.switch_is_on = False

        # Create bus to get events from GStreamer pipeline
        bus = self.pipeline.get_bus()
        bus.add_signal_watch()
        bus.connect('message', self.on_message)

    def build_pipeline(self):
        pipeline = Gst.Pipeline()

        self.audio_src1 = Gst.ElementFactory.make('audiotestsrc', None)
        self.audio_src2 = Gst.ElementFactory.make('audiotestsrc', None)
        self.audio_src3 = Gst.ElementFactory.make('audiotestsrc', None)
        mixer = Gst.ElementFactory.make('audiomixer', None)
        self.bandpass = Gst.ElementFactory.make('audiochebband', None)
        self.audio_sink = Gst.ElementFactory.make('pulsesink', None)

        self.init_elements()

        pipeline.add(self.audio_src1)
        pipeline.add(self.audio_src2)
        pipeline.add(self.audio_src3)
        pipeline.add(mixer)
        pipeline.add(self.bandpass)
        pipeline.add(self.audio_sink)

        self.audio_src1.link(mixer)
        mixer.link(self.bandpass)
        self.bandpass.link(self.audio_sink)

        self.audio_src2.link(mixer)
        self.audio_src3.link(mixer)

        return pipeline

    def init(self):
        self.init_menu()

    def init_elements(self):
        self.audio_sink.set_property('device', 'PulseEffects_apps')
        self.audio_sink.set_property('volume', 1.0)
        self.audio_sink.set_property('mute', False)

        self.audio_src1.set_property('wave', 'sine')
        self.audio_src2.set_property('wave', 'sine')
        self.audio_src3.set_property('wave', 'sine')

        self.bandpass.set_property('mode', 'band-pass')
        self.bandpass.set_property('type', 1)
        self.bandpass.set_property('ripple', 0)
        self.bandpass.set_property('poles', 4)

    def set_state(self, state):
        if state == 'ready':
            s = self.pipeline.set_state(Gst.State.READY)

            if s == Gst.StateChangeReturn.FAILURE:
                print("Failed set PulseEffects Gstreamer pipeline to ready!!!")

                return False
            else:
                return True
        elif state == 'paused':
            s = self.pipeline.set_state(Gst.State.PAUSED)

            if s == Gst.StateChangeReturn.FAILURE:
                print("Failed to pause PulseEffects test signal pipeline!!!")

                return False
            else:
                return True
        elif state == 'playing':
            s = self.pipeline.set_state(Gst.State.PLAYING)

            if s == Gst.StateChangeReturn.FAILURE:
                print("Playing PulseEffects test signal pipeline failed!!!")

                return False
            else:
                return True
        elif state == 'null':
            s = self.pipeline.set_state(Gst.State.NULL)

            if s == Gst.StateChangeReturn.FAILURE:
                print("Stopping PulseEffects test signal pipeline failed!!!")

                return False
            else:
                return True

    def on_message(self, bus, msg):
        if msg.type == Gst.MessageType.ERROR:
            print('on_error():', msg.parse_error())

    # amp is a rescaling factor so that all frequencies have the same intensity
    def set_freq(self, amp, lower, center, upper):
        self.set_state('null')

        self.init_elements()

        self.audio_src1.set_property('volume', 1.0 / amp**(0.5))
        self.audio_src2.set_property('volume', 1.0 / amp**(0.5))
        self.audio_src3.set_property('volume', 1.0 / amp**(0.5))

        self.audio_src1.set_property('freq', lower)
        self.audio_src2.set_property('freq', center)
        self.audio_src3.set_property('freq', upper)

        current_bandpass_upper = self.bandpass.get_property('upper-frequency')

        if lower > current_bandpass_upper:
            self.bandpass.set_property('upper-frequency', upper)
            self.bandpass.set_property('lower-frequency', lower)
        else:
            self.bandpass.set_property('lower-frequency', lower)
            self.bandpass.set_property('upper-frequency', upper)

        if self.switch_is_on:
            self.set_state('playing')

    def init_menu(self):
        builder = Gtk.Builder()

        builder.add_from_file(self.module_path + '/ui/test_signal_menu.glade')

        builder.connect_signals(self)

        menu = builder.get_object('menu')
        default = builder.get_object('test_signal_band8')

        default.set_active(True)

        button = self.app_builder.get_object('test_signal_popover')

        popover = Gtk.Popover.new(button)
        popover.props.transitions_enabled = True
        popover.add(menu)

        def button_clicked(arg):
            if popover.get_visible():
                popover.hide()
            else:
                popover.show_all()

        button.connect("clicked", button_clicked)

    def on_test_signal_switch_state_set(self, obj, state):
        if state:
            self.init_elements()

            if not self.sie_effects.is_playing:
                self.sie_effects.set_state('playing')

            self.set_state('playing')
            self.switch_is_on = True
        else:
            self.switch_is_on = False
            self.set_state('null')

    def on_test_signal_freq_toggled(self, obj):
        if obj.get_active():
            obj_id = Gtk.Buildable.get_name(obj)

            if obj_id == 'test_signal_band0':
                # amplitude scaling factor, lower, center, upper
                self.set_freq(1, 23, 26, 29)
            elif obj_id == 'test_signal_band1':
                self.set_freq(1.58, 38, 41, 44)
            elif obj_id == 'test_signal_band2':
                self.set_freq(2.5, 62, 65, 68)
            elif obj_id == 'test_signal_band3':
                self.set_freq(3.96, 100, 103, 106)
            elif obj_id == 'test_signal_band4':
                self.set_freq(6.27, 159, 163, 166)
            elif obj_id == 'test_signal_band5':
                self.set_freq(9.96, 256, 259, 262)
            elif obj_id == 'test_signal_band6':
                self.set_freq(15.77, 407, 410, 413)
            elif obj_id == 'test_signal_band7':
                self.set_freq(24.96, 646, 649, 652)
            elif obj_id == 'test_signal_band8':
                self.set_freq(39.58, 1026, 1029, 1032)
            elif obj_id == 'test_signal_band9':
                self.set_freq(62.73, 1628, 1631, 1634)
            elif obj_id == 'test_signal_band10':
                self.set_freq(99.42, 2582, 2585, 2588)
            elif obj_id == 'test_signal_band11':
                self.set_freq(157.58, 4094, 4097, 4100)
            elif obj_id == 'test_signal_band12':
                self.set_freq(249.73, 6490, 6493, 6496)
            elif obj_id == 'test_signal_band13':
                self.set_freq(395.81, 10288, 10291, 10294)
            elif obj_id == 'test_signal_band14':
                self.set_freq(627.31, 16307, 16310, 16313)
