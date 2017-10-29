# -*- coding: utf-8 -*-

import logging

import gi
import numpy as np
from scipy.interpolate import CubicSpline
gi.require_version('Gst', '1.0')
from gi.repository import GObject, Gst


Gst.init(None)


class MicrophonePipeline(GObject.GObject):

    __gsignals__ = {
        'new_equalizer_input_level': (GObject.SignalFlags.RUN_FIRST, None,
                                      (float,)),
        'new_spectrum': (GObject.SignalFlags.RUN_FIRST, None,
                         (float, float, object)),
        'noise_measured': (GObject.SignalFlags.RUN_FIRST, None, ())
    }

    def __init__(self):
        GObject.GObject.__init__(self)

        self.rate = 48000
        self.max_freq = 20000  # Hz
        self.min_freq = 20  # Hz
        self.spectrum_nbands = 3600
        self.spectrum_freqs = []
        self.spectrum_x_axis = np.array([])
        self.spectrum_n_points = 3600  # number of freqs displayed
        self.spectrum_nfreqs = 0
        self.spectrum_threshold = -120  # dB

        self.is_playing = False

        self.measure_noise = False
        self.subtract_noise = False
        self.ambient_noise = np.array([])

        self.log = logging.getLogger('PulseEffects')

        self.calc_spectrum_freqs()

        self.pipeline = self.build_pipeline()

        # Create bus to get events from GStreamer pipeline
        bus = self.pipeline.get_bus()
        bus.add_signal_watch()
        bus.connect('message::error', self.on_message_error)
        bus.connect('message::info', self.on_message_info)
        bus.connect('message::warning', self.on_message_warning)
        bus.connect('message::element', self.on_message_element)

    def build_pipeline(self):
        pipeline = Gst.Pipeline()

        self.audio_src = Gst.ElementFactory.make('pulsesrc', 'audio_src')

        source_caps = Gst.ElementFactory.make("capsfilter", None)

        self.spectrum = Gst.ElementFactory.make('spectrum', 'spectrum')

        self.audio_sink = Gst.ElementFactory.make('fakesink')

        self.audio_src.set_property('volume', 1.0)
        self.audio_src.set_property('mute', False)

        caps = ['audio/x-raw', 'format=F32LE',
                'rate=' + str(self.rate), 'channels=1']

        src_caps = Gst.Caps.from_string(",".join(caps))
        source_caps.set_property("caps", src_caps)

        self.spectrum.set_property('bands', self.spectrum_nbands)
        self.spectrum.set_property('threshold', self.spectrum_threshold)

        pipeline.add(self.audio_src)
        pipeline.add(source_caps)
        pipeline.add(self.spectrum)
        pipeline.add(self.audio_sink)

        self.audio_src.link(source_caps)
        source_caps.link(self.spectrum)
        self.spectrum.link(self.audio_sink)

        return pipeline

    def set_state(self, state):
        if state == 'ready':
            s = self.pipeline.set_state(Gst.State.READY)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.critical("Could not set Gstreamer pipeline to ready")
                return False
            else:
                self.is_playing = False
                self.log.info('mic pipeline state: ready')
                return True
        elif state == 'paused':
            s = self.pipeline.set_state(Gst.State.PAUSED)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.error("Failed to pause Gstreamer pipeline")
                return False
            else:
                self.is_playing = False
                self.log.info('mic pipeline state: paused')
                return True
        elif state == 'playing':
            s = self.pipeline.set_state(Gst.State.PLAYING)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.critical("Playing Gstreamer pipeline has failed")
                return False
            else:
                self.is_playing = True
                self.log.info('mic pipeline state: playing')
                return True
        elif state == 'null':
            s = self.pipeline.set_state(Gst.State.NULL)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.error("Could not stop Gstreamer pipeline")
                return False
            else:
                self.is_playing = False
                self.log.info('mic pipeline state: null')
                return True

    def calc_spectrum_freqs(self):
        self.spectrum_freqs = []

        for i in range(self.spectrum_nbands):
            freq = self.rate * (0.5 * i + 0.25) / self.spectrum_nbands

            if freq > self.max_freq:
                break

            if freq >= self.min_freq:
                self.spectrum_freqs.append(freq)

        self.spectrum_nfreqs = len(self.spectrum_freqs)

        fmin = np.log10(self.min_freq)
        fmax = np.log10(self.max_freq)

        self.spectrum_x_axis = np.logspace(fmin, fmax, self.spectrum_n_points)

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

        if plugin == 'equalizer_input_level':
            peak = msg.get_structure().get_value('peak')

            self.emit('new_equalizer_input_level', peak[0])
        elif plugin == 'spectrum':
            magnitudes = msg.get_structure().get_value('magnitude')

            cs = CubicSpline(self.spectrum_freqs,
                             magnitudes[:self.spectrum_nfreqs])

            magnitudes = cs(self.spectrum_x_axis)

            if self.measure_noise:
                self.ambient_noise = magnitudes
                self.measure_noise = False
                self.emit('noise_measured')

            if self.subtract_noise:
                magnitudes = magnitudes - self.ambient_noise

            max_mag = np.amax(magnitudes)
            min_mag = np.amin(magnitudes)

            if max_mag > min_mag:
                magnitudes = (magnitudes - min_mag) / (max_mag - min_mag)

                self.emit('new_spectrum', min_mag, max_mag, magnitudes)

        return True

    def set_source_monitor_name(self, name):
        self.audio_src.set_property('device', name)

    def set_time_window(self, value):
        self.spectrum.set_property('interval', int(value * 1000000000))
