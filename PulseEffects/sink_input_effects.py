# -*- coding: utf-8 -*-

import logging

import gi
import numpy as np
from scipy.interpolate import CubicSpline
gi.require_version('Gst', '1.0')
from gi.repository import GObject, Gst


Gst.init(None)


class SinkInputEffects(GObject.GObject):

    __gsignals__ = {
        'new_limiter_input_level': (GObject.SIGNAL_RUN_FIRST, None,
                                    (float, float)),
        'new_limiter_output_level': (GObject.SIGNAL_RUN_FIRST, None,
                                     (float, float)),
        'new_autovolume': (GObject.SIGNAL_RUN_FIRST, None,
                           (float,)),
        'new_compressor_input_level': (GObject.SIGNAL_RUN_FIRST, None,
                                       (float, float)),
        'new_compressor_output_level': (GObject.SIGNAL_RUN_FIRST, None,
                                        (float, float)),
        'new_compressor_gain_reduction': (GObject.SIGNAL_RUN_FIRST, None,
                                          (float,)),
        'new_limiter_attenuation': (GObject.SIGNAL_RUN_FIRST, None,
                                    (float,)),
        'new_reverb_input_level': (GObject.SIGNAL_RUN_FIRST, None,
                                   (float, float)),
        'new_reverb_output_level': (GObject.SIGNAL_RUN_FIRST, None,
                                    (float, float)),
        'new_equalizer_input_level': (GObject.SIGNAL_RUN_FIRST, None,
                                      (float, float)),
        'new_equalizer_output_level': (GObject.SIGNAL_RUN_FIRST, None,
                                       (float, float)),
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

        spectrum = Gst.ElementFactory.make('spectrum', 'spectrum')

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

        spectrum.set_property('bands', self.spectrum_nbands)
        spectrum.set_property('threshold', self.spectrum_threshold)

        self.eq_highpass = Gst.ElementFactory.make('audiocheblimit', None)
        self.eq_highpass.set_property('mode', 'high-pass')
        self.eq_highpass.set_property('type', 1)
        self.eq_highpass.set_property('ripple', 0)

        self.eq_lowpass = Gst.ElementFactory.make('audiocheblimit', None)
        self.eq_lowpass.set_property('mode', 'low-pass')
        self.eq_lowpass.set_property('type', 1)
        self.eq_lowpass.set_property('ripple', 0)

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
        pipeline.add(self.equalizer_input_gain)
        pipeline.add(self.eq_highpass)
        pipeline.add(self.eq_lowpass)
        pipeline.add(equalizer_input_level)
        pipeline.add(self.equalizer)
        pipeline.add(self.equalizer_output_gain)
        pipeline.add(self.output_limiter)
        pipeline.add(equalizer_output_level)
        pipeline.add(spectrum)
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
        reverb_output_level.link(self.equalizer_input_gain)
        self.equalizer_input_gain.link(self.eq_highpass)
        self.eq_highpass.link(self.eq_lowpass)
        self.eq_lowpass.link(equalizer_input_level)
        equalizer_input_level.link(self.equalizer)
        self.equalizer.link(self.equalizer_output_gain)
        self.equalizer_output_gain.link(self.output_limiter)
        self.output_limiter.link(equalizer_output_level)
        equalizer_output_level.link(spectrum)
        spectrum.link(self.audio_sink)

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

        # print(self.eq_band0.get_property('bandwidth'))

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

    def on_message_element(self, bus, msg):
        plugin = msg.src.get_name()

        if plugin == 'limiter_input_level':
            peak = msg.get_structure().get_value('peak')

            self.emit('new_limiter_input_level', peak[0], peak[1])
        elif plugin == 'limiter_output_level':
            peak = msg.get_structure().get_value('peak')

            self.emit('new_limiter_output_level', peak[0], peak[1])
            self.emit('new_compressor_input_level', peak[0], peak[1])

            attenuation = round(self.limiter.get_property('attenuation'))

            if attenuation != self.old_limiter_attenuation:
                self.old_limiter_attenuation = attenuation

                self.emit('new_limiter_attenuation', attenuation)
        elif plugin == 'autovolume':
            if self.autovolume_enabled:
                peak = msg.get_structure().get_value('peak')

                max_value = max(peak)

                if max_value > self.autovolume_threshold:
                    self.auto_gain(max_value)

        elif plugin == 'compressor_output_level':
            peak = msg.get_structure().get_value('peak')

            self.emit('new_compressor_output_level', peak[0], peak[1])
            self.emit('new_reverb_input_level', peak[0], peak[1])

            gain_reduction = round(
                self.compressor.get_property('gain-reduction'))

            if gain_reduction != self.old_compressor_gain_reduction:
                self.old_compressor_gain_reduction = gain_reduction

                self.emit('new_compressor_gain_reduction', gain_reduction)
        elif plugin == 'reverb_output_level':
            peak = msg.get_structure().get_value('peak')

            self.emit('new_reverb_output_level', peak[0], peak[1])
        elif plugin == 'equalizer_input_level':
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
        # value must be in ms
        self.autovolume_level.set_property('interval', value * 1000000)

    def set_limiter_input_gain(self, value):
        self.limiter.set_property('input-gain', value)

    def set_limiter_limit(self, value):
        self.limiter.set_property('limit', value)

    def set_limiter_release_time(self, value):
        self.limiter.set_property('release-time', value)

    def set_panorama(self, value):
        self.panorama.set_property('panorama', value)

    def set_compressor_measurement_type(self, value):
        self.compressor.set_property('rms-peak', value)

    def set_compressor_attack(self, value):
        self.compressor.set_property('attack-time', value)

    def set_compressor_release(self, value):
        self.compressor.set_property('release-time', value)

    def set_compressor_threshold(self, value):
        self.compressor.set_property('threshold-level', value)

    def set_compressor_ratio(self, value):
        self.compressor.set_property('ratio', value)

    def set_compressor_knee(self, value):
        self.compressor.set_property('knee-radius', value)

    def set_compressor_makeup(self, value):
        self.compressor.set_property('makeup-gain', value)

    def set_reverb_room_size(self, value):
        self.freeverb.set_property('room-size', value)

    def set_reverb_damping(self, value):
        self.freeverb.set_property('damping', value)

    def set_reverb_width(self, value):
        self.freeverb.set_property('width', value)

    def set_reverb_level(self, value):
        self.freeverb.set_property('level', value)

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

    def set_eq_highpass_cutoff_freq(self, value):
        self.eq_highpass.set_property('cutoff', value)

    def set_eq_highpass_poles(self, value):
        self.eq_highpass.set_property('poles', value)

    def set_eq_lowpass_cutoff_freq(self, value):
        self.eq_lowpass.set_property('cutoff', value)

    def set_eq_lowpass_poles(self, value):
        self.eq_lowpass.set_property('poles', value)
