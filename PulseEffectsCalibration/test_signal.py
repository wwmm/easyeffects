# -*- coding: utf-8 -*-
import logging

import gi
import numpy as np
from scipy.interpolate import CubicSpline
gi.require_version('Gst', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import GObject, Gst


Gst.init(None)


class TestSignal(GObject.GObject):

    __gsignals__ = {
        'new_spectrum': (GObject.SIGNAL_RUN_FIRST, None,
                         (float, float, object))
    }

    def __init__(self):
        GObject.GObject.__init__(self)

        self.max_spectrum_freq = 20000  # Hz
        self.spectrum_nbands = 3600
        self.spectrum_freqs = []
        self.spectrum_x_axis = np.array([])
        self.spectrum_n_points = 3600  # number of freqs displayed
        self.spectrum_nfreqs = 0
        self.spectrum_threshold = -120  # dB
        self.rate = 48000

        self.log = logging.getLogger('PulseEffectsCalibration')

        self.calc_spectrum_freqs()

        self.pipeline = self.build_pipeline()

        # Create bus to get events from GStreamer pipeline
        bus = self.pipeline.get_bus()
        bus.add_signal_watch()
        bus.connect('message::error', self.on_message_error)
        bus.connect('message::info', self.on_message_info)
        bus.connect('message::warning', self.on_message_warning)
        bus.connect('message::element', self.on_message_element)

    def calc_spectrum_freqs(self):
        self.spectrum_freqs = []

        for i in range(self.spectrum_nbands):
            freq = self.rate * (0.5 * i + 0.25) / self.spectrum_nbands

            if freq > self.max_spectrum_freq:
                break

            self.spectrum_freqs.append(freq)

        self.spectrum_nfreqs = len(self.spectrum_freqs)

        self.spectrum_x_axis = np.logspace(1.3, 4.3, self.spectrum_n_points)

    def set_spectrum_n_points(self, value):
        self.spectrum_n_points = value

        self.spectrum_x_axis = np.logspace(1.3, 4.3, value)

    def enable_spectrum(self, state):
        self.spectrum.set_property('post-messages', state)

    def build_pipeline(self):
        pipeline = Gst.Pipeline()

        self.audio_src1 = Gst.ElementFactory.make('audiotestsrc', None)
        self.audio_src2 = Gst.ElementFactory.make('audiotestsrc', None)
        src1_caps = Gst.ElementFactory.make("capsfilter", None)
        src2_caps = Gst.ElementFactory.make("capsfilter", None)
        self.bandpass = Gst.ElementFactory.make('audiochebband', None)
        mixer = Gst.ElementFactory.make('audiomixer', None)
        self.spectrum = Gst.ElementFactory.make('spectrum', 'spectrum')
        self.audio_sink = Gst.ElementFactory.make('pulsesink', None)

        # self.audio_sink.set_property('device', 'PulseEffects_apps')
        self.audio_sink.set_property('volume', 1.0)
        self.audio_sink.set_property('mute', False)

        self.audio_src1.set_property('wave', 'sine')
        self.audio_src2.set_property('wave', 'pink-noise')

        self.bandpass.set_property('mode', 'band-pass')
        self.bandpass.set_property('type', 1)
        self.bandpass.set_property('poles', 4)
        self.bandpass.set_property('ripple', 0)

        self.spectrum.set_property('bands', self.spectrum_nbands)
        self.spectrum.set_property('threshold', self.spectrum_threshold)
        self.spectrum.set_property('interval', int(1 * 1000000000))

        caps = ['audio/x-raw', 'format=F32LE',
                'rate=' + str(self.rate), 'channels=2']

        src_caps = Gst.Caps.from_string(",".join(caps))

        src1_caps.set_property("caps", src_caps)
        src2_caps.set_property("caps", src_caps)

        mixer.set_property('caps', src_caps)

        pipeline.add(self.audio_src1)
        pipeline.add(self.audio_src2)
        pipeline.add(src1_caps)
        pipeline.add(src2_caps)
        pipeline.add(self.bandpass)
        pipeline.add(mixer)
        pipeline.add(self.spectrum)
        pipeline.add(self.audio_sink)

        self.audio_src1.link(src1_caps)
        src1_caps.link(mixer)
        mixer.link(self.spectrum)
        self.spectrum.link(self.audio_sink)

        self.audio_src2.link(src2_caps)
        src2_caps.link(self.bandpass)
        self.bandpass.link(mixer)

        return pipeline

    def set_state(self, state):
        if state == 'ready':
            s = self.pipeline.set_state(Gst.State.READY)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.critical("Could not set Gstreamer pipeline to ready")
                return False
            else:
                self.log.info('test signals pipeline state: ready')
                return True
        elif state == 'paused':
            s = self.pipeline.set_state(Gst.State.PAUSED)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.error("Failed to pause Gstreamer pipeline")
                return False
            else:
                self.log.info('test signals pipeline state: paused')
                return True
        elif state == 'playing':
            s = self.pipeline.set_state(Gst.State.PLAYING)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.critical("Playing Gstreamer pipeline has failed")
                return False
            else:
                self.log.info('test signals pipeline state: playing')
                return True
        elif state == 'null':
            s = self.pipeline.set_state(Gst.State.NULL)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.error("Could not stop Gstreamer pipeline")
                return False
            else:
                self.log.info('test signals pipeline state: null')
                return True

    def on_message_error(self, bus, msg):
        self.log.error(msg.parse_error())
        self.set_state('null')

        return True

    def on_message_info(self, bus, msg):
        self.log.info(msg.parse_info())

        return True

    def on_message_warning(self, bus, msg):
        self.log.warning(msg.parse_warning())

        return True

    def on_message_element(self, bus, msg):
        plugin = msg.src.get_name()

        if plugin == 'spectrum':
            magnitudes = msg.get_structure().get_value('magnitude')

            cs = CubicSpline(self.spectrum_freqs,
                             magnitudes[:self.spectrum_nfreqs])

            magnitudes = cs(self.spectrum_x_axis)

            max_mag = np.amax(magnitudes)
            min_mag = np.amin(magnitudes)

            if max_mag > min_mag:
                magnitudes = (magnitudes - min_mag) / (max_mag - min_mag)

                self.emit('new_spectrum', min_mag, max_mag, magnitudes)

        return True

    def set_wave1_volume(self, value):
        self.audio_src1.set_property('volume', value)

    def set_wave1_freq(self, value):
        self.audio_src1.set_property('freq', value)

    def set_wave1_type(self, value):
        self.audio_src1.set_property('wave', value)

    # amp is a rescaling factor so that all frequencies have the same intensity
    def set_wave2_freq(self, lower, upper, poles):
        current_lower = self.bandpass.get_property('lower-frequency')

        if upper < current_lower:
            self.bandpass.set_property('lower-frequency', lower)
            self.bandpass.set_property('upper-frequency', upper)
        else:
            self.bandpass.set_property('upper-frequency', upper)
            self.bandpass.set_property('lower-frequency', lower)

        self.bandpass.set_property('poles', poles)

    def set_wave2_volume(self, value):
        self.audio_src2.set_property('volume', value)
