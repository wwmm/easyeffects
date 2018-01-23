# -*- coding: utf-8 -*-

import os

import gi
import numpy as np
gi.require_version('Gst', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import GObject, Gst, Gtk
from scipy.interpolate import CubicSpline

from PulseEffectsTest.pipeline_base import PipelineBase


class Microphone(PipelineBase):

    __gsignals__ = {
        'noise_measured': (GObject.SignalFlags.RUN_FIRST, None, ()),
        'new_guideline_position': (GObject.SignalFlags.RUN_FIRST, None,
                                   (float,))
    }

    def __init__(self, rate):
        PipelineBase.__init__(self, rate)

        self.module_path = os.path.dirname(__file__)
        self.measure_noise = False
        self.subtract_noise = False
        self.ambient_noise = np.array([])

        self.calc_spectrum_freqs()
        self.build_pipeline()
        self.init_ui()

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

    def init_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/microphone.glade')

        self.builder.connect_signals(self)

        self.ui_window = self.builder.get_object('window')
        self.ui_noise_spinner = self.builder.get_object('noise_spinner')

        time_window = self.builder.get_object('time_window')
        guideline_position = self.builder.get_object('guideline_position')

        time_window.set_value(2)
        guideline_position.set_value(0.5)

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

                self.ui_noise_spinner.stop()

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

    def on_time_window_value_changed(self, obj):
        value = obj.get_value()

        self.spectrum.set_property('interval', int(value * 1000000000))

    def on_measure_noise_clicked(self, obj):
        self.mic.measure_noise = True
        self.ui_noise_spinner.start()

    def on_subtract_noise_toggled(self, obj):
        self.mic.subtract_noise = obj.get_active()

    def on_guideline_position_value_changed(self, obj):
        self.emit('new_guideline_position', obj.get_value())
