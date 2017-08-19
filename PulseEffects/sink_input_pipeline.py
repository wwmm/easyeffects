# -*- coding: utf-8 -*-

import logging

import gi
gi.require_version('Gst', '1.0')
import numpy as np
from gi.repository import GObject, Gst


Gst.init(None)


class SinkInputPipeline(GObject.GObject):

    __gsignals__ = {
        'new_autovolume': (GObject.SIGNAL_RUN_FIRST, None,
                           (float,)),
        'new_spectrum': (GObject.SIGNAL_RUN_FIRST, None,
                         (object,))
    }

    def __init__(self, sampling_rate):
        GObject.GObject.__init__(self)

        self.old_limiter_attenuation = 0
        self.old_compressor_gain_reduction = 0
        self.rate = sampling_rate
        self.max_spectrum_freq = 20000  # Hz
        self.spectrum_nbands = 1600
        self.spectrum_freqs = []
        self.spectrum_x_axis = np.array([])
        self.spectrum_n_points = 250  # number of freqs displayed
        self.spectrum_nfreqs = 0
        self.spectrum_threshold = -120  # dB

        self.autovolume_enabled = False
        self.autovolume_target = -12  # dB
        self.autovolume_tolerance = 1  # dB
        self.autovolume_threshold = -50  # autovolume only if avg > threshold

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
        bus.connect('message::latency', self.on_message_latency)
        # on_message_element is implemented by child class
        bus.connect('message::element', self.on_message_element)

    def build_pipeline(self):
        pipeline = Gst.Pipeline()

        self.audio_src = Gst.ElementFactory.make('pulsesrc', 'audio_src')

        source_caps = Gst.ElementFactory.make("capsfilter", None)

        self.limiter = Gst.ElementFactory.make(
            'ladspa-fast-lookahead-limiter-1913-so-fastlookaheadlimiter', None)

        self.compressor = Gst.ElementFactory.make(
            'ladspa-sc4-1882-so-sc4', None)

        self.panorama = Gst.ElementFactory.make('audiopanorama', None)

        self.freeverb = Gst.ElementFactory.make('freeverb', None)

        self.equalizer_input_gain = Gst.ElementFactory.make('volume', None)
        self.equalizer_output_gain = Gst.ElementFactory.make('volume', None)

        self.equalizer = Gst.ElementFactory.make('equalizer-nbands', None)

        self.spectrum = Gst.ElementFactory.make('spectrum', 'spectrum')

        self.output_limiter = Gst.ElementFactory.make(
            'ladspa-fast-lookahead-limiter-1913-so-fastlookaheadlimiter', None)

        self.audio_sink = Gst.ElementFactory.make('pulsesink', 'audio_sink')

        limiter_input_level = Gst.ElementFactory.make(
            'level', 'limiter_input_level')
        limiter_output_level = Gst.ElementFactory.make(
            'level', 'limiter_output_level')
        compressor_output_level = Gst.ElementFactory.make(
            'level', 'compressor_output_level')
        reverb_output_level = Gst.ElementFactory.make(
            'level', 'reverb_output_level')
        highpass_output_level = Gst.ElementFactory.make(
            'level', 'highpass_output_level')
        lowpass_output_level = Gst.ElementFactory.make(
            'level', 'lowpass_output_level')
        equalizer_input_level = Gst.ElementFactory.make(
            'level', 'equalizer_input_level')
        equalizer_output_level = Gst.ElementFactory.make(
            'level', 'equalizer_output_level')

        self.autovolume_level = Gst.ElementFactory.make('level', 'autovolume')

        self.audio_src.set_property('volume', 1.0)
        self.audio_src.set_property('mute', False)
        self.audio_src.set_property('provide-clock', False)
        self.audio_src.set_property('slave-method', 're-timestamp')

        caps = ['audio/x-raw', 'format=F32LE',
                'rate=' + str(self.rate), 'channels=2']

        src_caps = Gst.Caps.from_string(",".join(caps))
        source_caps.set_property("caps", src_caps)

        self.audio_sink.set_property('volume', 1.0)
        self.audio_sink.set_property('mute', False)

        self.panorama.set_property('method', 'psychoacoustic')

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

        self.highpass = Gst.ElementFactory.make('audiocheblimit', None)
        self.highpass.set_property('mode', 'high-pass')
        self.highpass.set_property('type', 1)
        self.highpass.set_property('ripple', 0)

        self.lowpass = Gst.ElementFactory.make('audiocheblimit', None)
        self.lowpass.set_property('mode', 'low-pass')
        self.lowpass.set_property('type', 1)
        self.lowpass.set_property('ripple', 0)

        self.output_limiter.set_property('input-gain', 0)
        self.output_limiter.set_property('limit', 0)
        self.output_limiter.set_property('release-time', 2.0)

        pipeline.add(self.audio_src)
        pipeline.add(source_caps)
        pipeline.add(limiter_input_level)
        pipeline.add(self.limiter)
        pipeline.add(limiter_output_level)
        pipeline.add(self.autovolume_level)
        pipeline.add(self.panorama)
        pipeline.add(self.compressor)
        pipeline.add(compressor_output_level)
        pipeline.add(self.freeverb)
        pipeline.add(reverb_output_level)
        pipeline.add(self.highpass)
        pipeline.add(highpass_output_level)
        pipeline.add(self.lowpass)
        pipeline.add(lowpass_output_level)
        pipeline.add(self.equalizer_input_gain)
        pipeline.add(equalizer_input_level)
        pipeline.add(self.equalizer)
        pipeline.add(self.equalizer_output_gain)
        pipeline.add(equalizer_output_level)
        pipeline.add(self.spectrum)
        pipeline.add(self.output_limiter)
        pipeline.add(self.audio_sink)

        self.audio_src.link(source_caps)
        source_caps.link(limiter_input_level)
        limiter_input_level.link(self.limiter)
        self.limiter.link(limiter_output_level)
        limiter_output_level.link(self.autovolume_level)
        self.autovolume_level.link(self.panorama)
        self.panorama.link(self.compressor)
        self.compressor.link(compressor_output_level)
        compressor_output_level.link(self.freeverb)
        self.freeverb.link(reverb_output_level)
        reverb_output_level.link(self.highpass)
        self.highpass.link(highpass_output_level)
        highpass_output_level.link(self.lowpass)
        self.lowpass.link(lowpass_output_level)
        lowpass_output_level.link(self.equalizer_input_gain)
        self.equalizer_input_gain.link(equalizer_input_level)
        equalizer_input_level.link(self.equalizer)
        self.equalizer.link(self.equalizer_output_gain)
        self.equalizer_output_gain.link(equalizer_output_level)
        equalizer_output_level.link(self.spectrum)
        self.spectrum.link(self.output_limiter)
        self.output_limiter.link(self.audio_sink)

        return pipeline

    def print_eq_freqs(self):
        print(self.eq_band0.get_property('freq'))
        print(self.eq_band1.get_property('freq'))
        print(self.eq_band2.get_property('freq'))
        print(self.eq_band3.get_property('freq'))
        print(self.eq_band4.get_property('freq'))
        print(self.eq_band5.get_property('freq'))
        print(self.eq_band6.get_property('freq'))
        print(self.eq_band7.get_property('freq'))
        print(self.eq_band8.get_property('freq'))
        print(self.eq_band9.get_property('freq'))
        print(self.eq_band10.get_property('freq'))
        print(self.eq_band11.get_property('freq'))
        print(self.eq_band12.get_property('freq'))
        print(self.eq_band13.get_property('freq'))
        print(self.eq_band14.get_property('freq'))

    def print_eq_bandwwidths(self):
        print(self.eq_band0.get_property('bandwidth'))
        print(self.eq_band1.get_property('bandwidth'))
        print(self.eq_band2.get_property('bandwidth'))
        print(self.eq_band3.get_property('bandwidth'))
        print(self.eq_band4.get_property('bandwidth'))
        print(self.eq_band5.get_property('bandwidth'))
        print(self.eq_band6.get_property('bandwidth'))
        print(self.eq_band7.get_property('bandwidth'))
        print(self.eq_band8.get_property('bandwidth'))
        print(self.eq_band9.get_property('bandwidth'))
        print(self.eq_band10.get_property('bandwidth'))
        print(self.eq_band11.get_property('bandwidth'))
        print(self.eq_band12.get_property('bandwidth'))
        print(self.eq_band13.get_property('bandwidth'))
        print(self.eq_band14.get_property('bandwidth'))

    def set_state(self, state):
        if state == 'ready':
            s = self.pipeline.set_state(Gst.State.READY)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.critical("Could not set Gstreamer pipeline to ready")
                return False
            else:
                self.is_playing = False
                self.log.info('apps pipeline state: ready')
                return True
        elif state == 'paused':
            s = self.pipeline.set_state(Gst.State.PAUSED)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.error("Failed to pause Gstreamer pipeline")
                return False
            else:
                self.is_playing = False
                self.log.info('apps pipeline state: paused')
                return True
        elif state == 'playing':
            s = self.pipeline.set_state(Gst.State.PLAYING)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.critical("Playing Gstreamer pipeline has failed")
                return False
            else:
                self.is_playing = True
                self.log.info('apps pipeline state: playing')
                return True
        elif state == 'null':
            s = self.pipeline.set_state(Gst.State.NULL)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.error("Could not stop Gstreamer pipeline")
                return False
            else:
                self.is_playing = False
                self.log.info('apps pipeline state: null')
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

    def auto_gain(self, max_value):
        max_value = int(max_value)

        if max_value > self.autovolume_target + self.autovolume_tolerance:
            gain = self.limiter.get_property('input-gain')

            if gain - 1 >= -20:
                gain = gain - 1

                self.emit('new_autovolume', gain)
        elif max_value < self.autovolume_target - self.autovolume_tolerance:
            gain = self.limiter.get_property('input-gain')

            if gain + 1 <= 20:
                gain = gain + 1

                self.emit('new_autovolume', gain)

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

    def on_message_latency(self, bus, msg):
        plugin = msg.src.get_name()

        if plugin == 'audio_sink':
            latency = msg.src.get_property('latency-time')
            buffer_time = msg.src.get_property('buffer-time')

            self.log.info('pulsesink latency-time [us]: ' + str(latency))
            self.log.info('pulsesink buffer-time [us]: ' +
                          str(buffer_time))
        elif plugin == 'audio_src':
            latency = msg.src.get_property('actual-latency-time')
            buffer_time = msg.src.get_property('actual-buffer-time')

            self.log.info('pulsesrc latency-time [us]: ' + str(latency))
            self.log.info('pulsesrc buffer-time [us]: ' + str(buffer_time))

        return True

    def set_source_monitor_name(self, name):
        self.audio_src.set_property('device', name)

    def set_output_sink_name(self, name):
        self.audio_sink.set_property('device', name)

    def get_configured_src_device(self):
        return self.audio_src.get_property('device')

    def get_current_src_device(self):
        return self.audio_src.get_property('current-device')

    def init_buffer_time(self, value):
        self.audio_src.set_property('buffer-time', value)
        self.audio_sink.set_property('buffer-time', value)

    def set_buffer_time(self, value):
        self.set_state('ready')
        self.audio_src.set_property('buffer-time', value)
        self.audio_sink.set_property('buffer-time', value)
        self.set_state('playing')

    def set_spectrum_n_points(self, value):
        self.spectrum_n_points = value

        self.spectrum_x_axis = np.logspace(1.3, 4.3, value)

    def enable_spectrum(self, state):
        self.spectrum.set_property('post-messages', state)

    def init_latency_time(self, value):
        self.audio_src.set_property('latency-time', value)
        self.audio_sink.set_property('latency-time', value)

    def set_latency_time(self, value):
        self.set_state('ready')
        self.audio_src.set_property('latency-time', value)
        self.audio_sink.set_property('latency-time', value)
        self.set_state('playing')

    def set_autovolume_state(self, value):
        self.autovolume_enabled = value

    def set_autovolume_window(self, value):
        # value must be in seconds
        self.autovolume_level.set_property('interval', int(value * 1000000000))
