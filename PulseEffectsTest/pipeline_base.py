# -*- coding: utf-8 -*-

import logging

import gi
import numpy as np
gi.require_version('Gst', '1.0')
from gi.repository import GObject, Gst

Gst.init(None)


class PipelineBase(GObject.GObject):

    __gsignals__ = {
        'new_spectrum': (GObject.SignalFlags.RUN_FIRST, None,
                         (float, float, object))
    }

    switch_on_all_apps = GObject.Property(type=bool, default=False)

    def __init__(self, rate):
        GObject.GObject.__init__(self)

        self.is_playing = False
        self.rate = rate
        self.max_freq = 20000  # Hz
        self.min_freq = 20  # Hz
        self.spectrum_nbands = 3600
        self.spectrum_freqs = []
        self.spectrum_x_axis = np.array([])
        self.spectrum_n_points = 250  # number of freqs displayed
        self.spectrum_nfreqs = 0
        self.spectrum_threshold = -120  # dB

        self.log = logging.getLogger('PulseEffects')

        self.calc_spectrum_freqs()

        self.pipeline = Gst.Pipeline()

        # Create bus to get events from GStreamer pipeline
        bus = self.pipeline.get_bus()
        bus.add_signal_watch()
        bus.connect('message::error', self.on_message_error)
        bus.connect('message::info', self.on_message_info)
        bus.connect('message::warning', self.on_message_warning)
        # on_message_element is implemented by child class
        bus.connect('message::element', self.on_message_element)

        self.spectrum = Gst.ElementFactory.make('spectrum', 'spectrum')

        self.spectrum.set_property('bands', self.spectrum_nbands)
        self.spectrum.set_property('threshold', self.spectrum_threshold)

    def set_state(self, state):
        if state == 'ready':
            s = self.pipeline.set_state(Gst.State.READY)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.critical("Could not set Gstreamer pipeline to ready")
                return False
            else:
                self.is_playing = False
                self.log.info('pipeline state: ready')
                return True
        elif state == 'paused':
            s = self.pipeline.set_state(Gst.State.PAUSED)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.error("Failed to pause Gstreamer pipeline")
                return False
            else:
                self.is_playing = False
                self.log.info('pipeline state: paused')
                return True
        elif state == 'playing':
            s = self.pipeline.set_state(Gst.State.PLAYING)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.critical("Playing Gstreamer pipeline has failed")
                return False
            else:
                self.is_playing = True
                self.log.info('pipeline state: playing')
                return True
        elif state == 'null':
            s = self.pipeline.set_state(Gst.State.NULL)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.error("Could not stop Gstreamer pipeline")
                return False
            else:
                self.is_playing = False
                self.log.info('pipeline state: null')
                return True

    def on_message_error(self, bus, msg):
        self.log.error(msg.parse_error())
        self.set_state('null')
        self.set_state('playing')

        return True

    def on_message_info(self, bus, msg):
        self.log.info(msg.parse_info())

        return True

    def on_message_warning(self, bus, msg):
        self.log.warning(msg.parse_warning())

        return True

    def enable_spectrum(self, state):
        self.spectrum.set_property('post-messages', state)

    def set_spectrum_n_points(self, value):
        self.spectrum_n_points = value

        # 20 Hz = 10^(1.3), 20000 Hz = 10^(4.3)

        self.spectrum_x_axis = np.logspace(1.3, 4.3, value)

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
