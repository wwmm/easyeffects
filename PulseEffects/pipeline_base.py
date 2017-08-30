# -*- coding: utf-8 -*-

import logging

import gi
gi.require_version('Gst', '1.0')
import numpy as np
from gi.repository import GObject, Gst


Gst.init(None)


class PipelineBase(GObject.GObject):

    __gsignals__ = {
        'new_spectrum': (GObject.SIGNAL_RUN_FIRST, None,
                         (object,))
    }

    def __init__(self, sampling_rate):
        GObject.GObject.__init__(self)

        self.is_playing = False
        self.rate = sampling_rate
        self.max_spectrum_freq = 20000  # Hz
        self.spectrum_nbands = 1600
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
        bus.connect('message::latency', self.on_message_latency)
        # on_message_element is implemented by child class
        bus.connect('message::element', self.on_message_element)

        self.audio_src = Gst.ElementFactory.make('pulsesrc', 'audio_src')

        self.source_caps = Gst.ElementFactory.make("capsfilter", None)

        self.spectrum = Gst.ElementFactory.make('spectrum', 'spectrum')

        self.audio_sink = Gst.ElementFactory.make('pulsesink', 'audio_sink')

        self.audio_src.set_property('volume', 1.0)
        self.audio_src.set_property('mute', False)
        self.audio_src.set_property('provide-clock', False)
        self.audio_src.set_property('slave-method', 're-timestamp')

        caps = ['audio/x-raw', 'format=F32LE',
                'rate=' + str(self.rate), 'channels=2']

        src_caps = Gst.Caps.from_string(",".join(caps))
        self.source_caps.set_property("caps", src_caps)

        self.audio_sink.set_property('volume', 1.0)
        self.audio_sink.set_property('mute', False)

        self.spectrum.set_property('bands', self.spectrum_nbands)
        self.spectrum.set_property('threshold', self.spectrum_threshold)

        self.build_limiter_bin()
        self.build_compressor_bin()
        self.build_reverb_bin()
        self.build_highpass_bin()
        self.build_lowpass_bin()
        self.build_equalizer_bin()

    def build_limiter_bin(self):
        self.limiter = Gst.ElementFactory.make(
            'ladspa-fast-lookahead-limiter-1913-so-fastlookaheadlimiter', None)
        limiter_input_level = Gst.ElementFactory.make('level',
                                                      'limiter_input_level')
        limiter_output_level = Gst.ElementFactory.make('level',
                                                       'limiter_output_level')

        self.limiter_bin = Gst.Bin.new('limiter_bin')
        self.limiter_bin.add(self.limiter)
        self.limiter_bin.add(limiter_input_level)
        self.limiter_bin.add(limiter_output_level)

        limiter_input_level.link(self.limiter)
        self.limiter.link(limiter_output_level)

        pad = limiter_input_level.get_static_pad('sink')
        ghost_pad = Gst.GhostPad.new('limiter_bin_sink', pad)
        ghost_pad.set_active(True)
        self.limiter_bin.add_pad(ghost_pad)

        pad = limiter_output_level.get_static_pad('src')
        ghost_pad = Gst.GhostPad.new('limiter_bin_src', pad)
        ghost_pad.set_active(True)
        self.limiter_bin.add_pad(ghost_pad)

    def build_compressor_bin(self):
        self.compressor = Gst.ElementFactory.make(
            'ladspa-sc4-1882-so-sc4', None)
        compressor_input_level = Gst.ElementFactory.make(
            'level', 'compressor_input_level')
        compressor_output_level = Gst.ElementFactory.make(
            'level', 'compressor_output_level')

        self.compressor_bin = Gst.Bin.new('compressor_bin')
        self.compressor_bin.add(self.compressor)
        self.compressor_bin.add(compressor_input_level)
        self.compressor_bin.add(compressor_output_level)

        compressor_input_level.link(self.compressor)
        self.compressor.link(compressor_output_level)

        pad = compressor_input_level.get_static_pad('sink')
        ghost_pad = Gst.GhostPad.new('compressor_bin_sink', pad)
        ghost_pad.set_active(True)
        self.compressor_bin.add_pad(ghost_pad)

        pad = compressor_output_level.get_static_pad('src')
        ghost_pad = Gst.GhostPad.new('compressor_bin_src', pad)
        ghost_pad.set_active(True)
        self.compressor_bin.add_pad(ghost_pad)

    def build_reverb_bin(self):
        self.freeverb = Gst.ElementFactory.make('freeverb', None)
        reverb_input_level = Gst.ElementFactory.make('level',
                                                     'reverb_input_level')
        reverb_output_level = Gst.ElementFactory.make('level',
                                                      'reverb_output_level')

        self.reverb_bin = Gst.Bin.new('reverb_bin')
        self.reverb_bin.add(self.freeverb)
        self.reverb_bin.add(reverb_input_level)
        self.reverb_bin.add(reverb_output_level)

        reverb_input_level.link(self.freeverb)
        self.freeverb.link(reverb_output_level)

        pad = reverb_input_level.get_static_pad('sink')
        ghost_pad = Gst.GhostPad.new('reverb_bin_sink', pad)
        ghost_pad.set_active(True)
        self.reverb_bin.add_pad(ghost_pad)

        pad = reverb_output_level.get_static_pad('src')
        ghost_pad = Gst.GhostPad.new('reverb_bin_src', pad)
        ghost_pad.set_active(True)
        self.reverb_bin.add_pad(ghost_pad)

    def build_highpass_bin(self):
        self.highpass = Gst.ElementFactory.make('audiocheblimit', None)
        highpass_input_level = Gst.ElementFactory.make('level',
                                                       'highpass_input_level')
        highpass_output_level = Gst.ElementFactory.make(
            'level', 'highpass_output_level')

        self.highpass.set_property('mode', 'high-pass')
        self.highpass.set_property('type', 1)
        self.highpass.set_property('ripple', 0)

        self.highpass_bin = Gst.Bin.new('highpass_bin')
        self.highpass_bin.add(self.highpass)
        self.highpass_bin.add(highpass_input_level)
        self.highpass_bin.add(highpass_output_level)

        highpass_input_level.link(self.highpass)
        self.highpass.link(highpass_output_level)

        pad = highpass_input_level.get_static_pad('sink')
        ghost_pad = Gst.GhostPad.new('highpass_bin_sink', pad)
        ghost_pad.set_active(True)
        self.highpass_bin.add_pad(ghost_pad)

        pad = highpass_output_level.get_static_pad('src')
        ghost_pad = Gst.GhostPad.new('highpass_bin_src', pad)
        ghost_pad.set_active(True)
        self.highpass_bin.add_pad(ghost_pad)

    def build_lowpass_bin(self):
        self.lowpass = Gst.ElementFactory.make('audiocheblimit', None)
        lowpass_input_level = Gst.ElementFactory.make('level',
                                                      'lowpass_input_level')
        lowpass_output_level = Gst.ElementFactory.make('level',
                                                       'lowpass_output_level')

        self.lowpass.set_property('mode', 'low-pass')
        self.lowpass.set_property('type', 1)
        self.lowpass.set_property('ripple', 0)

        self.lowpass_bin = Gst.Bin.new('lowpass_bin')
        self.lowpass_bin.add(self.lowpass)
        self.lowpass_bin.add(lowpass_input_level)
        self.lowpass_bin.add(lowpass_output_level)

        lowpass_input_level.link(self.lowpass)
        self.lowpass.link(lowpass_output_level)

        pad = lowpass_input_level.get_static_pad('sink')
        ghost_pad = Gst.GhostPad.new('lowpass_bin_sink', pad)
        ghost_pad.set_active(True)
        self.lowpass_bin.add_pad(ghost_pad)

        pad = lowpass_output_level.get_static_pad('src')
        ghost_pad = Gst.GhostPad.new('lowpass_bin_src', pad)
        ghost_pad.set_active(True)
        self.lowpass_bin.add_pad(ghost_pad)

    def build_equalizer_bin(self):
        self.equalizer_input_gain = Gst.ElementFactory.make('volume', None)
        self.equalizer_output_gain = Gst.ElementFactory.make('volume', None)
        equalizer = Gst.ElementFactory.make('equalizer-nbands', None)
        equalizer_input_level = Gst.ElementFactory.make(
            'level', 'equalizer_input_level')
        equalizer_output_level = Gst.ElementFactory.make(
            'level', 'equalizer_output_level')

        equalizer.set_property('num-bands', 15)

        self.eq_band0 = equalizer.get_child_by_index(0)
        self.eq_band1 = equalizer.get_child_by_index(1)
        self.eq_band2 = equalizer.get_child_by_index(2)
        self.eq_band3 = equalizer.get_child_by_index(3)
        self.eq_band4 = equalizer.get_child_by_index(4)
        self.eq_band5 = equalizer.get_child_by_index(5)
        self.eq_band6 = equalizer.get_child_by_index(6)
        self.eq_band7 = equalizer.get_child_by_index(7)
        self.eq_band8 = equalizer.get_child_by_index(8)
        self.eq_band9 = equalizer.get_child_by_index(9)
        self.eq_band10 = equalizer.get_child_by_index(10)
        self.eq_band11 = equalizer.get_child_by_index(11)
        self.eq_band12 = equalizer.get_child_by_index(12)
        self.eq_band13 = equalizer.get_child_by_index(13)
        self.eq_band14 = equalizer.get_child_by_index(14)

        # It seems there is a bug in the low shelf filter.
        # When we increase the lower shelf gain higher frequencies
        # are attenuated. Setting the first band to peak type instead of
        # shelf fixes this.

        self.eq_band0.set_property('type', 0)  # 0: peak type
        self.eq_band14.set_property('type', 0)  # 0: peak type

        self.equalizer_bin = Gst.Bin.new('equalizer_bin')
        self.equalizer_bin.add(self.equalizer_input_gain)
        self.equalizer_bin.add(equalizer_input_level)
        self.equalizer_bin.add(equalizer)
        self.equalizer_bin.add(self.equalizer_output_gain)
        self.equalizer_bin.add(equalizer_output_level)

        self.equalizer_input_gain.link(equalizer_input_level)
        equalizer_input_level.link(equalizer)
        equalizer.link(self.equalizer_output_gain)
        self.equalizer_output_gain.link(equalizer_output_level)

        pad = self.equalizer_input_gain.get_static_pad('sink')
        ghost_pad = Gst.GhostPad.new('equalizer_bin_sink', pad)
        ghost_pad.set_active(True)
        self.equalizer_bin.add_pad(ghost_pad)

        pad = equalizer_output_level.get_static_pad('src')
        ghost_pad = Gst.GhostPad.new('equalizer_bin_src', pad)
        ghost_pad.set_active(True)
        self.equalizer_bin.add_pad(ghost_pad)

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

    def calc_spectrum_freqs(self):
        self.spectrum_freqs = []

        for i in range(self.spectrum_nbands):
            freq = self.rate * (0.5 * i + 0.25) / self.spectrum_nbands

            if freq > self.max_spectrum_freq:
                break

            self.spectrum_freqs.append(freq)

        self.spectrum_nfreqs = len(self.spectrum_freqs)

        self.spectrum_x_axis = np.logspace(1.3, 4.3, self.spectrum_n_points)
