# -*- coding: utf-8 -*-

import gi
gi.require_version('Gst', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gst, Gtk

Gst.init(None)


class TestSignal():

    def __init__(self, app):
        self.app = app
        self.builder = app.builder

        self.handlers = {
            'on_test_signal_switch_state_set':
            self.on_test_signal_switch_state_set,
            'on_test_signal_freq_toggled': self.on_test_signal_freq_toggled,
        }

        self.pipeline = self.build_pipeline()

        # Create bus to get events from GStreamer pipeline
        bus = self.pipeline.get_bus()
        bus.add_signal_watch()
        bus.connect('message', self.on_message)

    def build_pipeline(self):
        pipeline = Gst.Pipeline()

        self.audio_src1 = Gst.ElementFactory.make('audiotestsrc', None)
        self.audio_src2 = Gst.ElementFactory.make('audiotestsrc', None)
        mixer = Gst.ElementFactory.make('audiomixer', None)
        self.bandpass = Gst.ElementFactory.make('audiochebband', None)
        self.audio_sink = Gst.ElementFactory.make('pulsesink', None)

        self.init_elements()

        pipeline.add(self.audio_src1)
        pipeline.add(self.audio_src2)
        pipeline.add(mixer)
        pipeline.add(self.bandpass)
        pipeline.add(self.audio_sink)

        self.audio_src1.link(mixer)
        mixer.link(self.bandpass)
        self.bandpass.link(self.audio_sink)

        self.audio_src2.link(mixer)

        return pipeline

    def init(self):
        self.init_menu()

    def init_elements(self):
        self.audio_sink.set_property('device', 'PulseEffects')
        self.audio_sink.set_property('volume', 1.0)
        self.audio_sink.set_property('mute', False)

        self.audio_src1.set_property('wave', 'sine')
        self.audio_src2.set_property('wave', 'sine')

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
    def set_freq(self, amp, lower, upper):
        self.set_state('null')

        self.init_elements()

        self.audio_src1.set_property('volume', 10**(-24 / 20) / amp**(0.5))
        self.audio_src2.set_property('volume', 10**(-24 / 20) / amp**(0.5))

        self.audio_src1.set_property('freq', lower)
        self.audio_src2.set_property('freq', upper)

        current_bandpass_upper = self.bandpass.get_property('upper-frequency')

        if lower > current_bandpass_upper:
            self.bandpass.set_property('upper-frequency', upper)
            self.bandpass.set_property('lower-frequency', lower)
        else:
            self.bandpass.set_property('lower-frequency', lower)
            self.bandpass.set_property('upper-frequency', upper)

        if self.app.generating_test_signal:
            self.set_state('playing')

    def init_menu(self):
        button = self.builder.get_object('test_signal_popover')
        menu = self.builder.get_object('test_signal_menu')
        default = self.builder.get_object('test_signal_band5')

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

    def on_test_signal_switch_state_set(self, obj, state):
        if state:
            self.app.generating_test_signal = True
            self.init_elements()
            self.set_state('playing')
        else:
            self.app.generating_test_signal = False
            self.set_state('null')

    def on_test_signal_freq_toggled(self, obj):
        if obj.get_active():
            obj_id = Gtk.Buildable.get_name(obj)

            if obj_id == 'test_signal_band0':
                self.set_freq(0.03, 28, 30)
            elif obj_id == 'test_signal_band1':
                self.set_freq(0.06, 58, 60)
            elif obj_id == 'test_signal_band2':
                self.set_freq(0.125, 118, 120)
            elif obj_id == 'test_signal_band3':
                self.set_freq(0.25, 236, 238)
            elif obj_id == 'test_signal_band4':
                self.set_freq(0.5, 473, 475)
            elif obj_id == 'test_signal_band5':
                self.set_freq(1, 946, 948)
            elif obj_id == 'test_signal_band6':
                self.set_freq(2, 1888, 1890)
            elif obj_id == 'test_signal_band7':
                self.set_freq(4, 3769, 3771)
            elif obj_id == 'test_signal_band8':
                self.set_freq(8, 7522, 7524)
            elif obj_id == 'test_signal_band9':
                self.set_freq(16, 15010, 15012)
