# -*- coding: utf-8 -*-

import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst

Gst.init(None)


class TestSignal():

    def __init__(self):
        self.pipeline = self.build_pipeline()

        # Create bus to get events from GStreamer pipeline
        bus = self.pipeline.get_bus()
        bus.add_signal_watch()
        bus.connect('message', self.on_message)

    def build_pipeline(self):
        pipeline = Gst.Pipeline()

        self.audio_src = Gst.ElementFactory.make('audiotestsrc', None)
        self.bandpass = Gst.ElementFactory.make('audiowsincband', None)
        self.audio_sink = Gst.ElementFactory.make('pulsesink', None)

        self.audio_src.set_property('wave', 'pink-noise')
        self.audio_src.set_property('volume', 1.0)

        self.bandpass.set_property('length', 20001)

        self.audio_sink.set_property('device', 'PulseEffects')

        pipeline.add(self.audio_src)
        pipeline.add(self.bandpass)
        pipeline.add(self.audio_sink)

        self.audio_src.link(self.bandpass)
        self.bandpass.link(self.audio_sink)
        self.audio_src.link(self.audio_sink)

        return pipeline

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

    def set_freq(self, value):
        self.audio_src.set_property('freq', value)

    def set_bandpass(self, lower, upper):
        current_lower = self.bandpass.get_property('lower-frequency')
        current_upper = self.bandpass.get_property('upper-frequency')

        print(current_lower, lower)
        print(current_upper, upper)

        if lower > current_upper:
            self.bandpass.set_property('upper-frequency', upper)
            self.bandpass.set_property('lower-frequency', lower)
        else:
            self.bandpass.set_property('lower-frequency', lower)
            self.bandpass.set_property('upper-frequency', upper)
