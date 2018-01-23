# -*- coding: utf-8 -*-

import gi
import numpy as np
gi.require_version('Gst', '1.0')
from gi.repository import Gst
from scipy.interpolate import CubicSpline

from PulseEffectsTest.pipeline_base import PipelineBase


class TestSignal(PipelineBase):

    def __init__(self, rate):
        PipelineBase.__init__(self, rate)

        self.calc_spectrum_freqs()
        self.build_pipeline()

    def build_pipeline(self):
        self.audio_src1 = Gst.ElementFactory.make('audiotestsrc', None)
        self.audio_src2 = Gst.ElementFactory.make('audiotestsrc', None)
        src1_caps = Gst.ElementFactory.make("capsfilter", None)
        src2_caps = Gst.ElementFactory.make("capsfilter", None)
        self.bandpass = Gst.ElementFactory.make('audiochebband', None)
        mixer = Gst.ElementFactory.make('audiomixer', None)
        self.spectrum = Gst.ElementFactory.make('spectrum', 'spectrum')
        self.audio_sink = Gst.ElementFactory.make('pulsesink', None)

        self.audio_sink.set_property('device', 'PulseEffects_apps')
        self.audio_sink.set_property('volume', 1.0)
        self.audio_sink.set_property('mute', False)

        pa_props_str = 'props,application.name=PulseEffectsTest'
        pa_props = Gst.Structure.new_from_string(pa_props_str)

        self.audio_sink.set_property('stream-properties', pa_props)

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

        self.pipeline.add(self.audio_src1)
        self.pipeline.add(self.audio_src2)
        self.pipeline.add(src1_caps)
        self.pipeline.add(src2_caps)
        self.pipeline.add(self.bandpass)
        self.pipeline.add(mixer)
        self.pipeline.add(self.spectrum)
        self.pipeline.add(self.audio_sink)

        self.audio_src1.link(src1_caps)
        src1_caps.link(mixer)
        mixer.link(self.spectrum)
        self.spectrum.link(self.audio_sink)

        self.audio_src2.link(src2_caps)
        src2_caps.link(self.bandpass)
        self.bandpass.link(mixer)

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
