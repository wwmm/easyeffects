# -*- coding: utf-8 -*-

import gi
gi.require_version('Gst', '1.0')
from gi.repository import GObject, Gst

import re

Gst.init(None)


class GstEffects(GObject.GObject):

    __gsignals__ = {
        'new_level_before_limiter': (GObject.SIGNAL_RUN_FIRST, None,
                                     (float, float)),
        'new_level_after_limiter': (GObject.SIGNAL_RUN_FIRST, None,
                                    (float, float)),
        'new_limiter_attenuation': (GObject.SIGNAL_RUN_FIRST, None,
                                    (float,)),
        'new_level_after_reverb': (GObject.SIGNAL_RUN_FIRST, None,
                                   (float, float)),
        'new_level_after_eq': (GObject.SIGNAL_RUN_FIRST, None,
                               (float, float)),
        'new_spectrum': (GObject.SIGNAL_RUN_FIRST, None,
                         (object,))
    }

    def __init__(self):
        GObject.GObject.__init__(self)

        self.old_limiter_attenuation = 0
        self.rate = None
        self.max_spectrum_freq = 15000  # Hz
        self.spectrum_nbands = 360
        self.spectrum_freqs = []
        self.spectrum_nfreqs = 0
        self.spectrum_threshold = -100  # dB

        self.pipeline = self.build_pipeline()

        # Create bus to get events from GStreamer pipeline
        bus = self.pipeline.get_bus()
        bus.add_signal_watch()
        bus.connect('message', self.on_message)

    def build_pipeline(self):
        pipeline = Gst.Pipeline()

        self.audio_src = Gst.ElementFactory.make('pulsesrc', None)

        self.limiter = Gst.ElementFactory.make(
            'ladspa-fast-lookahead-limiter-1913-so-fastlookaheadlimiter', None)

        self.equalizer = Gst.ElementFactory.make('equalizer-10bands', None)

        self.audio_sink = Gst.ElementFactory.make('pulsesink', None)

        queue_src = Gst.ElementFactory.make('queue', None)

        level_before_limiter = Gst.ElementFactory.make(
            'level', 'level_before_limiter')
        level_after_limiter = Gst.ElementFactory.make(
            'level', 'level_after_limiter')
        level_after_reverb = Gst.ElementFactory.make(
            'level', 'level_after_reverb')
        level_after_eq = Gst.ElementFactory.make('level', 'level_after_eq')

        self.freeverb = Gst.ElementFactory.make('freeverb', None)

        spectrum_src_type = Gst.ElementFactory.make("typefind", None)

        spectrum = Gst.ElementFactory.make('spectrum', 'spectrum')

        self.audio_src.set_property('client-name', 'PulseEffects')
        self.audio_src.set_property('device', 'PulseEffects.monitor')
        self.audio_src.set_property('provide-clock', False)

        spectrum.set_property('bands', self.spectrum_nbands)
        spectrum.set_property('threshold', self.spectrum_threshold)

        pipeline.add(self.audio_src)
        pipeline.add(queue_src)
        pipeline.add(level_before_limiter)
        pipeline.add(self.limiter)
        pipeline.add(level_after_limiter)
        pipeline.add(self.freeverb)
        pipeline.add(level_after_reverb)
        pipeline.add(self.equalizer)
        pipeline.add(level_after_eq)
        pipeline.add(spectrum_src_type)
        pipeline.add(spectrum)
        pipeline.add(self.audio_sink)

        self.audio_src.link(queue_src)
        queue_src.link(level_before_limiter)
        level_before_limiter.link(self.limiter)
        self.limiter.link(level_after_limiter)
        level_after_limiter.link(self.freeverb)
        self.freeverb.link(level_after_reverb)
        level_after_reverb.link(self.equalizer)
        self.equalizer.link(level_after_eq)
        level_after_eq.link(spectrum_src_type)
        spectrum_src_type.link(spectrum)
        spectrum.link(self.audio_sink)

        spectrum_src_type.connect("have-type", self.media_probe)

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
                print("Failed to pause PulseEffects Gstreamer pipeline!!!")

                return False
            else:
                return True
        elif state == 'playing':
            s = self.pipeline.set_state(Gst.State.PLAYING)

            if s == Gst.StateChangeReturn.FAILURE:
                print("Playing PulseEffects Gstreamer pipeline failed!!!")

                return False
            else:
                return True
        elif state == 'null':
            s = self.pipeline.set_state(Gst.State.NULL)

            if s == Gst.StateChangeReturn.FAILURE:
                print("Stopping PulseEffects Gstreamer pipeline failed!!!")

                return False
            else:
                return True

    def media_probe(self, obj, arg0, caps):
        self.rate = caps.get_structure(0).get_value("rate")

        if self.rate:
            for i in range(self.spectrum_nbands):
                freq = self.rate * (0.5 * i + 0.25) / self.spectrum_nbands

                self.spectrum_freqs.append(freq)

            # getting only freqs below self.max_spectrum_freq
            element = next(i for i in self.spectrum_freqs if i >
                           self.max_spectrum_freq)

            cutt_off_idx = self.spectrum_freqs.index(element)

            self.spectrum_freqs = self.spectrum_freqs[:cutt_off_idx]

            self.spectrum_nfreqs = len(self.spectrum_freqs)

    def on_message(self, bus, msg):
        if msg.type == Gst.MessageType.ERROR:
            print('on_error():', msg.parse_error())
        elif msg.type == Gst.MessageType.ELEMENT:
            plugin = msg.src.get_name()

            if plugin == 'level_before_limiter':
                peak = msg.get_structure().get_value('peak')

                self.emit('new_level_before_limiter', peak[0], peak[1])
            elif plugin == 'level_after_limiter':
                peak = msg.get_structure().get_value('peak')

                self.emit('new_level_after_limiter', peak[0], peak[1])

                attenuation = round(self.limiter.get_property('attenuation'))

                if attenuation != self.old_limiter_attenuation:
                    self.old_limiter_attenuation = attenuation

                    self.emit('new_limiter_attenuation', attenuation)
            elif plugin == 'level_after_reverb':
                peak = msg.get_structure().get_value('peak')

                self.emit('new_level_after_reverb', peak[0], peak[1])
            elif plugin == 'level_after_eq':
                peak = msg.get_structure().get_value('peak')

                self.emit('new_level_after_eq', peak[0], peak[1])
            elif plugin == 'spectrum':
                # GstValueList is not yet supported in gst-python
                # we are going to get the magnitudes parsing the structure
                # string
                struct_str = msg.get_structure().to_string()

                magnitude_str = re.search(r'magnitude=\(float\){(.*)}',
                                          struct_str)

                if magnitude_str:
                    magnitudes = magnitude_str.group(
                        1).replace(' ', '').split(',')

                    magnitudes = magnitudes[:self.spectrum_nfreqs]

                    magnitudes = [float(v) for v in magnitudes]

                    min_value = min(magnitudes)

                    magnitudes = [v - min_value for v in magnitudes]

                    self.emit('new_spectrum', magnitudes)
        return True

    def set_output_sink_name(self, name):
        self.audio_sink.set_property('device', name)

    def set_buffer_time(self, value):
        self.set_state('ready')
        self.audio_src.set_property('buffer-time', value)
        self.audio_sink.set_property('buffer-time', value)
        self.set_state('playing')

    def set_limiter_input_gain(self, value):
        self.limiter.set_property('input-gain', value)

    def set_limiter_limit(self, value):
        self.limiter.set_property('limit', value)

    def set_limiter_release_time(self, value):
        self.limiter.set_property('release-time', value)

    def set_eq_band0(self, value):
        self.equalizer.set_property('band0', value)

    def set_eq_band1(self, value):
        self.equalizer.set_property('band1', value)

    def set_eq_band2(self, value):
        self.equalizer.set_property('band2', value)

    def set_eq_band3(self, value):
        self.equalizer.set_property('band3', value)

    def set_eq_band4(self, value):
        self.equalizer.set_property('band4', value)

    def set_eq_band5(self, value):
        self.equalizer.set_property('band5', value)

    def set_eq_band6(self, value):
        self.equalizer.set_property('band6', value)

    def set_eq_band7(self, value):
        self.equalizer.set_property('band7', value)

    def set_eq_band8(self, value):
        self.equalizer.set_property('band8', value)

    def set_eq_band9(self, value):
        self.equalizer.set_property('band9', value)

    def set_reverb_room_size(self, value):
        self.freeverb.set_property('room-size', value)

    def set_reverb_damping(self, value):
        self.freeverb.set_property('damping', value)

    def set_reverb_width(self, value):
        self.freeverb.set_property('width', value)

    def set_reverb_level(self, value):
        self.freeverb.set_property('level', value)
