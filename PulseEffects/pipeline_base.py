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

    switch_on_all_apps = GObject.Property(type=bool, default=False)

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

        queue_src = Gst.ElementFactory.make('queue', None)

        self.source_caps = Gst.ElementFactory.make("capsfilter", None)

        self.effects_bin = GstInsertBin.InsertBin.new('effects_bin')

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
        bin_name = element.get_name()
        plugin_name = bin_name.split('_')[0]

        if success:
            self.log.info(user_data + plugin_name + ' plugin was enabled')
        else:
            self.log.critical(user_data + 'failed to enable ' + plugin_name)

    def on_filter_removed(self, bin, element, success, user_data):
        bin_name = element.get_name()
        plugin_name = bin_name.split('_')[0]

        if success:
            self.log.info(user_data + plugin_name + ' plugin was disabled')
        else:
            self.log.critical(user_data + 'failed to disable ' + plugin_name)

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

    def set_pa_props(self, props):
        pa_props_str = 'props,' + props
        pa_props = Gst.Structure.new_from_string(pa_props_str)

        self.audio_src.set_property('stream-properties', pa_props)

    def set_source_monitor_name(self, name):
        self.audio_src.set_property('device', name)

    def update_source_monitor_name(self, obj, name):
        self.audio_src.set_property('device', name)

    def set_output_sink_name(self, name):
        self.audio_sink.set_property('device', name)

    def update_output_sink_name(self, obj, name):
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
        self.set_state('ready')
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
