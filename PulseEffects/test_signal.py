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

        self.audio_src1 = Gst.ElementFactory.make('audiotestsrc', None)
        self.audio_src2 = Gst.ElementFactory.make('audiotestsrc', None)
        adder = Gst.ElementFactory.make('adder', None)
        self.audio_sink = Gst.ElementFactory.make('pulsesink', None)

        self.audio_sink.set_property('device', 'PulseEffects')

        self.audio_src1.set_property('wave', 'sine')
        self.audio_src2.set_property('wave', 'sine')

        # self.audio_src1.set_property('volume', 10**(-48.0 / 20.0))
        # self.audio_src2.set_property('volume', 10**(-48.0 / 20.0))

        pipeline.add(self.audio_src1)
        pipeline.add(self.audio_src2)
        pipeline.add(adder)
        pipeline.add(self.audio_sink)

        self.audio_src1.link(adder)
        adder.link(self.audio_sink)

        self.audio_src2.link(adder)

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

    def set_freq(self, amp, lower, upper):
        self.audio_src1.set_property('volume', 10**(-48 / 20) / amp**(0.5))
        self.audio_src2.set_property('volume', 10**(-48 / 20) / amp**(0.5))

        self.audio_src1.set_property('freq', lower)
        self.audio_src2.set_property('freq', upper)
