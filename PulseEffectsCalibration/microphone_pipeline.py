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
        'new_equalizer_input_level': (GObject.SIGNAL_RUN_FIRST, None,
                                      (float, float)),
        'new_equalizer_output_level': (GObject.SIGNAL_RUN_FIRST, None,
                                       (float, float)),
        'new_spectrum': (GObject.SIGNAL_RUN_FIRST, None,
                         (object,))
    }

    def __init__(self):
        GObject.GObject.__init__(self)

        self.old_limiter_attenuation = 0
        self.old_compressor_gain_reduction = 0
        self.rate = 48000
        self.max_spectrum_freq = 20000  # Hz
        self.spectrum_nbands = 1600
        self.spectrum_freqs = []
        self.spectrum_x_axis = np.array([])
        self.spectrum_n_points = 250  # number of freqs displayed
        self.spectrum_nfreqs = 0
        self.spectrum_threshold = -120  # dB

        self.is_playing = False

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

        self.equalizer_input_gain = Gst.ElementFactory.make('volume', None)
        self.equalizer_output_gain = Gst.ElementFactory.make('volume', None)

        self.equalizer = Gst.ElementFactory.make('equalizer-nbands', None)

        self.spectrum = Gst.ElementFactory.make('spectrum', 'spectrum')

        self.audio_sink = Gst.ElementFactory.make('fakesink')

        equalizer_input_level = Gst.ElementFactory.make(
            'level', 'equalizer_input_level')
        equalizer_output_level = Gst.ElementFactory.make(
            'level', 'equalizer_output_level')

        self.audio_src.set_property('volume', 1.0)
        self.audio_src.set_property('mute', False)
        self.audio_src.set_property('provide-clock', False)
        self.audio_src.set_property('slave-method', 're-timestamp')

        caps = ['audio/x-raw', 'format=F32LE',
                'rate=' + str(self.rate), 'channels=2']

        src_caps = Gst.Caps.from_string(",".join(caps))
        source_caps.set_property("caps", src_caps)

        self.equalizer.set_property('num-bands', 15)

        self.eq_band0 = self.equalizer.get_child_by_index(0)
        self.eq_band1 = self.equalizer.get_child_by_index(1)
        self.eq_band2 = self.equalizer.get_child_by_index(2)
        self.eq_band3 = self.equalizer.get_child_by_index(3)
        self.eq_band4 = self.equalizer.get_child_by_index(4)
        self.eq_band5 = self.equalizer.get_child_by_index(5)
        self.eq_band6 = self.equalizer.get_child_by_index(6)
        self.eq_band7 = self.equalizer.get_child_by_index(7)
        self.eq_band8 = self.equalizer.get_child_by_index(8)
        self.eq_band9 = self.equalizer.get_child_by_index(9)
        self.eq_band10 = self.equalizer.get_child_by_index(10)
        self.eq_band11 = self.equalizer.get_child_by_index(11)
        self.eq_band12 = self.equalizer.get_child_by_index(12)
        self.eq_band13 = self.equalizer.get_child_by_index(13)
        self.eq_band14 = self.equalizer.get_child_by_index(14)

        # It seems there is a bug in the low shelf filter.
        # When we increase the lower shelf gain higher frequencies
        # are attenuated. Setting the first band to peak type instead of
        # shelf fixes this.

        self.eq_band0.set_property('type', 0)  # 0: peak type
        self.eq_band14.set_property('type', 0)  # 0: peak type

        self.spectrum.set_property('bands', self.spectrum_nbands)
        self.spectrum.set_property('threshold', self.spectrum_threshold)

        pipeline.add(self.audio_src)
        pipeline.add(source_caps)
        pipeline.add(self.equalizer_input_gain)
        pipeline.add(equalizer_input_level)
        pipeline.add(self.equalizer)
        pipeline.add(self.equalizer_output_gain)
        pipeline.add(equalizer_output_level)
        pipeline.add(self.spectrum)
        pipeline.add(self.audio_sink)

        self.audio_src.link(source_caps)
        source_caps.link(self.equalizer_input_gain)
        self.equalizer_input_gain.link(equalizer_input_level)
        equalizer_input_level.link(self.equalizer)
        self.equalizer.link(self.equalizer_output_gain)
        self.equalizer_output_gain.link(equalizer_output_level)
        equalizer_output_level.link(self.spectrum)
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

            if freq > self.max_spectrum_freq:
                break

            self.spectrum_freqs.append(freq)

        self.spectrum_nfreqs = len(self.spectrum_freqs)

        self.spectrum_x_axis = np.logspace(1.3, 4.3, self.spectrum_n_points)

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

            self.emit('new_equalizer_input_level', peak[0], peak[1])
        elif plugin == 'equalizer_output_level':
            peak = msg.get_structure().get_value('peak')

            self.emit('new_equalizer_output_level', peak[0], peak[1])
        elif plugin == 'spectrum':
            magnitudes = msg.get_structure().get_value('magnitude')

            cs = CubicSpline(self.spectrum_freqs,
                             magnitudes[:self.spectrum_nfreqs])

            magnitudes = cs(self.spectrum_x_axis)

            max_mag = np.amax(magnitudes)
            min_mag = self.spectrum_threshold

            if max_mag > min_mag:
                magnitudes = (min_mag - magnitudes) / min_mag

                self.emit('new_spectrum', magnitudes)

        return True

    def set_source_monitor_name(self, name):
        self.audio_src.set_property('device', name)

    def set_eq_input_gain(self, value):
        self.equalizer_input_gain.set_property('volume', value)

    def set_eq_output_gain(self, value):
        self.equalizer_output_gain.set_property('volume', value)

    def set_eq_band0(self, value):
        self.eq_band0.set_property('gain', value)

    def set_eq_band1(self, value):
        self.eq_band1.set_property('gain', value)

    def set_eq_band2(self, value):
        self.eq_band2.set_property('gain', value)

    def set_eq_band3(self, value):
        self.eq_band3.set_property('gain', value)

    def set_eq_band4(self, value):
        self.eq_band4.set_property('gain', value)

    def set_eq_band5(self, value):
        self.eq_band5.set_property('gain', value)

    def set_eq_band6(self, value):
        self.eq_band6.set_property('gain', value)

    def set_eq_band7(self, value):
        self.eq_band7.set_property('gain', value)

    def set_eq_band8(self, value):
        self.eq_band8.set_property('gain', value)

    def set_eq_band9(self, value):
        self.eq_band9.set_property('gain', value)

    def set_eq_band10(self, value):
        self.eq_band10.set_property('gain', value)

    def set_eq_band11(self, value):
        self.eq_band11.set_property('gain', value)

    def set_eq_band12(self, value):
        self.eq_band12.set_property('gain', value)

    def set_eq_band13(self, value):
        self.eq_band13.set_property('gain', value)

    def set_eq_band14(self, value):
        self.eq_band14.set_property('gain', value)
