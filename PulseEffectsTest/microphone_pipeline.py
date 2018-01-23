# -*- coding: utf-8 -*-

import gi
import numpy as np
gi.require_version('Gst', '1.0')
from gi.repository import GObject, Gst
from scipy.interpolate import CubicSpline

from PulseEffectsTest.pipeline_base import PipelineBase


Gst.init(None)


class MicrophonePipeline(PipelineBase):

    __gsignals__ = {
        'noise_measured': (GObject.SignalFlags.RUN_FIRST, None, ())
    }

    def __init__(self, rate):
        PipelineBase.__init__(self, rate)

        self.measure_noise = False
        self.subtract_noise = False
        self.ambient_noise = np.array([])

        self.calc_spectrum_freqs()
        self.build_pipeline()

    def build_pipeline(self):
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

        self.pipeline.add(self.audio_src)
        self.pipeline.add(source_caps)
        self.pipeline.add(self.spectrum)
        self.pipeline.add(self.audio_sink)

        self.audio_src.link(source_caps)
        source_caps.link(self.spectrum)
        self.spectrum.link(self.audio_sink)

    def on_message_element(self, bus, msg):
        plugin = msg.src.get_name()

        if plugin == 'spectrum':
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
