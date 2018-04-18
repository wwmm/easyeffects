# -*- coding: utf-8 -*-

import logging

import gi
import numpy as np
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
from gi.repository import GObject, Gst, GstInsertBin

Gst.init(None)


class PipelineBase(GObject.GObject):

    __gsignals__ = {
        'new_spectrum': (GObject.SignalFlags.RUN_FIRST, None, (object,))
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

        self.log_tag = str()

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

        queue_src = Gst.ElementFactory.make('queue', None)

        self.source_caps = Gst.ElementFactory.make('capsfilter', None)

        self.effects_bin = GstInsertBin.InsertBin.new('effects_bin')

        self.spectrum = Gst.ElementFactory.make('spectrum', 'spectrum')

        self.audio_sink = Gst.ElementFactory.make('pulsesink', 'audio_sink')

        self.audio_src.set_property('volume', 1.0)
        self.audio_src.set_property('mute', False)
        self.audio_src.set_property('provide-clock', False)
        self.audio_src.set_property('slave-method', 're-timestamp')

        caps = ['audio/x-raw', 'format=F32LE', 'rate=' + str(self.rate),
                'channels=2']

        src_caps = Gst.Caps.from_string(','.join(caps))
        self.source_caps.set_property('caps', src_caps)

        self.audio_sink.set_property('volume', 1.0)
        self.audio_sink.set_property('mute', False)
        self.audio_sink.set_property('provide-clock', True)

        self.spectrum.set_property('bands', self.spectrum_nbands)
        self.spectrum.set_property('threshold', self.spectrum_threshold)

        queue_src.set_property('silent', True)

        self.pipeline.add(self.audio_src)
        self.pipeline.add(self.source_caps)
        self.pipeline.add(queue_src)
        self.pipeline.add(self.effects_bin)
        self.pipeline.add(self.audio_sink)

        self.audio_src.link(self.source_caps)
        self.source_caps.link(queue_src)
        queue_src.link(self.effects_bin)
        self.effects_bin.link(self.audio_sink)

    def on_filter_added(self, bin, element, success, user_data):
        name_array = element.get_name().split('_')
        name = ''

        for n in name_array:
            name += n + ' '

        if success:
            self.log.debug(self.log_tag + name + 'was enabled')
        else:
            self.log.critical(self.log_tag + 'failed to enable' + name)

    def on_filter_removed(self, bin, element, success, user_data):
        name_array = element.get_name().split('_')
        name = ''

        for n in name_array:
            name += n + ' '

        if success:
            self.log.debug(self.log_tag + name + 'was disabled')
        else:
            self.log.critical(self.log_tag + 'failed to disable' + name)

    def set_state(self, state):
        if state == 'ready':
            s = self.pipeline.set_state(Gst.State.READY)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.critical(self.log_tag +
                                  'Could not set Gstreamer pipeline to ready')

                return False
            else:
                self.is_playing = False

                self.log.debug(self.log_tag + 'pipeline state: ready')

                return True
        elif state == 'paused':
            s = self.pipeline.set_state(Gst.State.PAUSED)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.error(self.log_tag +
                               'Failed to pause Gstreamer pipeline')

                return False
            else:
                self.is_playing = False

                self.log.debug(self.log_tag + 'pipeline state: paused')

                return True
        elif state == 'playing':
            s = self.pipeline.set_state(Gst.State.PLAYING)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.critical(self.log_tag +
                                  'Playing Gstreamer pipeline has failed')

                return False
            else:
                self.is_playing = True

                self.log.debug(self.log_tag + 'pipeline state: playing')

                return True
        elif state == 'null':
            s = self.pipeline.set_state(Gst.State.NULL)

            if s == Gst.StateChangeReturn.FAILURE:
                self.log.error(self.log_tag +
                               'Could not stop Gstreamer pipeline')

                return False
            else:
                self.is_playing = False

                self.log.debug(self.log_tag + 'pipeline state: null')

                return True

    def on_message_error(self, bus, message):
        g_error, msg = message.parse_error()

        self.log.debug(self.log_tag + g_error.message + ' : ' + msg)

        if (g_error.message == 'Disconnected: Entity killed' or
                g_error.message == 'Failed to connect stream: No such entity'):
            self.set_state('null')
            self.audio_src.set_property('device', None)
            self.set_state('playing')
        elif g_error.message == 'Internal data stream error.':
            if 'audio_src' in msg:
                self.set_state('null')

                self.audio_src.set_property('latency-time', 10000)
                self.audio_src.set_property('buffer-time', 100000)

                self.set_state('playing')
        elif 'pa_context_move_source_output_by_name' in g_error.message:
            pass
        else:
            self.set_state('null')
            self.set_state('playing')

        return True

    def on_message_info(self, bus, message):
        g_error, msg = message.parse_info()

        self.log.debug(self.log_tag + g_error.message + ' : ' + msg)

        return True

    def on_message_warning(self, bus, message):
        g_error, msg = message.parse_warning()

        self.log.debug(self.log_tag + g_error.message + ' : ' + msg)

        return True

    def on_message_latency(self, bus, msg):
        plugin = msg.src
        plugin_name = plugin.get_name()

        if plugin_name == 'audio_sink':
            latency = msg.src.get_property('latency-time')
            buffer_time = msg.src.get_property('buffer-time')

            self.log.debug(self.log_tag + 'pulsesink latency-time [us]: ' +
                           str(latency))
            self.log.debug(self.log_tag + 'pulsesink buffer-time [us]: ' +
                           str(buffer_time))
        elif plugin_name == 'audio_src':
            latency = msg.src.get_property('actual-latency-time')
            buffer_time = msg.src.get_property('actual-buffer-time')

            plugin.set_property('buffer-time', buffer_time)
            plugin.set_property('latency-time', latency)

            self.log.debug(self.log_tag + 'pulsesrc latency-time [us]: ' +
                           str(latency))
            self.log.debug(self.log_tag + 'pulsesrc buffer-time [us]: ' +
                           str(buffer_time))

        return True

    def set_pa_props(self, props):
        pa_props_str = 'props,' + props
        pa_props = Gst.Structure.new_from_string(pa_props_str)

        self.audio_src.set_property('stream-properties', pa_props)

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

        # 20 Hz = 10^(1.3), 20000 Hz = 10^(4.3)

        self.spectrum_x_axis = np.logspace(1.3, 4.3, value)

    def init_latency_time(self, value):
        self.audio_src.set_property('latency-time', value)
        self.audio_sink.set_property('latency-time', value)

    def set_latency_time(self, value):
        self.set_state('null')
        self.audio_src.set_property('latency-time', value)
        self.audio_sink.set_property('latency-time', value)
        self.set_state('playing')

    def calc_spectrum_freqs(self):
        self.spectrum_freqs = []

        for i in range(self.spectrum_nbands):
            freq = self.rate * (0.5 * i + 0.25) / self.spectrum_nbands

            if freq > self.max_spectrum_freq:
                break

            self.spectrum_freqs.append(freq)

        self.spectrum_nfreqs = len(self.spectrum_freqs)

        # 20 Hz = 10^(1.3), 20000 Hz = 10^(4.3)

        self.spectrum_x_axis = np.logspace(1.3, 4.3, self.spectrum_n_points)
